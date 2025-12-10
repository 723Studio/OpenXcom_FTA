/* tiny-AES-c lightweight AES implementation (public domain / MIT-style)
 * Minimal subset used: AES-256 CBC decrypt
 */
#ifndef TINY_AES_H
#define TINY_AES_H

#include <stdint.h>
#include <stddef.h>

struct AES_ctx {
    uint8_t RoundKey[240];
    uint8_t Iv[16];
};

#ifdef __cplusplus
extern "C" {
#endif

void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, size_t length);

#ifdef __cplusplus
}
#endif

#endif // TINY_AES_H
