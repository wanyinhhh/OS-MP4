// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

// DoubleIndirect------------------------------------------------------
DoubleIndirect::DoubleIndirect()
{
    // memset(numSectorsPointer, -1, sizeof(numSectorsPointer));
}
//----------------------------------------------------------------------
// DoubleIndirect::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
DoubleIndirect::FetchFrom(int sector)
{   
    DEBUG(dbgFile, "DoubleIndirect::FetchFrom");
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// DoubleIndirect::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
DoubleIndirect::WriteBack(int sector)
{   
    DEBUG(dbgFile, "DoubleIndirect::WriteBack");
    kernel->synchDisk->WriteSector(sector, (char *)this); 
}





// IndirectHeader--------------------------------------------------------
IndirectHeader::IndirectHeader()
{
    memset(numSectorsPointer, -1, sizeof(numSectorsPointer));
}
//----------------------------------------------------------------------
// DoubleIndirect::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
IndirectHeader::FetchFrom(int sector)
{   
    DEBUG(dbgFile, "DoubleIndirect::FetchFrom");
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// DoubleIndirect::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
IndirectHeader::WriteBack(int sector)
{   
    DEBUG(dbgFile, "DoubleIndirect::WriteBack");
    kernel->synchDisk->WriteSector(sector, (char *)this); 
}



//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

FileHeader::FileHeader()
{
    // MP4 
    // initial value
    numBytes = -1;
    numSectors = -1;
    // next_numBytes = -1;
    // next_numSectors = -1;
    // next_hdr_datasector = -1;
    // numIndirectSector = -1;
    next_hdr = NULL;
}

bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{ 
    // if (fileSize > MaxFileSize) { // find next
    //     numBytes = MaxFileSize;
    //     next_numBytes = fileSize - MaxFileSize;
    //     numSectors = divRoundUp(numBytes, SectorSize); 
    //     next_numSectors = divRoundUp(next_numBytes, SectorSize);
    // } else { // Only one
    //     numBytes = fileSize;
    //     next_numBytes = -1;
    //     numSectors  = divRoundUp(numBytes, SectorSize);
    //     next_numSectors = -1;
    // }

    // // numSectors  = divRoundUp(fileSize, SectorSize);
    // if (freeMap->NumClear() < numSectors)
	//     return FALSE;		// not enough space


    // for (int i = 0; i < numSectors; i++) {
    //     dataSectors[i] = freeMap->FindAndSet();
    //     // since we checked that there was enough free space,
    //     // we expect this to succeed
    //     ASSERT(dataSectors[i] >= 0);
    // }

    // // Allocate next fileheader 
    // if (next_numBytes > 0) {
    //     next_hdr_datasector = freeMap->FindAndSet();
    //         if (next_hdr_datasector == -1) { // no free block space
    //             return FALSE;
    //         } else {
    //             next_hdr = new FileHeader;
    //             return next_hdr->Allocate(freeMap, next_numBytes);
    //         }
    // }
    // return TRUE;

    numIndirectSector == -1;
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	    return FALSE;		// not enough space
    
    // the number of sector is allocated
    int sectorAllocated = 0;
    // Direct is enough
    if (numSectors < NumDirect) {
        DEBUG(dbgFile, "Allocate: Direct Table is enough.");
        for (int i = 0; i < numSectors; i++) {
            dataSectors[i] = freeMap->FindAndSet();
        // since we checked that there was enough free space,
        // we expect this to succeed
            ASSERT(dataSectors[i] >= 0);
        }
    } else { // use Indirect
        DEBUG(dbgFile, "Allocate: use Direct Table and Indirect Table.");
        // First, use Direct Table
        DEBUG(dbgFile, "Allocate: use Direct Table.");
        for (int i = 0; i < NumDirect; i++) { 
            dataSectors[i] = freeMap->FindAndSet();
            ASSERT(dataSectors[i] >= 0);
        }
        sectorAllocated = NumDirect;
        DEBUG(dbgFile, "Allocate: use Indirect Table.");
        // Second, use Indirect Table
        DoubleIndirect *db_Indirect = new DoubleIndirect();
        numIndirectSector = freeMap->FindAndSet();
        // Third, allocate indirect header
        for (int i = 0; i < IndirectHeaderPerDoubleIndirect; i++) {
            if (sectorAllocated < numSectors) {
                DEBUG(dbgFile, "Allocated: sector for Double Indirect table.");
                IndirectHeader *indir_Header = new IndirectHeader();
                db_Indirect->numSectorsPointer[i] = freeMap->FindAndSet();
                for (int j = 0; j < SectorPerIndirectHeader; j++) {
                    if (sectorAllocated < numSectors) {
                        DEBUG(dbgFile, "Allocated: sector for Indirect Header.");
                        indir_Header->numSectorsPointer[j] = freeMap->FindAndSet();
                        sectorAllocated++; 
                    }
                }
                int indir_Header_sector = db_Indirect->numSectorsPointer[i];
                indir_Header->WriteBack(indir_Header_sector);
                delete indir_Header;
            }

        }
        db_Indirect->WriteBack(numIndirectSector);
        delete db_Indirect;
    }
    return TRUE;

}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(PersistentBitmap *freeMap)
{
    // if (next_hdr_datasector == -1) {  // only one
    //     for (int i = 0; i < numSectors; i++) {
    //         ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    //         freeMap->Clear((int) dataSectors[i]);
    //     }
    // } else { 
    //     for (int i = 0; i < numSectors; i++) {
    //         ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    //         freeMap->Clear((int) dataSectors[i]);
    //     }
    //     next_hdr->Deallocate(freeMap);
    // }

    if (numSectors < NumDirect) { 
        DEBUG(dbgFile, "Deallocate: Direct Table.");
        for (int i = 0; i < numSectors; i++) {
            ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }
    } else {
        // First, Deallocate Direct Table
        DEBUG(dbgFile, "Deallocate: Direct Table.");
        for (int i = 0; i < NumDirect; i++) { 
            dataSectors[i] = freeMap->FindAndSet();
            ASSERT(dataSectors[i] >= 0);
        }
        // Second, Double Indirect Table & Indirect Header
        // need to FetchFrom the sector and Clear the freeMap
        DEBUG(dbgFile, "Deallocate: Double Indirect Table & Indirect Header.");
        DoubleIndirect *db_Indirect = new DoubleIndirect();
        db_Indirect->FetchFrom(numIndirectSector);
        for (int i = 0; i < IndirectHeaderPerDoubleIndirect; i++) {
            if (db_Indirect->numSectorsPointer[i] != -1) {
                IndirectHeader *indir_Header = new IndirectHeader();
                indir_Header->FetchFrom(db_Indirect->numSectorsPointer[i]);
                for (int j = 0; j < SectorPerIndirectHeader; j++) {
                    if (indir_Header->numSectorsPointer[j] != -1) {
                        ASSERT(freeMap->Test((int)indir_Header->numSectorsPointer[j]));  // ought to be marked!
                        freeMap->Clear((int)indir_Header->numSectorsPointer[j]);
                        // initialize the array, set to -1
                        indir_Header->numSectorsPointer[j] = -1;
                    }
                }
                ASSERT(freeMap->Test((int)db_Indirect->numSectorsPointer[i]));  // ought to be marked!
                freeMap->Clear((int)db_Indirect->numSectorsPointer[i]);
                // initialize the array, set to -1
                db_Indirect->numSectorsPointer[i] = -1;
                delete indir_Header;
            }
        }
        ASSERT(freeMap->Test((int)numIndirectSector));  // ought to be marked!
        freeMap->Clear((int)numIndirectSector);
        // initialize the array, set to -1
        numIndirectSector = -1;
        delete db_Indirect;

    }

}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{   
    // if (next_hdr_datasector == -1) { // only one
    //     kernel->synchDisk->ReadSector(sector, (char *)this + sizeof(FileHeader*));    
    // } else {
    //     kernel->synchDisk->ReadSector(sector, (char *)this + sizeof(FileHeader*));
    //     next_hdr = new FileHeader;
    //     next_hdr->FetchFrom(next_hdr_datasector);
    // }
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{   
    DEBUG(dbgFile, "Write the modified contents of the file header back to disk.");
    kernel->synchDisk->WriteSector(sector, (char *)this); 
    // if (next_hdr_datasector != -1) {
    //     next_hdr->WriteBack(next_hdr_datasector); 
    // }
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    int sector_index = offset / SectorSize;
    if (sector_index < NumDirect) { // direct is enough
        return(dataSectors[sector_index]);
    } else {
        // return next_hdr->ByteToSector((sector_index-NumDirect)*SectorSize);
        // fetch both double indirect and indirect header
        DoubleIndirect *db_Indirect = new DoubleIndirect();
        // cout << "4444444\n";
        // cout << "numIndirectSector : " << numIndirectSector <<'\n';
        db_Indirect->FetchFrom(numIndirectSector);
        IndirectHeader *indir_Header = new IndirectHeader();
        indir_Header->FetchFrom(db_Indirect->numSectorsPointer[sector_index / IndirectHeaderPerDoubleIndirect]);

        int result = indir_Header->numSectorsPointer[sector_index % SectorPerIndirectHeader]; 

        delete db_Indirect;
        delete indir_Header;
        // cout << "result :" << result << '\n';
        ASSERT(result >= 0);
        return result;
    }
    // return(dataSectors[offset / SectorSize]);
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	    printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");

    for (i = k = 0; i < numSectors; i++) {
        kernel->synchDisk->ReadSector(dataSectors[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                    printf("%c", data[j]);
                else
                    printf("\\%x", (unsigned char)data[j]);
            }
            printf("\n"); 
    }
    delete [] data;
}



// MP4
// Extend file size
bool FileHeader::ExtendFileSize(PersistentBitmap *freeMap, int fileSize){

    int ori_filesize = FileLength();
    int ori_numSectors = divRoundUp(ori_filesize, SectorSize);
    int extra_numSectors = divRoundUp(fileSize, SectorSize);

    int total_numSectors = ori_numSectors + extra_numSectors;
    //there is no enough free sector
    if (freeMap->NumClear() < extra_numSectors || total_numSectors > NumSectors) {
        return FALSE; // not enough space

    } 
    // if (freeMap->NumClear() < numSectors)
	//     return FALSE;		// not enough space
    
    int sectorAllocated = 0;
    if (ori_numSectors < NumDirect) {
        DEBUG(dbgFile, "Allocate: Direct Table is enough.");
        for (int i = 0; i < ori_numSectors; i++) {
            dataSectors[i] = freeMap->FindAndSet();
        // since we checked that there was enough free space,
        // we expect this to succeed
            ASSERT(dataSectors[i] >= 0);
        }
    }  

    DoubleIndirect *db_Indirect;
    IndirectHeader *indir_Header;

    sectorAllocated = ori_numSectors;  
    if(sectorAllocated < extra_numSectors){
        DEBUG(dbgFile, "Allocate: use Indirect Table and Indirect Header.");
        db_Indirect = new DoubleIndirect();
        // is already in using -> FetchFrom
        if(numIndirectSector != -1) {
            db_Indirect->FetchFrom(numIndirectSector);
        } else {
            numIndirectSector = freeMap->FindAndSet();
        }

        for(int i = 0; i < IndirectHeaderPerDoubleIndirect; i++){
            if (sectorAllocated < extra_numSectors) {
                indir_Header = new IndirectHeader();
                // is already in using -> FetchFrom
                if(db_Indirect->numSectorsPointer[i] != -1) {
                    indir_Header->FetchFrom(db_Indirect->numSectorsPointer[i]);
                } else { 
                    db_Indirect->numSectorsPointer[i] = freeMap->FindAndSet();
                }   
                for(int j = 0; j < SectorPerIndirectHeader; j++){
                    if (sectorAllocated < extra_numSectors) {
                        if(indir_Header->numSectorsPointer[j] != -1) { 
                            continue;
                        } else {
                            indir_Header->numSectorsPointer[j] = freeMap->FindAndSet();
                            sectorAllocated++;
                        }    
                    }
                }
            }
            indir_Header->WriteBack(db_Indirect->numSectorsPointer[i]);   
        }
        db_Indirect->WriteBack(numIndirectSector);
    }
    delete db_Indirect;
    delete indir_Header;

    // renew 
    numBytes = numBytes + fileSize;
    numSectors = numSectors + extra_numSectors;

    return TRUE;
}