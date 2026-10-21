#ifndef LINUX_PLATFORM

#include "partition_table_update.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_flash.h"
#include "esp_flash_internal.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_core_dump.h"

static const char *TAG = "part_update";

// Dati binari della partition table incorporata nel binario tramite target_add_binary_data
extern const uint8_t _binary_partition_table_bin_start[];
extern const uint8_t _binary_partition_table_bin_end[];

esp_err_t update_partition_table(void)
{
    const size_t embedded_size = (size_t)(_binary_partition_table_bin_end - _binary_partition_table_bin_start);
    if (embedded_size == 0 || embedded_size > ESP_PARTITION_TABLE_MAX_LEN) {
        ESP_LOGE(TAG, "Dimensione partition table incorporata non valida: %zu", embedded_size);
        return ESP_ERR_INVALID_SIZE;
    }

    const esp_partition_info_t *emb_table = (const esp_partition_info_t *)_binary_partition_table_bin_start;
    int emb_num_parts = 0;
    esp_err_t err = esp_partition_table_verify(emb_table, true, &emb_num_parts);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Partition table incorporata non valida: %s", esp_err_to_name(err));
        return err;
    }

    // Leggi la partition table attualmente in flash
    uint8_t flash_table_buf[ESP_PARTITION_TABLE_MAX_LEN];
    err = esp_flash_read(esp_flash_default_chip, flash_table_buf, ESP_PARTITION_TABLE_OFFSET, embedded_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Errore lettura partition table da flash: %s", esp_err_to_name(err));
        return err;
    }

    // Se il contenuto in flash coincide con quello incorporato, non c'è nulla da fare
    if (memcmp(flash_table_buf, _binary_partition_table_bin_start, embedded_size) == 0) {
        ESP_LOGI(TAG, "Partition table in flash già allineata.");
        return ESP_OK;
    }

    ESP_LOGW(TAG, "Rilevata discrepanza nella partition table in flash. Avvio aggiornamento...");

    // Verifica di sicurezza: controlla che la partizione attualmente in esecuzione non sia stata spostata
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running != NULL) {
        bool running_matched = false;
        for (int i = 0; i < emb_num_parts; i++) {
            const esp_partition_info_t *p = &emb_table[i];
            if (p->magic == ESP_PARTITION_MAGIC &&
                p->type == running->type &&
                p->subtype == running->subtype &&
                p->pos.offset == running->address) {
                running_matched = true;
                break;
            }
        }
        if (!running_matched) {
            ESP_LOGE(TAG, "Sicurezza: la partizione corrente (offset 0x%08" PRIx32 ") non corrisponde nella nuova tabella! Annullamento.",
                     running->address);
            return ESP_ERR_INVALID_STATE;
        }
    }

    // Disabilita la protezione di scrittura sulle regioni protette (offset 0x8000)
    err = esp_flash_set_dangerous_write_protection(esp_flash_default_chip, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Impossibile disabilitare protezione di scrittura: %s", esp_err_to_name(err));
        return err;
    }

    // Cancella il settore di flash contenente la partition table (4KB = ESP_PARTITION_TABLE_SIZE)
    err = esp_flash_erase_region(esp_flash_default_chip, ESP_PARTITION_TABLE_OFFSET, ESP_PARTITION_TABLE_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Errore cancellazione settore partition table: %s", esp_err_to_name(err));
        esp_flash_set_dangerous_write_protection(esp_flash_default_chip, true);
        return err;
    }

    // Scrive la nuova partition table
    err = esp_flash_write(esp_flash_default_chip, _binary_partition_table_bin_start, ESP_PARTITION_TABLE_OFFSET, embedded_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Errore scrittura partition table: %s", esp_err_to_name(err));
        esp_flash_set_dangerous_write_protection(esp_flash_default_chip, true);
        return err;
    }

    // Riabilita la protezione di scrittura
    esp_flash_set_dangerous_write_protection(esp_flash_default_chip, true);

    // Rileggi e verifica i dati scritti in flash
    uint8_t readback_buf[ESP_PARTITION_TABLE_MAX_LEN];
    err = esp_flash_read(esp_flash_default_chip, readback_buf, ESP_PARTITION_TABLE_OFFSET, embedded_size);
    if (err != ESP_OK || memcmp(readback_buf, _binary_partition_table_bin_start, embedded_size) != 0) {
        ESP_LOGE(TAG, "Verifica fallita dopo la scrittura della partition table!");
        return ESP_FAIL;
    }

    int verified_parts = 0;
    err = esp_partition_table_verify((const esp_partition_info_t *)readback_buf, true, &verified_parts);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Integrità nuova partition table non valida: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Partition table in flash aggiornata con successo (%d partizioni verificate).", verified_parts);

    // Registrazione dinamica a runtime per le partizioni non presenti nella RAM list di boot (es. coredump)
    for (int i = 0; i < emb_num_parts; i++) {
        const esp_partition_info_t *p = &emb_table[i];
        if (p->magic != ESP_PARTITION_MAGIC) {
            continue;
        }

        char label[17] = {0};
        memcpy(label, p->label, sizeof(p->label));

        const esp_partition_t *existing = esp_partition_find_first(
            (esp_partition_type_t)p->type,
            (esp_partition_subtype_t)p->subtype,
            label
        );

        if (existing == NULL) {
            const esp_partition_t *reg_part = NULL;
            err = esp_partition_register_external(
                NULL,
                p->pos.offset,
                p->pos.size,
                label,
                (esp_partition_type_t)p->type,
                (esp_partition_subtype_t)p->subtype,
                &reg_part
            );
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Partizione '%s' registrata dinamicamente a runtime: offset 0x%08" PRIx32 ", dim 0x%08" PRIx32,
                         label, p->pos.offset, p->pos.size);
                if (p->type == PART_TYPE_DATA && p->subtype == 0x03) { // coredump
                    esp_core_dump_init();
                    esp_err_t cd_err = esp_core_dump_image_check();
                    if (cd_err != ESP_OK && cd_err != ESP_ERR_NOT_FOUND) {
                        ESP_LOGI(TAG, "Inizializzazione pulita spazio coredump...");
                        esp_core_dump_image_erase();
                    }
                }
            } else {
                ESP_LOGW(TAG, "Impossibile registrare la partizione '%s' a runtime: %s", label, esp_err_to_name(err));
            }
        }
    }

    return ESP_OK;
}

#endif // LINUX_PLATFORM
