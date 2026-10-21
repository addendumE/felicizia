#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LINUX_PLATFORM

/**
 * @brief Verifica ed aggiorna la tabella delle partizioni in flash se necessario.
 *
 * Confronta la tabella delle partizioni attualmente presente in flash (offset 0x8000)
 * con la tabella compilata nell'eseguibile corrente (incorporata dal build system).
 * Se la tabella in flash differisce (es. dispositivo aggiornato via OTA da una versione precedente
 * che non conteneva la partizione 'coredump'):
 *  1. Disabilita temporaneamente la protezione contro la scrittura su regioni critiche di flash.
 *  2. Cancella il settore della tabella delle partizioni e scrive la nuova tabella.
 *  3. Riabilita la protezione di scrittura.
 *  4. Rilegge e verifica l'integrità della tabella scritta (tramite checksum MD5 ed header).
 *  5. Registra a runtime le partizioni mancanti (es. coredump) e re-inizializza il sottosistema
 *     coredump in modo che sia immediatamente fruibile senza richiedere un reboot immediato
 *     (evitando inoltre il rischio di rollback del bootloader OTA se in stato PENDING_VERIFY).
 *
 * @return ESP_OK se la tabella è aggiornata o era già aggiornata con successo.
 *         Un opportuno codice di errore esp_err_t in caso di problemi.
 */
esp_err_t update_partition_table(void);

#else

static inline esp_err_t update_partition_table(void) { return ESP_OK; }

#endif // LINUX_PLATFORM

#ifdef __cplusplus
}
#endif
