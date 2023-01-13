// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------
void
ExceptionHandler(ExceptionType which)
{
    char ch;
    int val;
    int type = kernel->machine->ReadRegister(2);
    int status, exit, threadID, programID, fileID, numChar;
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    DEBUG(dbgTraCode, "In ExceptionHandler(), Received Exception " << which << " type: " << type << ", " << kernel->stats->totalTicks);
    switch (which) {
    case SyscallException:
	switch(type) {
	    case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
			cout<<"in exception\n";
			ASSERTNOTREACHED();
		    break;
	    case SC_PrintInt:
			DEBUG(dbgSys, "Print Int\n");
			val=kernel->machine->ReadRegister(4);
			DEBUG(dbgTraCode, "In ExceptionHandler(), into SysPrintInt, " << kernel->stats->totalTicks);    
			SysPrintInt(val); 	
			DEBUG(dbgTraCode, "In ExceptionHandler(), return from SysPrintInt, " << kernel->stats->totalTicks);
			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
		    break;

		#ifndef FILESYS_STUB
	    case SC_Open:
			DEBUG(dbgSys, "Open a file, initiated by user program.\n");
			val = kernel->machine->ReadRegister(4);
			{
      // accquire the filename from main memory
			char *filename = &(kernel->machine->mainMemory[val]);
			// cout << filename << endl;
			status = SysOpen  (filename);
			kernel->machine->WriteRegister(2, (int) status);
			}
			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
		    break;

  	    case SC_Write:
			DEBUG(dbgSys, "Write a file, initiated by user program.\n");
	    	numChar = kernel->machine->ReadRegister(4);
	    	{
      // accquire the char let this char write into the file 
			char *buffer = &(kernel->machine->mainMemory[numChar]);
			//cout << buffer << endl;
      // return the number of characters actually written to the file
			status = SysWrite(buffer, (int)kernel->machine->ReadRegister(5), (int)kernel->machine->ReadRegister(6));
			kernel->machine->WriteRegister(2, (int) status);
			}
			// check if successfully Write a file
			if (status != -1) {
				DEBUG(dbgSys, "Successfully Write a file, initiated by user program.\n");
			} else {
				DEBUG(dbgSys, "Fail to write a file, initiated by user program.\n");
			}

			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

			return;
	        ASSERTNOTREACHED();
	  	    break;

  	    case SC_Read:
		
			DEBUG(dbgSys, "Read a file, initiated by user program.\n");
	    	numChar = kernel->machine->ReadRegister(4);
	    	{
            // accquire the buffer to "read" form the file to the buffer 
			char *buffer = &(kernel->machine->mainMemory[numChar]);
			status = SysRead(buffer, (int)kernel->machine->ReadRegister(5), (int)kernel->machine->ReadRegister(6));
			kernel->machine->WriteRegister(2, (int) status);
			}
			// check if successfully Read a file
			if (status != -1) {
				DEBUG(dbgSys, "Successfully read a file, initiated by user program.\n");
			} else {
				DEBUG(dbgSys, "Fail to read a file, initiated by user program.\n");
			}

			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

			return;
	        ASSERTNOTREACHED();
	  	    break;

	  	case SC_Close:
			DEBUG(dbgSys, "Close a file, initiated by user program.\n");
	    	val = kernel->machine->ReadRegister(4);
	    	
	    	{
			status = SysClose(val);
			kernel->machine->WriteRegister(2, (int) status);
			}

			if (status == 1) {
				DEBUG(dbgSys, "Successfully close a file, initiated by user program.\n");
			}
			

			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

			return;
	        ASSERTNOTREACHED();
	  	    break;

	    case SC_MSG:
			DEBUG(dbgSys, "Message received.\n");
			val = kernel->machine->ReadRegister(4);
			{
			char *msg = &(kernel->machine->mainMemory[val]);
			cout << msg << endl;
			}
			SysHalt();
			ASSERTNOTREACHED();
		    break;
	    case SC_Create: // FILESYS_STUB
			val = kernel->machine->ReadRegister(4);
			{
			char *filename = &(kernel->machine->mainMemory[val]);
			int Size = kernel->machine->ReadRegister(5);
			//cout << filename << endl;
			status = SysCreate(filename, Size);
			kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
			break;
      	case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));
			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);	
			/* Modify return point */
			{
			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
				
			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
		
			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			}
			cout << "result is " << result << "\n";	
			return;	
			ASSERTNOTREACHED();
			break;
	    case SC_Exit:
			DEBUG(dbgAddr, "Program exit\n");
            		val=kernel->machine->ReadRegister(4);
            		cout << "return value:" << val << endl;
			kernel->currentThread->Finish();
            break;
      	default:
			cerr << "Unexpected system call " << type << "\n";
	    break;

		#else // FILESYS
		case SC_Create: // FILESYS
			val = kernel->machine->ReadRegister(4);
			{
			char *filename = &(kernel->machine->mainMemory[val]);
			//cout << filename << endl;
			status = SysCreate(filename);
			kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
			break;
		case SC_Open:
			DEBUG(dbgSys, "Open a file, initiated by user program.\n");
			val = kernel->machine->ReadRegister(4);
			{
      // accquire the filename from main memory
			char *filename = &(kernel->machine->mainMemory[val]);
			// cout << filename << endl;
			status = SysOpen  (filename);
			kernel->machine->WriteRegister(2, (int) status);
			}
			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
		    break;

  	    case SC_Write:
			DEBUG(dbgSys, "Write a file, initiated by user program.\n");
	    	numChar = kernel->machine->ReadRegister(4);
	    	{
      // accquire the char let this char write into the file 
			char *buffer = &(kernel->machine->mainMemory[numChar]);
			//cout << buffer << endl;
      // return the number of characters actually written to the file
			status = SysWrite(buffer, (int)kernel->machine->ReadRegister(5), (int)kernel->machine->ReadRegister(6));
			kernel->machine->WriteRegister(2, (int) status);
			}
			// check if successfully Write a file
			if (status != -1) {
				DEBUG(dbgSys, "Successfully Write a file, initiated by user program.\n");
			} else {
				DEBUG(dbgSys, "Fail to write a file, initiated by user program.\n");
			}

			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

			return;
	        ASSERTNOTREACHED();
	  	    break;

  	    case SC_Read:
		
			DEBUG(dbgSys, "Read a file, initiated by user program.\n");
	    	numChar = kernel->machine->ReadRegister(4);
	    	{
            // accquire the buffer to "read" form the file to the buffer 
			char *buffer = &(kernel->machine->mainMemory[numChar]);
			status = SysRead(buffer, (int)kernel->machine->ReadRegister(5), (int)kernel->machine->ReadRegister(6));
			kernel->machine->WriteRegister(2, (int) status);
			}
			// check if successfully Read a file
			if (status != -1) {
				DEBUG(dbgSys, "Successfully read a file, initiated by user program.\n");
			} else {
				DEBUG(dbgSys, "Fail to read a file, initiated by user program.\n");
			}

			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

			return;
	        ASSERTNOTREACHED();
	  	    break;

	  	case SC_Close:
			DEBUG(dbgSys, "Close a file, initiated by user program.\n");
	    	val = kernel->machine->ReadRegister(4);
	    	
	    	{
			status = SysClose(val);
			kernel->machine->WriteRegister(2, (int) status);
			}

			if (status == 1) {
				DEBUG(dbgSys, "Successfully close a file, initiated by user program.\n");
			}

			// Set Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

			return;
	        ASSERTNOTREACHED();
	  	    break;
		#endif
	}
	break;
	default:
		cerr << "Unexpected user mode exception " << (int)which << "\n";
		break;
    }
    ASSERTNOTREACHED();
}

