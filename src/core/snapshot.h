#ifndef MEU_SNAPSHOT_H
#define MEU_SNAPSHOT_H
#include "core/simulation.h"
#include <stddef.h>
#define MEU_SAVE_BYTES 256
#define MEU_SAVE_HEX_BYTES (MEU_SAVE_BYTES*2+1)
#define MEU_SAVE_SCHEMA 2
#define MEU_SAVE_WORLD 1
typedef enum {
    MEU_SAVE_OK, MEU_SAVE_MALFORMED, MEU_SAVE_SCHEMA_UNSUPPORTED,
    MEU_SAVE_WORLD_UNSUPPORTED, MEU_SAVE_UNSAFE, MEU_SAVE_BUSY, MEU_SAVE_IO, MEU_SAVE_MISSING
} MeuSaveResult;
/* Output buffers/state are untouched on rejection. No host struct is serialized. */
MeuSaveResult meu_snapshot_encode(const MeuSimulation *state,uint32_t generation,uint8_t output[MEU_SAVE_BYTES]);
MeuSaveResult meu_snapshot_decode(const uint8_t *data,size_t size,MeuSimulation *output,uint32_t *generation);
MeuSaveResult meu_snapshot_restore(MeuSimulation *state,const uint8_t *data,size_t size,uint32_t *generation);
void meu_snapshot_hex(const uint8_t data[MEU_SAVE_BYTES],char output[MEU_SAVE_HEX_BYTES]);
bool meu_snapshot_unhex(const char *text,uint8_t output[MEU_SAVE_BYTES]);
const char *meu_save_message(MeuSaveResult result);
#endif
