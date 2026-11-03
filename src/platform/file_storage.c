#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "platform/file_storage.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HANDLE FileLock;
#define INVALID_LOCK INVALID_HANDLE_VALUE
static bool wide(const char *text,wchar_t output[MEU_PATH_BYTES])
{
    return text && MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,output,MEU_PATH_BYTES)>0;
}
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
typedef int FileLock;
#define INVALID_LOCK (-1)
#endif
#ifdef MEU_STORAGE_TESTING
static int fault;
void meu_file_test_fail(int phase) { fault=phase; }
static bool checkpoint(int phase) { return fault!=phase; }
#else
static bool checkpoint(int phase) { (void)phase; return true; }
#endif
bool meu_file_path(const char *dir,const char *name,char out[MEU_PATH_BYTES])
{
    if (!dir || !*dir || !name) return false;
    size_t length=strlen(dir);
    bool separated=dir[length-1]=='/';
#ifdef _WIN32
    separated|=dir[length-1]=='\\';
#endif
    const char *separator=separated?"":"/";
    int n=snprintf(out,MEU_PATH_BYTES,"%s%s%s",dir,separator,name);
    return n>0 && n<MEU_PATH_BYTES;
}
bool meu_file_mkdir(const char *dir)
{
#ifdef _WIN32
    wchar_t path[MEU_PATH_BYTES]; if (!wide(dir,path)) return false;
    if (CreateDirectoryW(path,NULL)) return true;
    DWORD attrs=GetFileAttributesW(path);
    return attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY)!=0;
#else
    if (!dir || !*dir) return false;
    if (mkdir(dir,0700)==0) return true;
    struct stat info;
    return errno==EEXIST && stat(dir,&info)==0 && S_ISDIR(info.st_mode);
#endif
}
/* Read one byte beyond the format bound so oversized data cannot be truncated
   into an apparently valid snapshot. The original file is never edited. */
static bool read_record(const char *path,MeuSaveRecord *r)
{
    memset(r,0,sizeof(*r)); r->result=MEU_SAVE_MALFORMED;
#ifdef _WIN32
    wchar_t name[MEU_PATH_BYTES]; if (!wide(path,name)) return false;
    HANDLE f=CreateFileW(name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (f==INVALID_HANDLE_VALUE) return GetLastError()==ERROR_FILE_NOT_FOUND || GetLastError()==ERROR_PATH_NOT_FOUND;
    r->present=true; DWORD count=0;
    bool ok=ReadFile(f,r->data,sizeof(r->data),&count,NULL)!=0;
    CloseHandle(f); if (!ok) return false; r->size=count;
#else
    int f=open(path,O_RDONLY|O_CLOEXEC);
    if (f<0) return errno==ENOENT;
    r->present=true; bool ok=true;
    while (r->size<sizeof(r->data)) {
        ssize_t n=read(f,r->data+r->size,sizeof(r->data)-r->size);
        if (n<0 && errno==EINTR) continue;
        if (n<0) { ok=false; break; }
        if (!n) break;
        r->size+=(size_t)n;
    }
    if (close(f)!=0) ok=false;
    if (!ok) return false;
#endif
    r->result=meu_snapshot_decode(r->data,r->size,NULL,&r->generation); return true;
}
bool meu_file_inspect(const char *dir,MeuJournal *journal)
{
    MeuJournal staged; char path[MEU_PATH_BYTES];
    if (!meu_file_path(dir,"latest.meu",path) || !read_record(path,&staged.latest) ||
        !meu_file_path(dir,"previous.meu",path) || !read_record(path,&staged.previous)) return false;
    *journal=staged; return true;
}
static bool sync_directory(const char *dir)
{
#ifdef _WIN32
    (void)dir; return true; /* MoveFileExW below requests write-through publication. */
#else
    int f=open(dir,O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    if (f<0) return false;
    bool ok=fsync(f)==0; if (close(f)!=0) ok=false; return ok;
#endif
}
static bool replace(const char *source,const char *target,const char *dir)
{
#ifdef _WIN32
    wchar_t a[MEU_PATH_BYTES],b[MEU_PATH_BYTES];
    if (!wide(source,a) || !wide(target,b)) return false;
    if (!MoveFileExW(a,b,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) return false;
#else
    if (rename(source,target)!=0) return false;
#endif
    return sync_directory(dir);
}
static void discard(const char *path)
{
#ifdef _WIN32
    wchar_t name[MEU_PATH_BYTES]; if (wide(path,name)) DeleteFileW(name);
#else
    unlink(path);
#endif
}
static bool publish(const char *path,const char *dir,const uint8_t data[MEU_SAVE_BYTES])
{
    char pending[MEU_PATH_BYTES];
    int n=snprintf(pending,sizeof(pending),"%s.pending",path);
    if (n<0 || n>=(int)sizeof(pending) || !checkpoint(1)) return false;
    bool ok=false;
#ifdef _WIN32
    wchar_t name[MEU_PATH_BYTES]; if (!wide(pending,name)) return false;
    HANDLE f=CreateFileW(name,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (f==INVALID_HANDLE_VALUE) return false;
    DWORD written=0;
    ok=WriteFile(f,data,MEU_SAVE_BYTES,&written,NULL)!=0 && written==MEU_SAVE_BYTES;
    if (ok) ok=checkpoint(2) && FlushFileBuffers(f)!=0;
    if (!CloseHandle(f)) ok=false;
#else
    int f=open(pending,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);
    if (f<0) return false;
    size_t written=0;
    while (written<MEU_SAVE_BYTES) {
        ssize_t count=write(f,data+written,MEU_SAVE_BYTES-written);
        if (count<0 && errno==EINTR) continue;
        if (count<=0) break;
        written+=(size_t)count;
    }
    ok=written==MEU_SAVE_BYTES && checkpoint(2) && fsync(f)==0;
    if (close(f)!=0) ok=false;
#endif
    if (ok) ok=checkpoint(3) && replace(pending,path,dir);
    if (!ok) discard(pending);
    return ok;
}
static FileLock lock_journal(const char *dir)
{
    char path[MEU_PATH_BYTES]; if (!meu_file_path(dir,"journal.lock",path)) return INVALID_LOCK;
#ifdef _WIN32
    wchar_t name[MEU_PATH_BYTES]; if (!wide(path,name)) return INVALID_LOCK;
    return CreateFileW(name,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#else
    int f=open(path,O_RDWR|O_CREAT|O_CLOEXEC,0600);
    if (f<0) return INVALID_LOCK;
    struct flock lock={0}; lock.l_type=F_WRLCK; lock.l_whence=SEEK_SET;
    if (fcntl(f,F_SETLK,&lock)<0) { close(f); return INVALID_LOCK; }
    return f;
#endif
}
static void unlock_journal(FileLock lock)
{
#ifdef _WIN32
    CloseHandle(lock);
#else
    close(lock);
#endif
}
static bool quarantine(const char *dir,const char *name,const char *bad)
{
    char from[MEU_PATH_BYTES],to[MEU_PATH_BYTES];
    return meu_file_path(dir,name,from) && meu_file_path(dir,bad,to) && replace(from,to,dir);
}
MeuSaveResult meu_file_save(const char *dir,const MeuSimulation *s,uint32_t *generation)
{
    if (!meu_file_mkdir(dir)) return MEU_SAVE_IO;
    FileLock lock=lock_journal(dir); if (lock==INVALID_LOCK) return MEU_SAVE_IO;
    MeuSaveResult result=MEU_SAVE_IO; MeuJournal journal;
    if (meu_file_inspect(dir,&journal)) {
        uint32_t latest=journal.latest.result==MEU_SAVE_OK?journal.latest.generation:0;
        uint32_t previous=journal.previous.result==MEU_SAVE_OK?journal.previous.generation:0;
        uint32_t next=(latest>previous?latest:previous)+1;
        uint8_t data[MEU_SAVE_BYTES]; char path[MEU_PATH_BYTES];
        result=meu_snapshot_encode(s,next,data);
        if (result==MEU_SAVE_OK) {
            bool ok=true;
            if (journal.latest.present && journal.latest.result!=MEU_SAVE_OK)
                ok=quarantine(dir,"latest.meu","damaged-latest.meu");
            if (ok && journal.previous.present && journal.previous.result!=MEU_SAVE_OK)
                ok=quarantine(dir,"previous.meu","damaged-previous.meu");
            if (ok && latest) ok=meu_file_path(dir,"previous.meu",path) && publish(path,dir,journal.latest.data);
            if (ok) ok=checkpoint(4) && meu_file_path(dir,"latest.meu",path) && publish(path,dir,data);
            if (ok) { if (generation) *generation=next; }
            else result=MEU_SAVE_IO;
        }
    }
    unlock_journal(lock); return result;
}
MeuSaveResult meu_file_load(const char *dir,bool previous,MeuSimulation *s,uint32_t *generation)
{
    MeuJournal journal; if (!meu_file_inspect(dir,&journal)) return MEU_SAVE_IO;
    const MeuSaveRecord *r=previous?&journal.previous:&journal.latest;
    if (!r->present) return MEU_SAVE_MISSING;
    return meu_snapshot_restore(s,r->data,r->size,generation);
}
static bool parent_directory(const char *path,char dir[MEU_PATH_BYTES])
{
    if (!path || !*path || strlen(path)>=MEU_PATH_BYTES) return false;
    snprintf(dir,MEU_PATH_BYTES,"%s",path);
    char *a=strrchr(dir,'/'),*separator=a;
#ifdef _WIN32
    char *b=strrchr(dir,'\\');
    if (b && (!a || b>a)) separator=b;
#endif
    if (separator) { if (separator==dir) separator[1]=0; else *separator=0; }
    else snprintf(dir,MEU_PATH_BYTES,".");
    return true;
}
MeuSaveResult meu_file_export(const char *path,const MeuSimulation *s)
{
    uint8_t data[MEU_SAVE_BYTES]; MeuSaveResult result=meu_snapshot_encode(s,1,data);
    if (result!=MEU_SAVE_OK) return result;
    char dir[MEU_PATH_BYTES];
    return parent_directory(path,dir) && publish(path,dir,data)?MEU_SAVE_OK:MEU_SAVE_IO;
}
MeuSaveResult meu_file_import(const char *path,MeuSimulation *s,uint32_t *generation)
{
    MeuSaveRecord record;
    if (!read_record(path,&record)) return MEU_SAVE_IO;
    if (!record.present) return MEU_SAVE_MISSING;
    return meu_snapshot_restore(s,record.data,record.size,generation);
}
