#ifndef STABLE_H
#define STABLE_H
#include "synch.h"
#include "bitmap.h"
#define MAX_SEMAPHORE 10


#endif // STABLE_H

// Lop Sem dung de quan ly semaphore.
class Sem
{
private:
	char name[50];		// Ten cua semaphore
	Semaphore* sem;		// Tao semaphore de quan ly
public:
	// Khoi tao doi tuong Sem. Gan gia tri ban dau la null
	// khoi tao sem su dung
	Sem(char* na, int i)
	{
		strcpy(this->name, na);
		sem = new Semaphore(this->name, i);
	}

	~Sem()
	{
		if(sem)
			delete sem;
	}

	void wait()
	{
		sem->P();	// Down(sem)
	}

	void signal()
	{
		sem->V();	// Up(sem)
	}
	
	char* GetName()
	{
		return this->name;
	}
};

class STable
{
private:
	BitMap* bm;	// quản lý slot trống
	Sem* semTab[MAX_SEMAPHORE];	
public:
	STable();
	//max 10 Semaphore, ini val = null
	~STable();	
	int Create(char *name, int init);

	// neu ton tai Semaphore “name”, this->P()
	int Wait(char *name);

	// neu ton tai Semaphore “name”, this->V()
	int Signal(char *name);
	
	// tim slot trong.
	int FindFreeSlot();

	
};
