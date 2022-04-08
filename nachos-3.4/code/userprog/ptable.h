// ptable.h 

#ifndef PTABLE_H
#define PTABLE_H

#include "bitmap.h"
#include "pcb.h"
#include "synch.h"

#define MAX_PROCESS 10

class PTable
{
private:
	int psize;
	BitMap *bm;                 // mark used slot in pcb
	PCB* pcb[MAX_PROCESS];

	Semaphore* bmsem;           // prevent 2 task a time

public:
     PTable(int = 10);           // Khoi tao size doi tuong pcb
    ~PTable();               
		
    int ExecUpdate(char*);      // SC_Exit
    int ExitUpdate(int);        // SC_Exit
    int JoinUpdate(int);        // SC_Join

    int GetFreeSlot();          
    bool IsExist(int pid);      // exist processID?
    
    void Remove(int pid);       // delete processID khi ket thuc
    char* GetFileName(int id);  
};
#endif // PTABLE_H

