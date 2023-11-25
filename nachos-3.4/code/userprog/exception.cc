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
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

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
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

//Change program counter of system after 4 byte to continue load next calls.
void IncreasePC(){
    int counter = machine->ReadRegister(PCReg)
    machine->WriteRegister(PrevPCReg, counter)
    counter = machine->ReadRegister(NextPCReg)
    machine->WriteRegister(PCReg,counter)
    machine->WriteRegister(NextPCReg,counter + 4)
}

// Input: - User space address (int) 
// - Limit of buffer (int) 
// Output:- Buffer (char*) 
// Purpose: Copy buffer from User memory space to System memory space 
char* User2System(int virtAddr,int limit) 
{ 
    int i;// index 
    int oneChar; 
    char* kernelBuf = NULL; 
    kernelBuf = new char[limit +1];//need for terminal string 
    if (kernelBuf == NULL) 
        return kernelBuf; 
    memset(kernelBuf,0,limit+1); 
    //printf("\n Filename u2s:"); 
    for (i = 0 ; i < limit ;i++) 
    { 
        machine->ReadMem(virtAddr+i,1,&oneChar); 
        kernelBuf[i] = (char)oneChar; 
        //printf("%c",kernelBuf[i]); 
        if (oneChar == 0) 
            break; 
    } 
    return kernelBuf; 
}

// Input: - User space address (int) 
// - Limit of buffer (int) 
// - Buffer (char[]) 
// Output:- Number of bytes copied (int) 
// Purpose: Copy buffer from System memory space to User memory space 
int System2User(int virtAddr,int len,char* buffer) 
{ 
    if (len < 0) return -1; 
    if (len == 0)return len; 
    int i = 0; 
    int oneChar = 0 ; 
    do{ 
        oneChar= (int) buffer[i]; 
        machine->WriteMem(virtAddr+i,1,oneChar); 
        i ++; 
    }while(i < len && oneChar != 0); 
        return i; 
} 

void ExceptionHandlerReadChar() {
    char c = 0;
    gSynchConsole->Read(&c,1);
    machine->WriteRegister(2, (int)c);
    int pcAfter = machine->ReadRegister(NextPCReg) + 4;
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, pcAfter);
}
void ExceptionHandlerPrintChar() {
    char c = (char)machine->ReadRegister(4);
    gSynchConsole->Write(&c,1);
    machine->WriteRegister(2, 0);
    int pcAfter = machine->ReadRegister(NextPCReg) + 4;
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, pcAfter);
}

void ExceptionHandlerReadString(){
    int buffeAddress = machine->ReadRegister(4);
    int maxLength = machine->ReadRegister(5);
    
    char *buffer = new char[maxLength + 1];
    int len = gSynchConsole->Read(buffer, maxLength);

    buffer[len] = '\0';

    System2User(buffeAddress, len +1 , buffer);
    delete buffer;  
}

void ExceptionHandlerPrintString(){
    int buffeAddress = machine->ReadRegister(4);
    char* buffer = User2System(bufferAddress, MAX_STRING_SIZE);

    int len = strlen(buffer);

    for(int i =0;i<len;i++){
        gSynchConsole->Write(&buffer[i], 1);
    }

    char nullChar = '\0';
    gSynchConsole->Write(&nullChar, 1);

    delete buffer;
}



void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch(which){
        case NoException: 
            return; 
        
        case PageFaultException:
            DEBUG(‘a’, "\n Page Fault Exception."); 
            printf ("\n\n Page Fault Exception."); 
            interrupt->Halt(); 
            break; 
        case ReadOnlyException:
            DEBUG(‘a’, "\n Read Only Exception."); 
            printf ("\n\n Read Only Exception."); 
            interrupt->Halt(); 
            break; 
        case BusErrorException:
            DEBUG(‘a’, "\n Bus Error Exception."); 
            printf ("\n\n Bus Error Exception."); 
            interrupt->Halt(); 
            break;
        case AddressErrorException:
            DEBUG(‘a’, "\n Address Error Exception."); 
            printf ("\n\n Address Error Exception."); 
            interrupt->Halt(); 
            break;
        case OverflowException:
            DEBUG(‘a’, "\n Overflow Exception."); 
            printf ("\n\n Overflow Exception."); 
            interrupt->Halt(); 
            break;
        case IllegalInstrException:
            DEBUG(‘a’, "\n Illegal Instruction Exception."); 
            printf ("\n\n Illegal Instruction Exception."); 
            interrupt->Halt(); 
            break;
        case NumExceptionTypes:
            DEBUG(‘a’, "\n Num Exception Types."); 
            printf ("\n\n Num Exception Types."); 
            interrupt->Halt(); 
            break;
        case SyscallException: 
            switch (type){ 
                case SC_Halt: 
                    DEBUG(‘a’, "\n Shutdown, initiated by user program."); 
                    printf ("\n\n Shutdown, initiated by user program."); 
                    interrupt->Halt(); 
                    break;
                case SC_Create:
                    {
                        int virtAddr; 
                        char* filename; 
                        DEBUG(‘a’,"\n SC_Create call ..."); 
                        DEBUG(‘a’,"\n Reading virtual address of filename"); 
                        // Lấy tham số tên tập tin từ thanh ghi r4 
                        virtAddr = machine->ReadRegister(4); 
                        DEBUG (‘a’,"\n Reading filename."); 
                        // MaxFileLength là = 32 
                        filename = User2System(virtAddr,MaxFileLength+1); 
                        if (filename == NULL) 
                        { 
                            printf("\n Not enough memory in system"); 
                            DEBUG(‘a’,"\n Not enough memory in system"); 
                            machine->WriteRegister(2,-1); // trả về lỗi cho chương 
                            // trình người dùng 
                            delete filename; 
                            return;
                        }
                        } 
                        DEBUG(‘a’,"\n Finish reading filename."); 
                        //DEBUG(‘a’,"\n File name : '"<<filename<<"'"); 
                        // Create file with size = 0 
                        // Dùng đối tượng fileSystem của lớp OpenFile để tạo file, 
                        // việc tạo file này là sử dụng các thủ tục tạo file của hệ điều 
                        // hành Linux, chúng ta không quản ly trực tiếp các block trên 
                        // đĩa cứng cấp phát cho file, việc quản ly các block của file 
                        // trên ổ đĩa là một đồ án khác 
                        if (!fileSystem->Create(filename,0))
                        { 
                            printf("\n Error create file '%s'",filename); 
                            machine->WriteRegister(2,-1); 
                            delete filename; 
                            return; 
                        } 
                        machine->WriteRegister(2,0); // trả về cho chương trình 
                        // người dùng thành công 
                        delete filename; 
                        break;
                
            }
            default: 
                printf("\n Unexpected user mode exception (%d %d)", which, 
                type); 
                interrupt->Halt();
    }

    // if ((which == SyscallException) && (type == SC_Halt)) {
	// DEBUG('a', "Shutdown, initiated by user program.\n");
   	// interrupt->Halt();
    // } else {
	// printf("Unexpected user mode exception %d %d\n", which, type);
	// ASSERT(FALSE);
    // }
}
