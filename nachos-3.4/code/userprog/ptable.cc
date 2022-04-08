#include "ptable.h"
#include "system.h"
#include "openfile.h"

#define For(i,a,b) for (int i = (a); i < b; ++i)

PTable::PTable(int size)
{

    if (size < 0)
        return;

    psize = size;
    bm = new BitMap(size);
    bmsem = new Semaphore("bmsem",1);

    For(i,0,MAX_PROCESS){
		pcb[i] = 0;
    }

	bm->Mark(0);

	pcb[0] = new PCB(0);
	pcb[0]->SetFileName("./test/scheduler");
	pcb[0]->parentID = -1;
}

PTable::~PTable()
{
    if( bm != 0 )
	delete bm;
    
    For(i,0,psize){
		if(pcb[i] != 0)
			delete pcb[i];
    }
		
	if( bmsem != 0)
		delete bmsem;
}

int PTable::ExecUpdate(char* name)
{


        //call mutex->P(); prevent fetch 2 task a time.
	bmsem->P();
	
	if(name == NULL)
	{
		printf("\nPTable::Exec : Can't not execute name is NULL.\n");
		bmsem->V();
		return -1;
	}
	// so sanh ten chuong trinh va currentThread de dam bao khong goi chinh no
	if( strcmp(name,"./test/scheduler") == 0 || strcmp(name,currentThread->getName()) == 0 )
	{
		printf("\nPTable::Exec : Can't not execute itself.\n");		
		bmsem->V();
		return -1;
	}

	int index = this->GetFreeSlot();

	if(index < 0)
	{
		printf("\nPTable::Exec :There is no free slot.\n");
		bmsem->V();
		return -1;
	}

	//neu co slot trong thi khoi tao 1 PCB voi processID = index
	pcb[index] = new PCB(index);
	pcb[index]->SetFileName(name);

	// parrentID la processID cua currentThread
    	pcb[index]->parentID = currentThread->processID;

	
	int pid = pcb[index]->Exec(name,index);

	bmsem->V();

	return pid;
}

int PTable::JoinUpdate(int id)
{
	// Ta kiểm tra tính hợp lệ của processID id và kiểm tra tiến trình gọi Join có phải là cha của tiến trình
	// có processID là id hay không. Nếu không thỏa, ta báo lỗi hợp lý và trả về -1.
	if(id < 0)
	{
		printf("\nPTable::JoinUpdate : id = %d", id);
		return -1;
	}
	// Check if process running is parent process of process which joins
	if(currentThread->processID != pcb[id]->parentID)
	{
		printf("\nPTable::JoinUpdate Can't join in process which is not it's parent process.\n");
		return -1;
	}

    	// Tăng numwait và gọi JoinWait() để chờ tiến trình con thực hiện.
	// Sau khi tiến trình con thực hiện xong, tiến trình đã được giải phóng
	pcb[pcb[id]->parentID]->IncNumWait();
	

	//pcb[id]->boolBG = 1;
	
	pcb[id]->JoinWait();

	// Xử lý exitcode.	
	int ec = pcb[id]->GetExitCode();
        // ExitRelease() để cho phép tiến trình con thoát.
	pcb[id]->ExitRelease();

    // Successfully
	return ec;
}
int PTable::ExitUpdate(int exitcode)
{              
    // Nếu tiến trình gọi là main process thì gọi Halt().
	int id = currentThread->processID;
	if(id == 0)
	{
		
		currentThread->FreeSpace();		
		interrupt->Halt();
		return 0;
	}
    
        if(IsExist(id) == false)
	{
		printf("\nPTable::ExitUpdate: This %d is not exist. Try again?", id);
		return -1;
	}

	


	
	// Ngược lại gọi SetExitCode để đặt exitcode cho tiến trình gọi.
	pcb[id]->SetExitCode(exitcode);
	pcb[pcb[id]->parentID]->DecNumWait();
    
    // Gọi JoinRelease để giải phóng tiến trình cha đang đợi nó(nếu có) và ExitWait() để xin tiến trình cha
    // cho phép thoát.
	pcb[id]->JoinRelease();
    // 
	pcb[id]->ExitWait();
	
	Remove(id);
	return exitcode;
}

// Find free slot in order to save the new process infom
int PTable::GetFreeSlot()
{
	return bm->Find();
}

// Check if Process ID is Exist
bool PTable::IsExist(int pid)
{
	return bm->Test(pid);
}

// Remove proccess ID out of table
// When it ends
void PTable::Remove(int pid)
{
	bm->Clear(pid);
	if(pcb[pid] != 0)
		delete pcb[pid];
}

char* PTable::GetFileName(int id)
{
	return (pcb[id]->GetFileName());
}

