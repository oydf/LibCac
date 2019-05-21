/*
	线程安全需要加锁，全局一份需要设计为线程安全的单例模式
*/

#ifndef _MEMORYMGE_H_
#define _MEMORYMGE_H_
#include<assert.h>
#include<mutex>
#include<iostream>

#define MAX_MEMORY_SZIE 1024 //最大内存块为1024k
using namespace std;

class MemoryAlloc;

//内存块描述信息，链式存储
struct MemoryBlock
{
public:
	//所属内存池
	MemoryAlloc* pAlloc;
	//指向下一块可用块
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用计数
	int nRef;
	//是否在内存池中
	bool bPool;
private:
	//字节对齐原则，预留超出占据内部碎片
	char a;
	char b;
	char c;
};

//内存池基类设计
class MemoryAlloc
{
protected:
	//内存池地址
	char* _pAdr;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元块的大小
	size_t _nSzie;
	//内存单元的数量
	size_t _nBlockSzie;
	//线程锁
	std::mutex _mutex;
public:
	MemoryAlloc()
	{
		_pAdr = nullptr;
		_pHeader = nullptr;
		_nSzie = 0;
		_nBlockSzie = 0;
	}
	~MemoryAlloc()
	{
		if(nullptr != _pAdr)
			free(_pAdr);
	}

	//初始化内存池为链式结构
	void initMemory()
	{
		if (nullptr != _pAdr)
			return;
		//内存池大小为 （描述信息+存储单元）* 个数
		size_t realSzie = _nSzie + sizeof(MemoryBlock);
		size_t poolSize = realSzie*_nBlockSzie;
		//向系统申请内存空间,转换为char*类型，单字节步长，便于操作
		_pAdr = (char*)malloc(poolSize);
		//对一大片内存空间切片管理,初始化头部块信息
		_pHeader = (MemoryBlock*)_pAdr;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//向下切片，初始化内存块
		MemoryBlock* p1 = _pHeader;
		for (size_t i = 1; i < _nBlockSzie; i++)
		{
			MemoryBlock* p2 = (MemoryBlock*)(_pAdr + (i * realSzie));
			p2->bPool = true;
			p2->nID = i;
			p2->nRef = 0;
			p2->pAlloc = this;
			p2->pNext = nullptr;
			p1->pNext = p2;
			p1 = p2;
		}
	}

	//申请内存
	void* allocMemory(size_t nSize)
	{
		//加锁
		std::lock_guard<std::mutex> mml(_mutex);
		if (nullptr == _pAdr)
		{
			initMemory();
		}

		MemoryBlock* pReturn = nullptr;

		//内存池中没有可用的内存块时直接向系统申请
		if (nullptr == _pHeader)
		{
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			cout << "从系统中申请：" << nSize + sizeof(MemoryBlock) << "KB" << endl;
		}
		else 
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			//此块是否被使用过
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
			cout << "从内存池：" << pReturn->pAlloc <<" 申请：" << pReturn->pAlloc->_nSzie <<  "KB" << endl;
		}
		//返回地址为内存块跳过描述信息的部分
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//释放内存
	void freeMemory(void* pMem)
	{
		//用户使用内存未包含描述信息，前移
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		//该块内存是否在内存池中分别处理
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> mml(_mutex);
			//引用计数，没有用户使用才释放
			if (--pBlock->nRef != 0)
			{
				return;
			}
			//头插法，回收到内存池内，作为新头
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else 
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
};

//内存分配器，采用模板来初始化
template<size_t nSzie, size_t nBlockSzie>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//64位和32位系统字节对齐方式差异，用void*来识别系统对齐规则
		const size_t n = sizeof(void*);
		//将传进来的内存块大小进行字节对齐 
		_nSzie = (nSzie / n)*n + (nSzie % n ? n : 0);
		_nBlockSzie = nBlockSzie;
	}
};

//内存管理，new和memorypool的中间层,对不同尺寸的内存池进行hash管理
class MemoryMge
{
public:
	//单例模式 静态
	static MemoryMge& getMemoryMgeInstance()
	{	
		static MemoryMge mge;
		return mge;
	}

	//申请内存
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			cout << "从系统中申请：" << nSize + sizeof(MemoryBlock) << "KB" << endl;
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}

	//释放内存
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}

	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}

private:
	MemoryAlloctor<64, 100000> _mem64;
	MemoryAlloctor<128, 100000> _mem128;
	MemoryAlloctor<256, 100000> _mem256;
	MemoryAlloctor<512, 100000> _mem512;
	MemoryAlloctor<1024, 100000> _mem1024;
	//0~64 65~128 129~256 257~512 513~1024
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];

	//初始化内存池映射数组
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}

	MemoryMge()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}

	~MemoryMge()
	{

	}
};


#endif //_MEMORYMGE_H_