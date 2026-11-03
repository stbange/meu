#include "platform/file_storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#c); return EXIT_FAILURE; } } while (0)
static bool raw(const char *dir,const char *name,const uint8_t *data,size_t size)
{
    char path[MEU_PATH_BYTES];if (!meu_file_path(dir,name,path)) return false;
    FILE *file=fopen(path,"wb");if (!file) return false;
    bool ok=fwrite(data,1,size,file)==size;if (fclose(file)!=0) ok=false;return ok;
}
int main(int argc,char **argv)
{
    CHECK(argc==2 && meu_file_mkdir(argv[1]));const char *dir=argv[1];
    const char *names[]={"latest.meu","previous.meu","latest.meu.pending","previous.meu.pending","damaged-latest.meu","damaged-previous.meu","export.meu","future.meu"};
    char path[MEU_PATH_BYTES];
    for (size_t i=0;i<sizeof(names)/sizeof(names[0]);++i) { CHECK(meu_file_path(dir,names[i],path));remove(path); }
    MeuSimulation s;meu_sim_init(&s);MeuJournal journal;uint32_t generation=99;
    CHECK(meu_file_inspect(dir,&journal) && !journal.latest.present && !journal.previous.present);
    CHECK(meu_file_load(dir,false,&s,&generation)==MEU_SAVE_MISSING && generation==99);
    CHECK(meu_file_save(dir,&s,&generation)==MEU_SAVE_OK && generation==1);
    s.x=.5f;s.settings.sensitivity=2;s.settings.invert_y=true;s.settings.volume=.25f;
    CHECK(meu_file_save(dir,&s,&generation)==MEU_SAVE_OK && generation==2);
    CHECK(meu_file_inspect(dir,&journal) && journal.latest.generation==2 && journal.previous.generation==1);
    uint8_t confirmed[MEU_SAVE_BYTES];memcpy(confirmed,journal.latest.data,sizeof(confirmed));
    MeuSimulation fresh;meu_sim_init(&fresh);
    CHECK(meu_file_load(dir,false,&fresh,&generation)==MEU_SAVE_OK && generation==2);
    CHECK(fresh.x==.5f && fresh.settings.sensitivity==2 && fresh.settings.invert_y && fresh.settings.volume==.25f && fresh.paused && fresh.travel.parked);
    for (int fault=MEU_FILE_FAIL_WRITE;fault<=MEU_FILE_FAIL_AFTER_BACKUP;++fault) {
        meu_file_test_fail(fault);generation=99;
        CHECK(meu_file_save(dir,&s,&generation)==MEU_SAVE_IO && generation==99);
        meu_file_test_fail(0);
        CHECK(meu_file_inspect(dir,&journal) && journal.latest.result==MEU_SAVE_OK && memcmp(journal.latest.data,confirmed,sizeof(confirmed))==0);
    }
    /* Rebuild two distinct complete generations, then damage the latest. */
    s.x=1;CHECK(meu_file_save(dir,&s,&generation)==MEU_SAVE_OK && generation==3);
    CHECK(meu_file_inspect(dir,&journal) && journal.previous.generation==2);
    uint8_t damaged[MEU_SAVE_BYTES];memcpy(damaged,journal.latest.data,sizeof(damaged));damaged[56]^=1;
    CHECK(raw(dir,"latest.meu",damaged,sizeof(damaged)));
    MeuSimulation unchanged=fresh;generation=99;
    CHECK(meu_file_load(dir,false,&fresh,&generation)==MEU_SAVE_MALFORMED && generation==99 && memcmp(&fresh,&unchanged,sizeof(fresh))==0);
    CHECK(meu_file_load(dir,true,&fresh,&generation)==MEU_SAVE_OK && generation==2 && fresh.x==.5f);
    CHECK(meu_file_inspect(dir,&journal) && memcmp(journal.latest.data,damaged,sizeof(damaged))==0);
    meu_file_test_fail(MEU_FILE_FAIL_WRITE);generation=99;
    CHECK(meu_file_save(dir,&fresh,&generation)==MEU_SAVE_IO && generation==99);
    meu_file_test_fail(0);
    CHECK(meu_file_inspect(dir,&journal) && !journal.latest.present && journal.previous.generation==2);
    CHECK(meu_file_load(dir,true,&fresh,&generation)==MEU_SAVE_OK && generation==2);
    CHECK(meu_file_save(dir,&fresh,&generation)==MEU_SAVE_OK && generation==3);
    CHECK(meu_file_path(dir,"damaged-latest.meu",path));FILE *file=fopen(path,"rb");CHECK(file!=NULL);
    uint8_t kept[MEU_SAVE_BYTES];CHECK(fread(kept,1,sizeof(kept),file)==sizeof(kept));fclose(file);
    CHECK(memcmp(kept,damaged,sizeof(kept))==0);
    CHECK(meu_file_path(dir,"export.meu",path) && meu_file_export(path,&fresh)==MEU_SAVE_OK);
    s.x=0;CHECK(meu_file_import(path,&s,&generation)==MEU_SAVE_OK && s.x==fresh.x && s.settings.invert_y);
    memcpy(damaged,confirmed,sizeof(damaged));damaged[8]=2;
    CHECK(raw(dir,"future.meu",damaged,sizeof(damaged)) && meu_file_path(dir,"future.meu",path));
    unchanged=s;
    CHECK(meu_file_import(path,&s,&generation)==MEU_SAVE_SCHEMA_UNSUPPORTED && memcmp(&s,&unchanged,sizeof(s))==0);
    /* A complete but unpublished temporary file is never treated as committed. */
    uint8_t candidate[MEU_SAVE_BYTES];CHECK(meu_snapshot_encode(&s,99,candidate)==MEU_SAVE_OK);
    CHECK(raw(dir,"latest.meu.pending",candidate,sizeof(candidate)));
    CHECK(meu_file_inspect(dir,&journal) && journal.latest.generation==3);
    CHECK(meu_file_load(dir,false,&s,&generation)==MEU_SAVE_OK && generation==3);
    CHECK(meu_file_path(dir,"export.meu",path));generation=99;
    CHECK(meu_file_save(path,&s,&generation)==MEU_SAVE_IO && generation==99);
    CHECK(meu_file_path(dir,"unicode-\xe2\x98\x85.meu",path) && meu_file_export(path,&s)==MEU_SAVE_OK);
    CHECK(meu_file_import(path,&fresh,NULL)==MEU_SAVE_OK);
#ifndef _WIN32
    CHECK(meu_file_path(dir,"back\\slash.meu",path) && meu_file_export(path,&s)==MEU_SAVE_OK);
    CHECK(meu_file_import(path,&fresh,NULL)==MEU_SAVE_OK);
#endif
    printf("PASS %d native durable-storage assertions\n",checks);return EXIT_SUCCESS;
}
