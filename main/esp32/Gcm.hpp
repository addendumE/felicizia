#include <stdint.h>
#include <string>
#include <vector>

#define AES_KEY_BYTES 32
#define IV_LEN 32
#define TAG_LEN 16

class Gcm
{
public:
    Gcm();
    virtual ~Gcm();
    std::vector<unsigned char> encrypt(const std::string &);
    std::string decrypt(const std::vector<unsigned char> &);
private:
    std::vector<unsigned char> aes_key;
    void print_hex(const char *label, const unsigned char *buf, size_t len);
};
