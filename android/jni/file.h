#ifndef __INCLUDED_FILE_H
#define __INCLUDED_FILE_H

#if defined(__cplusplus)
extern "C"
{
#endif

/** @file file.h
  The File library is an abstraction library that makes it possible to
  automatically find files either in Android's /data tree or in the APK
  itself.  The library provides a unified FILE*-like interface to both of
  the file types.  This makes it possible for native applications to migrate
  data between the APK and the /data filesystem without changing application code
  The library uses the apk_file.h system as a part of its implementation
  @see apk_file.h
*/
#include <stdio.h>

/**
 An opaque handle to a file, either in the APK or in the filesystem
 */
typedef void File;

/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  thread-created native threads cannot call this
  initialization function.  Calls APKInit() internally.
  @see apk_file.h
  @see thread.h
  */
void        FInit();

/**
  A wrapper similar to fopen.  Only provides read-access to files, since the
  file returned may be in the read-only APK.  Can be called from any 
  JNI-connected thread.
  @param path The path to the file.  This path is searched within /data and 
  within the APK
  @return A pointer to an open file on success, NULL on failure
  */
File*     FOpen(char const* path);

/**
  A wrapper similar to fclose.  Can be called from any JNI-connected thread.
  @param file A pointer to an open file opened via FOpen()
  */
void        FClose(File* file);

/**
  A wrapper similar to chdir.  Can be called from any thread.
  @param dir String path to be made current
  */
void        FChdir(const char* dir);


/**
  A wrapper similar to fgetc.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via FOpen()
  @return The character read from the file
  */
int         FGetc(File *stream);


/**
  A wrapper similar to fgets.  Can be called from any JNI-connected thread.
  @param s A char buffer to receive the string
  @param size The size of the buffer pointed to by s
  @param stream A pointer to an open file opened via FOpen()
  @return A pointer to s on success or NULL on failure
  */
char*       FGets(char* s, int size, File* stream);

/**
  Gets the size of the file in bytes.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via FOpen()
  @return The size of the file in bytes
  */
size_t      FSize(File* stream);

/**
  A wrapper equivalent to fseek.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via FOpen()
  @param offset The offset from the specified base
  @param type The offset base; same as calls to fseek
  @return Zero on success, nonzero on failure
  */
long        FSeek(File* stream, long offset, int type);

/**
  Gets the current file pointer offset.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via FOpen()
  @return The offset of the file pointer in the file in bytes
  */
long        FTell(File* stream);

/**
  A wrapper similar to fread.  Can be called from any JNI-connected thread.
  @param ptr A buffer of size size into which data will be read
  @param size size of element to be read
  @param nmemb count of elements to be read
  @param stream A pointer to an open file opened via FOpen()
  @return The number of elements read
  */
size_t      FRead(void* ptr, size_t size, size_t nmemb, File* stream);

/**
  A wrapper similar to feof.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via FOpen()
  @return Nonzero on EOF, zero otherwise
  */
int         FEOF(File *stream);

#if defined(__cplusplus)
}
#endif


#endif
