#ifndef MEU_FILE_STORAGE_H
#define MEU_FILE_STORAGE_H
#include "core/snapshot.h"
#define MEU_PATH_BYTES 4096
typedef struct {
    bool present;
    size_t size;
    uint8_t data[MEU_SAVE_BYTES+1];
    MeuSaveResult result;
    uint32_t generation;
} MeuSaveRecord;
typedef struct { MeuSaveRecord latest,previous; } MeuJournal;
bool meu_file_mkdir(const char *directory);
bool meu_file_path(const char *directory,const char *name,char output[MEU_PATH_BYTES]);
bool meu_file_inspect(const char *directory,MeuJournal *journal);
MeuSaveResult meu_file_save(const char *directory,const MeuSimulation *state,uint32_t *generation);
MeuSaveResult meu_file_load(const char *directory,bool previous,MeuSimulation *state,uint32_t *generation);
MeuSaveResult meu_file_export(const char *path,const MeuSimulation *state);
MeuSaveResult meu_file_import(const char *path,MeuSimulation *state,uint32_t *generation);
#ifdef MEU_STORAGE_TESTING
enum { MEU_FILE_FAIL_WRITE=1,MEU_FILE_FAIL_FLUSH,MEU_FILE_FAIL_REPLACE,MEU_FILE_FAIL_AFTER_BACKUP };
void meu_file_test_fail(int phase);
#endif
#endif
