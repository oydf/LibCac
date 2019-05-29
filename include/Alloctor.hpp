/*
	����new��delete���ڴ�ߴ�ķ���������ԭ�г��������ָ��
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
