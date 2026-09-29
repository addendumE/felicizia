#include "JsonHelper.h"
#include <cstring>

namespace JsonHelper {

std::string print(cJSON *item, size_t initial_size, size_t max_size)
{
    std::string out;
    if (!item) {
        return out;
    }

    size_t current_size = initial_size;
    if (current_size < 64) {
        current_size = 64;
    }

    while (current_size <= max_size) {
        out.resize(current_size);

        if (cJSON_PrintPreallocated(item, &out[0], (int)current_size, 0)) {
            out.resize(strlen(out.c_str()));
            return out;
        }

        if (current_size > max_size / 2) {
            if (current_size == max_size) {
                break;
            }
            current_size = max_size;
        } else {
            current_size *= 2;
        }
    }

    // Fallback di sicurezza: se supera max_size evita il blocco
    char *txt = cJSON_PrintUnformatted(item);
    if (txt) {
        out = txt;
        free(txt);
    } else {
        out.clear();
    }
    return out;
}

} // namespace JsonHelper
