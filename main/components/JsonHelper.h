#pragma once

#include <string>
#include "cJSON.h"

namespace JsonHelper {

/**
 * @brief Serializza un albero cJSON direttamente in una std::string senza allocazioni heap da parte di cJSON.
 * Utilizza cJSON_PrintPreallocated con raddoppio esponenziale del buffer in caso di spazio insufficiente.
 *
 * @param item L'oggetto cJSON da serializzare.
 * @param initial_size Dimensione iniziale stimata del buffer in byte (default 512).
 * @param max_size Dimensione massima consentita prima del fallback di sicurezza (default 8192).
 * @return std::string La stringa JSON serializzata (vuota in caso di errore).
 */
std::string print(cJSON *item, size_t initial_size = 512, size_t max_size = 8192);

} // namespace JsonHelper
