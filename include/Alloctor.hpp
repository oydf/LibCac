/*
	重载new和delete带内存尺寸的方法，兼容原有程序和智能指针
*/

#ifndef _ALLOCTOR_H_
#define _ALLOCTOR_H_
#include"MemoryMge.hpp"

using namespace std;

void* operator new(size_t size)
{
	return MemoryMge::getMemoryMgeInstance().allocMem(size);
}
void operator delete(void* p) 
{
	MemoryMge::getMemoryMgeInstance().freeMem(p);
}
void* operator new[](size_t size)
{
	return MemoryMge::getMemoryMgeInstance().allocMem(size);
}
void operator delete[](void* p)
{
	MemoryMge::getMemoryMgeInstance().freeMem(p);
}
void* mem_alloc(size_t size)
{
	return malloc(size);
}
void mem_free(void* p)
{
	free(p);
}
#endif // !_ALLOCTOR_H_
