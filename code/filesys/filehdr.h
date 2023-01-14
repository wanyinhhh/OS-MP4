// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "pbitmap.h"

// Indirect
#define IndirectHeaderPerDoubleIndirect 32 // double Indirect can have 32 indirect header
#define SectorPerIndirectHeader 32 * 512 // each indirect header can have 32*512 sectors // 不確定是不是這樣
#define NumIndirect 1
// direct
#define NumDirect 	((SectorSize - 2 * sizeof(int)) / sizeof(int))
// Indirect + Direct 
// 1*32*32*128 + NumDirect * SectorSize > 128KB
// 1MB = 1024KB => 128KB -> 64MB => 64*1024/128=512 
#define MaxFileSize 	((NumDirect * SectorSize) + (IndirectHeaderPerDoubleIndirect * SectorPerIndirectHeader* SectorSize * NumIndirect))

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.


// double Indirect can have 32 indirect header
// point to 32 indirect header
class DoubleIndirect {
  public:
    // initial value
    DoubleIndirect();
    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header

    // array -> store sector number -> Fetch
		int numSectorsPointer[NumSectors];

};

// each indirect header can have 32*512 sector
// indirect header point to 32*512 sectors
class IndirectHeader{
 public:
    // initial value
    IndirectHeader();
    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
		int numSectorsPointer[NumSectors];

};

class FileHeader {
  public:
    // MP4 
    // initial value
    FileHeader();

    bool Allocate(PersistentBitmap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(PersistentBitmap *bitMap);  // De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    // MP4
    // Get next_hdr
    FileHeader* GetNextFileHeader() {
      if (next_hdr_datasector == -1) {
          return NULL;
      } else {
          return next_hdr;
      }
    }

    FileHeader* next_hdr;

    bool ExtendFileSize(PersistentBitmap *freeMap, int fileSize);

  private:
    // direct
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
					// block in the file
    
    // indirect
    int numIndirectSector;

    // find next  
    int next_numBytes;
    int next_numSectors;
    int next_hdr_datasector;
    
};

#endif // FILEHDR_H
