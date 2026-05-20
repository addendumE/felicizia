#include <Gcm.hpp>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_random.h"
#include "mbedtls/aes.h"
#include "mbedtls/gcm.h"

static const char *TAG="GCM";

// Funzione di utilità per stampare i buffer in formato Hex nel terminale
void Gcm::print_hex(const char *label, const unsigned char *buf, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

Gcm::Gcm()
{

    aes_key.resize(AES_KEY_BYTES);
}
Gcm::~Gcm()
{

}   

#define GET_IV(vec) (&(vec)[0])
#define GET_TAG(vec) (&(vec)[IV_LEN])
#define GET_CIPHERTEXT(vec) (&(vec)[IV_LEN+TAG_LEN])


std::vector<unsigned char> Gcm::encrypt(const std::string &payload)
{
    std::vector<unsigned char> retvect(IV_LEN+TAG_LEN+payload.size());

    esp_fill_random(GET_IV(retvect), IV_LEN);
    mbedtls_gcm_context gcm_encrypt_ctx;
    mbedtls_gcm_init(&gcm_encrypt_ctx);
    int ret = mbedtls_gcm_setkey(&gcm_encrypt_ctx, MBEDTLS_CIPHER_ID_AES, aes_key.data(), 256);
    if (ret != 0) {
        ESP_LOGE(TAG, "Errore mbedtls_gcm_setkey (cifratura): %d", ret);
        return retvect;
    }
    ret = mbedtls_gcm_crypt_and_tag(&gcm_encrypt_ctx, MBEDTLS_GCM_ENCRYPT, payload.size(),
                                    GET_IV(retvect), IV_LEN, NULL, 0,
                                    (const unsigned char*)payload.data(), GET_CIPHERTEXT(retvect),
                                    TAG_LEN, GET_TAG(retvect));    
    if (ret == 0) {
        ESP_LOGI(TAG, "Cifratura completata con successo!");
        print_hex("IV (Hex)        ", GET_IV(retvect), IV_LEN);
        print_hex("Ciphertext (Hex)", GET_CIPHERTEXT(retvect), payload.size());
        print_hex("Tag (Hex)       ", GET_TAG(retvect), TAG_LEN);
    } else {
        ESP_LOGE(TAG, "Errore durante la cifratura: %d", ret);
        mbedtls_gcm_free(&gcm_encrypt_ctx);
        return retvect;
    }
    mbedtls_gcm_free(&gcm_encrypt_ctx);
    return retvect;
}


std::string Gcm::decrypt( const std::vector<unsigned char> &payload)
{
    size_t payload_len = payload.size()-TAG_LEN-IV_LEN;
    unsigned char decryptedtext[payload_len+1];
    mbedtls_gcm_context gcm_decrypt_ctx;
    mbedtls_gcm_init(&gcm_decrypt_ctx);

    int ret = mbedtls_gcm_setkey(&gcm_decrypt_ctx, MBEDTLS_CIPHER_ID_AES, aes_key.data(), 256);
    if (ret != 0) {
        ESP_LOGE(TAG, "Errore mbedtls_gcm_setkey (decrittografia): %d", ret);
        return "";
    }
    ret = mbedtls_gcm_auth_decrypt(&gcm_decrypt_ctx, payload_len,
                                   GET_IV(payload), IV_LEN, NULL, 0,
                                   GET_TAG(payload), TAG_LEN,
                                   GET_CIPHERTEXT(payload), decryptedtext);

    if (ret == 0) {
        decryptedtext[payload_len] = '\0'; // Stringa terminata correttamente
        ESP_LOGI(TAG, "AUTENTICAZIONE INTEGRITÀ OK! Il messaggio non è stato manomesso.");
        ESP_LOGI(TAG, "Payload decifrato: %s", (char *)decryptedtext);
    } else {
        ESP_LOGE(TAG, "ERRORE DI AUTENTICAZIONE! Il payload o il tag sono stati modificati maliziosamente. Codice: %d", ret);
    }
    mbedtls_gcm_free(&gcm_decrypt_ctx);
    return std::string((char*)decryptedtext);
}