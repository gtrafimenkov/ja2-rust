#ifndef __FILEMAN_H
#define __FILEMAN_H

#include "SGP/Types.h"

// This is a handle to an open file.
// The file can be a regular file on a disk, or a file from an slf archive.
typedef UINT32 HWFILE;

#define FILE_ACCESS_READ 0x01
#define FILE_ACCESS_WRITE 0x02
#define FILE_ACCESS_READWRITE 0x03

#define FILE_CREATE_NEW 0x0010         // create new file. fail if exists
#define FILE_CREATE_ALWAYS 0x0020      // create new file. overwrite existing
#define FILE_OPEN_EXISTING 0x0040      // open a file. fail if doesn't exist
#define FILE_OPEN_ALWAYS 0x0080        // open a file, create if doesn't exist
#define FILE_TRUNCATE_EXISTING 0x0100  // open a file, truncate to size 0. fail if no exist

#define FILE_SEEK_FROM_START 0x01
#define FILE_SEEK_FROM_END 0x02
#define FILE_SEEK_FROM_CURRENT 0x04

extern BOOLEAN FileMan_Initialize();
extern void FileMan_Shutdown(void);

extern BOOLEAN FileMan_Exists(STR strFilename);
extern BOOLEAN FileMan_ExistsNoDB(STR strFilename);
extern BOOLEAN FileMan_Delete(STR strFilename);
extern HWFILE FileMan_Open(STR strFilename, UINT32 uiOptions, BOOLEAN fDeleteOnClose);
extern void FileMan_Close(HWFILE);

extern BOOLEAN FileMan_Read(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead);
extern BOOLEAN FileMan_Write(HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite,
                             UINT32 *puiBytesWritten);

extern BOOLEAN FileMan_Seek(HWFILE, UINT32 uiDistance, UINT8 uiHow);
extern INT32 FileMan_GetPos(HWFILE);

// returns true if at end of file, else false
BOOLEAN FileMan_CheckEndOfFile(HWFILE hFile);

extern UINT32 FileMan_GetSize(HWFILE);
extern UINT32 FileMan_Size(STR strFilename);

BOOLEAN FileMan_GetFileWriteTime(HWFILE hFile, uint64_t *pLastWriteTime);

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

#endif
