/*
	����ģʽ������ģ����г�ʼ��,����ع���ͬ����ʹ��ͬһ��������
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
		//���
		int nId;
		//���ü���
		int nRef;
		//�Ƿ��ڳ���
		bool inPool;
		//��һ�ڴ��
		ObjHeader* pNext;
	private:
		//Ԥ���ڴ�
		char a;
		char b;
	};
	//����������Ϣָ��
	ObjHeader* _pHeader;
	//����ص�ַ
	char* _pBuf;
	//����
	std::mutex _mutex;

	//��ʼ�������
	void initObjPool()
	{
		if (nullptr != _pBuf)
			return;
		//�������صĴ�С
		size_t realSzie = sizeof(ObjType) + sizeof(ObjHeader);
		size_t n = ObjPoolSzie * realSzie;
		//����ص��ڴ�
		_pBuf = new char[n];
		//��ʼ��ͷ������
		_pHeader = (ObjHeader*)_pBuf;
		_pHeader->inPool = true;
		_pHeader->nId = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//�����Ƭ��ʼ������
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
		cout << "��ʼ�������" << endl;
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
	//��������ڴ�
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
			cout << "��ϵͳ���������" << endl;
		}
		else 
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			pReturn->nRef = 1;
			cout << "�ӳ����������" << endl;
		}
		return ((char*)pReturn + sizeof(ObjHeader));
	}

	//�ͷŶ����ڴ�
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
//�������صĻ���
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
	//�����ڴ�
	void* operator new(size_t nSize)
	{
		return getObjectPoolMge().allocObjMemory(nSize);
	}
	//�ͷ��ڴ�
	void operator delete(void* p)
	{
		getObjectPoolMge().freeObjMemory(p);
	}
	//��γ�ʼ������
	template<typename ...Args>
	static ObjType* createObject(Args ... args)
	{	//��������  �ɱ����
		//�ɹ���Ĳ���������Ӧ�Ĺ��캯���������������
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