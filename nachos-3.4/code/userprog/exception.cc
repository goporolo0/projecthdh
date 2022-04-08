// exception.cc
#include "copyright.h"
#include "system.h"
#include "syscall.h"

#define MaxFileLength 32

// tang 4 byte
void IncreasePC()
{
	int counter = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, counter);
	counter = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, counter);
	machine->WriteRegister(NextPCReg, counter + 4);
}

char *User2System(int virtAddr, int limit)
{
	int i;
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1];
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

// handle exception: noExcep,Halt,Syscall,...
void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which)
	{
	case NoException: // Everything ok!
		return;

	case PageFaultException: // No valid translation found
		DEBUG('a', "\n No valid translation found");
		printf("\n\n No valid translation found");
		interrupt->Halt();
		break;

	case ReadOnlyException: // Write attempted to page marked "read-only"
		DEBUG('a', "\n Write attempted to page marked read-only");
		printf("\n\n Write attempted to page marked read-only");
		interrupt->Halt();
		break;

	case BusErrorException: // Translation resulted in an invalid physical address
		DEBUG('a', "\n Translation resulted invalid physical address");
		printf("\n\n Translation resulted invalid physical address");
		interrupt->Halt();
		break;

	case AddressErrorException: // Unaligned reference or one that was beyond the end of the address space
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space");
		interrupt->Halt();
		break;

	case OverflowException:
		DEBUG('a', "\nInteger overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		interrupt->Halt();
		break;

	case NumExceptionTypes:
		DEBUG('a', "\n Number exception types");
		printf("\n\n Number exception types");
		interrupt->Halt();
		break;

	case SyscallException: // A program executed a system call.
		switch (type)
		{

		// Shutdown
		case SC_Halt:
			DEBUG('a', "\nDEBUG Shutdown, initiated by user program. ");
			printf("\nprintf Shutdown, initiated by user program. ");
			interrupt->Halt();
			return;

		// Read string
		case SC_ReadString:
		{
			int virtAddr, length;
			char *buffer;
			virtAddr = machine->ReadRegister(4);
			length = machine->ReadRegister(5);
			buffer = User2System(virtAddr, length);
			gSynchConsole->Read(buffer, length);
			System2User(virtAddr, length, buffer);
			delete buffer;
			break;
		}

		// Print an int (posstive, negative)
		case SC_PrintInt:
		{
			int number = machine->ReadRegister(4);
			if (number == 0)
			{
				gSynchConsole->Write("0", 1); // In ra man hinh so 0
				IncreasePC();
				return;
			}
			bool isNegative = false;
			int numberOfNum = 0;
			int firstNumIndex = 0;

			if (number < 0)
			{
				isNegative = true;
				number = number * -1;
				firstNumIndex = 1;
			}

			int t_number = number; // bien tam cho number
			while (t_number)
			{
				numberOfNum++;
				t_number /= 10;
			}

			// Tao buffer chuoi de in ra man hinh
			char *buffer;
			int MAX_BUFFER = 255;
			buffer = new char[MAX_BUFFER + 1];
			for (int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)
			{
				buffer[i] = (char)((number % 10) + 48);
				number /= 10;
			}
			if (isNegative)
			{
				buffer[0] = '-';
				buffer[numberOfNum + 1] = 0;
				gSynchConsole->Write(buffer, numberOfNum + 1);
				delete buffer;
				IncreasePC();
				return;
			}
			buffer[numberOfNum] = 0;
			gSynchConsole->Write(buffer, numberOfNum);
			delete buffer;
			IncreasePC();
			return;
		}

		// Print a char
		case SC_PrintChar:
		{
			char c = (char)machine->ReadRegister(4);
			gSynchConsole->Write(&c, 1);
			break;
		}

		// Print string
		case SC_PrintString:
		{
			int virtAddr;
			char *buffer;
			virtAddr = machine->ReadRegister(4);
			buffer = User2System(virtAddr, 255);
			int length = 0;
			while (buffer[length] != 0)
				length++;
			gSynchConsole->Write(buffer, length + 1);
			delete buffer;
			break;
		}

		// Input: filename
		// Output: -1 = Loi, 0 = Thanh cong
		case SC_CreateFile:
		{
			int virtAddr;
			char *filename;
			virtAddr = machine->ReadRegister(4);
			filename = User2System(virtAddr, MaxFileLength + 1);

			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}

			if (!fileSystem->Create(filename, 0))
			{
				printf("\n Error create file '%s'", filename);
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}
			// success creating
			machine->WriteRegister(2, 0);
			delete filename;
			break;
		}

		// OpenFileID Open(char *name, int type)
		// return openFileID (id cua file)
		case SC_Open:
		{

			int virtAddr = machine->ReadRegister(4);
			int type = machine->ReadRegister(5);
			char *filename;
			filename = User2System(virtAddr, MaxFileLength);

			int freeSlot = fileSystem->FindFreeSlot();

			strcpy(fileSystem->openList[freeSlot], filename);
			// printf(fileSystem->openList[2]);

			if (freeSlot != -1)
			{
				if (type == 0 || type == 1)
				{

					if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL)
					{
						machine->WriteRegister(2, freeSlot);
					}
				}
				else if (type == 2) // xu li stdin voi type quy uoc la 2
				{
					machine->WriteRegister(2, 0);
				}
				else // xu li stdout voi type quy uoc la 3
				{
					machine->WriteRegister(2, 1); // tra ve OpenFileID
				}
				delete[] filename;
				break;
			}
			machine->WriteRegister(2, -1); // Khong mo duoc file return -1

			delete[] filename;
			break;
		}

		// Input id cua file(OpenFileID)
		// return 0 || -1
		case SC_Close:
		{

			int fid = machine->ReadRegister(4);
			if (fid >= 0 && fid <= 9)
			{
				if (fileSystem->openf[fid])
				{
					delete fileSystem->openf[fid];
					fileSystem->openf[fid] = NULL; // Gan vung nho NULL
					machine->WriteRegister(2, 0);
					break;
				}
			}
			machine->WriteRegister(2, -1);
			break;
		}

			// Input: Read(char* buffer, int charcount, openFIleID id)
			// Output: -1: Loi, So byte read thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Doc file voi tham so la buffer, so ky tu cho phep va id cua file
		case SC_Read:
		{
			int virtAddr = machine->ReadRegister(4);
			int charcount = machine->ReadRegister(5);
			int id = machine->ReadRegister(6);
			int OldPos;
			int NewPos;
			char *buf;
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 9)
			{
				printf("\nKhong the read vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nKhong the read vi file  khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			if (fileSystem->openf[id]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
			{
				printf("\nKhong the read file stdout.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			OldPos = fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
			buf = User2System(virtAddr, charcount);
			// Xet truong hop doc file stdin (type quy uoc la 2)
			if (fileSystem->openf[id]->type == 2)
			{
				int size = gSynchConsole->Read(buf, charcount);
				System2User(virtAddr, size, buf);
				machine->WriteRegister(2, size); // Tra ve so byte thuc su doc duoc
				delete buf;
				IncreasePC();
				return;
			}
			// Xet truong hop doc file binh thuong thi tra ve so byte thuc su
			if ((fileSystem->openf[id]->Read(buf, charcount)) > 0)
			{
				// So byte thuc su = NewPos - OldPos
				NewPos = fileSystem->openf[id]->GetCurrentPos();
				System2User(virtAddr, NewPos - OldPos, buf);
				machine->WriteRegister(2, NewPos - OldPos);
			}
			else
			{
				// Truong hop doc file NULL tra ve -2
				machine->WriteRegister(2, -2);
			}
			delete buf;
			break;
		}

		// Input: Write(char* buffer, int charcount, OpenFileID id)
		// Output: -1: Loi, So byte write thuc su || -2: Thanh cong
		case SC_Write:
		{
			int virtAddr = machine->ReadRegister(4);
			int charcount = machine->ReadRegister(5);
			int id = machine->ReadRegister(6);
			int OldPos;
			int NewPos;
			char *buf;
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 9)
			{
				printf("\nKhong the write vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nKhong the write vi file nay khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
			if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)
			{
				printf("\nKhong the write file stdin hoac file only read.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			OldPos = fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
			buf = User2System(virtAddr, charcount);
			// Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
			if (fileSystem->openf[id]->type == 0)
			{
				if ((fileSystem->openf[id]->Write(buf, charcount)) > 0)
				{
					// So byte thuc su = NewPos - OldPos
					NewPos = fileSystem->openf[id]->GetCurrentPos();
					machine->WriteRegister(2, NewPos - OldPos);
					delete buf;
					IncreasePC();
					return;
				}
			}
			if (fileSystem->openf[id]->type == 3) // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
			{
				int i = 0;
				while (buf[i] != 0 && buf[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
				{
					gSynchConsole->Write(buf + i, 1); // Su dung ham Write cua lop SynchConsole
					i++;
				}
				buf[i] = '\n';
				gSynchConsole->Write(buf + i, 1); // Write ky tu '\n'
				machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
				delete buf;
				IncreasePC();
				return;
			}
		}

		// Input: Seek(int pos, openFileID id)
		// Output: -1: Loi, Vi tri thuc su: Thanh cong
		case SC_Seek:
		{
			int pos = machine->ReadRegister(4);
			int id = machine->ReadRegister(5);
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 9)
			{
				printf("\nKhong the seek vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nKhong the seek vi file nay khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra co goi Seek tren file console khong
			if (id == 0 || id == 1)
			{
				printf("\nKhong the seek tren file console.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Neu pos = -1 thi gan pos = Length nguoc lai thi giu nguyen pos
			pos = (pos == -1) ? fileSystem->openf[id]->Length() : pos;
			if (pos > fileSystem->openf[id]->Length() || pos < 0)
			{
				printf("\nKhong the seek file den vi tri nay.");
				machine->WriteRegister(2, -1);
			}
			else
			{
				// Neu hop le thi tra ve vi tri di chuyen thuc su trong file
				fileSystem->openf[id]->Seek(pos);
				machine->WriteRegister(2, pos);
			}
			IncreasePC();
			return;
		}

		// int Delete(char*name)
		case SC_Delete:
		{

			int virtAddr = machine->ReadRegister(4);
			char *filename;

			filename = User2System(virtAddr, MaxFileLength);

			for (int i = 2; i < 10; i++)
			{

				if (0 == strcmp(fileSystem->openList[i], filename))
				{
					printf(filename);
					printf(" is openning, cant delete");
					IncreasePC();
					return;
				}
			}
			printf("\ncan be delete");

			if (!fileSystem->Remove(filename))
				machine->WriteRegister(2, -1);
			else
				machine->WriteRegister(2, 0);

			delete[] filename;
			break;
		}
		case SC_Exec:
		{
			// Input: vi tri int
			// Output: Fail return -1, Success: return id cua thread dang chay
			// SpaceId Exec(char *name);
			int virtAddr;
			virtAddr = machine->ReadRegister(4); // doc dia chi ten chuong trinh tu thanh ghi r4
			char *name;
			name = User2System(virtAddr, MaxFileLength + 1); // Lay ten chuong trinh, nap vao kernel

			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				// IncreasePC();
				return;
			}
			OpenFile *oFile = fileSystem->Open(name);
			if (oFile == NULL)
			{
				printf("\nExec:: Can't open this file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}

			delete oFile;

			// Return child process id
			int id = pTab->ExecUpdate(name);
			machine->WriteRegister(2, id);

			delete[] name;
			IncreasePC();
			return;
		}
		case SC_Join:
		{
			// int Join(SpaceId id)
			// Input: id dia chi cua thread
			// Output:
			int id = machine->ReadRegister(4);

			int res = pTab->JoinUpdate(id);

			machine->WriteRegister(2, res);
			IncreasePC();
			return;
		}
		case SC_Exit:
		{
			// void Exit(int status);
			//  Input: status code
			int exitStatus = machine->ReadRegister(4);

			if (exitStatus != 0)
			{
				IncreasePC();
				return;
			}

			int res = pTab->ExitUpdate(exitStatus);
			// machine->WriteRegister(2, res);

			currentThread->FreeSpace();
			currentThread->Finish();
			IncreasePC();
			return;
		}
		case SC_CreateSemaphore:
		{
			// int CreateSemaphore(char* name, int semval).
			int virtAddr = machine->ReadRegister(4);
			int semval = machine->ReadRegister(5);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				delete[] name;
				IncreasePC();
				return;
			}

			int res = semTab->Create(name, semval);

			if (res == -1)
			{
				DEBUG('a', "\n Khong the khoi tao semaphore");
				printf("\n Khong the khoi tao semaphore");
				machine->WriteRegister(2, -1);
				delete[] name;
				IncreasePC();
				return;
			}

			delete[] name;
			machine->WriteRegister(2, res);
			IncreasePC();
			return;
		}

		case SC_Wait:
		{
			// int Wait(char* name)
			int virtAddr = machine->ReadRegister(4);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				delete[] name;
				IncreasePC();
				return;
			}

			int res = semTab->Wait(name);

			if (res == -1)
			{
				DEBUG('a', "\n Khong ton tai ten semaphore nay!");
				printf("\n Khong ton tai ten semaphore nay!");
				machine->WriteRegister(2, -1);
				delete[] name;
				IncreasePC();
				return;
			}

			delete[] name;
			machine->WriteRegister(2, res);
			IncreasePC();
			return;
		}
		case SC_Signal:
		{
			// int Signal(char* name)
			int virtAddr = machine->ReadRegister(4);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				delete[] name;
				IncreasePC();
				return;
			}

			int res = semTab->Signal(name);

			if (res == -1)
			{
				DEBUG('a', "\n Khong ton tai ten semaphore nay!");
				printf("\n Khong ton tai ten semaphore nay!");
				machine->WriteRegister(2, -1);
				delete[] name;
				IncreasePC();
				return;
			}

			delete[] name;
			machine->WriteRegister(2, res);
			IncreasePC();
			return;
		}
//end case

		default:
			break;
		}
		IncreasePC();
	}
}
