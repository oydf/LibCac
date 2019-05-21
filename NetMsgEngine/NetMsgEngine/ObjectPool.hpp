/*
	单例模式，采用模板进行初始化,对象池管理同类型使用同一个管理类
*/

#ifndef _OBJECTPOOL_H
#define _OBJECTPOOL_H

#include<iostream>
#include<mutex>
template<class ObjType, size_t ObjPoolSzie>
class ObjectPool
{
private:
	struct ObjHeader
	{
	public:
		//编号
		int nId;
		//引用计数
		int nRef;
		//是否在池中
		bool inPool;
		//下一内存块
		ObjHeader* pNext;
	private:
		//预留内存
		char a;
		char b;
	};
	//对象描述信息指针
	ObjHeader* _pHeader;
	//对象池地址
	char* _pBuf;
	//加锁
	std::mutex _mutex;

	//初始化对象池
	void initObjPool()
	{
		if (nullptr != _pBuf)
			return;
		//计算对象池的大小
		size_t realSzie = sizeof(ObjType) + sizeof(ObjHeader);
		size_t n = ObjPoolSzie * realSzie;
		//申请池的内存
		_pBuf = new char[n];
		//初始化头部对象
		_pHeader = (ObjHeader*)_pBuf;
		_pHeader->inPool = true;
		_pHeader->nId = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//向后切片初始化对象
		ObjHeader* p1 = _pHeader;
		for (int i = 0; i < ObjPoolSzie; i++)
		{
			ObjHeader* p2 = (ObjHeader*)(_pBuf + (i * realSzie));
			p2->inPool = true;
			p2->nId = i;
			p2->nRef = 0;
			p2->pNext = nullptr;
			p1->pNext = p2;
			p1 = p2;
		}
		cout << "初始化对象池" << endl;
	}
public:
	ObjectPool()
	{
		_pBuf = nullptr;
		initObjPool();
	}

	~ObjectPool()
	{
		if (_pBuf)
			delete[] _pBuf;
	}
	//申请对象内存
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> obl(_mutex);
		ObjHeader* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (ObjHeader*)new char[sizeof(ObjType) + sizeof(ObjHeader)];
			pReturn->inPool = false;
			pReturn->nId = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
			cout << "从系统中申请对象" << endl;
		}
		else 
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			pReturn->nRef = 1;
			cout << "从池中申请对象" << endl;
		}
		return ((char*)pReturn + sizeof(ObjHeader));
	}

	//释放对象内存
	void freeObjMemory(void* pMem)
	{
		ObjHeader* pBlock = (ObjHeader*)((char*)pMem - sizeof(ObjHeader));
		if (pBlock->inPool)
		{
			std::lock_guard<std::mutex> obl(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else 
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}
};
//管理对象池的基类
template<class ObjType, size_t ObjPoolSzie>
class ObjectPoolMgeBase
{
private:
	typedef ObjectPool<ObjType, ObjPoolSzie> ClassTypePool;
	static ClassTypePool& getObjectPoolMge()
	{
		static ClassTypePool sPool;
		return sPool;
	}
public:
	//申请内存
	void* operator new(size_t nSize)
	{
		return getObjectPoolMge().allocObjMemory(nSize);
	}
	//释放内存
	void operator delete(void* p)
	{
		getObjectPoolMge().freeObjMemory(p);
	}
	//多参初始化方法
	template<typename ...Args>
	static ObjType* createObject(Args ... args)
	{	//不定参数  可变参数
		//可构造的参数调用相应的构造函数，其余参数另处理
		ObjType* obj = new ObjType(args...);
		//...
		return obj;
	}

	static void destroyObject(ObjType* obj)
	{
		delete obj;
	}
};
#endif //_OBJECTPOOL_H