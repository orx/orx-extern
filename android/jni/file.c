#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apk_file.h"
#include <unistd.h>

typedef struct
{
    int     type;
    void   *handle;
} FileHandle;

enum
{
    APK_FILE,
    STD_FILE
};

void  FInit()
{
    APKInit();
}

File* FOpen(char const* path)
{
    FileHandle *handle = NULL;

    // First, try the given path with no mods...
    FILE *fp = fopen(path, "rb");

    // No luck...  Try prepending /data...
    if (!fp)
    {
        char buf[512];
        sprintf(buf, "/data/%s", path); //TODO: use storage card path in the future?
        fp = fopen(buf, "rb");
    }

    if (fp)
    {
        handle = (FileHandle*) malloc(sizeof(FileHandle));
        handle->type = STD_FILE;
        handle->handle = fp;
    }
    else
    {
        APKFile* apk = APKOpen(path);
        if (apk)
        {
            handle = (FileHandle*) malloc(sizeof(FileHandle));
            handle->type = APK_FILE;
            handle->handle = APKOpen(path);
        }
    }
    return (File*) handle;
}

void FClose(File* file)
{
    FileHandle *handle = (FileHandle*) file;
    if (!handle)
        return;

    if (handle->type != STD_FILE)
        APKClose(handle->handle);
    else
        fclose(handle->handle);
    free(handle);
}

void FChdir(const char* dir)
{
    (void)chdir(dir);
}

int FGetc(File *stream)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return -1;

    if (handle->type != STD_FILE)
        return APKGetc(handle->handle);
    else
        return fgetc(handle->handle);
}

char* FGets(char* s, int size, File* stream)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return NULL;

    if (handle->type != STD_FILE)
        return APKGets(s, size, handle->handle);
    else
        return fgets(s, size, handle->handle);
}

size_t FSize(File* stream)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return 0;

    if (handle->type != STD_FILE)
    {
        return APKSize(handle->handle);
    }
    else
    {
        int pos = ftell(handle->handle);
        int size = 0;
        fseek(handle->handle, 0, SEEK_END);
        size = ftell(handle->handle);
        fseek(handle->handle, pos, SEEK_SET);
        return size;
    }
}

long FSeek(File* stream, long offset, int type)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return 0;

    if (handle->type != STD_FILE)
        return APKSeek(handle->handle, offset, type);
    else
        return fseek(handle->handle, offset, type);
}

long FTell(File* stream)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return 0;

    if (handle->type != STD_FILE)
        return APKTell(handle->handle);
    else
        return ftell(handle->handle);
}

size_t FRead(void* ptr, size_t size, size_t nmemb, File* stream)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return 0;

    if (handle->type != STD_FILE)
        return APKRead(ptr, size, nmemb, handle->handle);
    else
        return fread(ptr, size, nmemb, handle->handle);
}

int FEOF(File *stream)
{
    FileHandle *handle = (FileHandle*) stream;

    if (!handle)
        return 0;

    if (handle->type != STD_FILE)
        return APKEOF(handle->handle);
    else
        return feof(((FILE*)handle->handle));
}
