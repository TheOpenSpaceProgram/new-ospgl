#pragma once
#include <mutex>


// Automatically unlocks a mutex on destructor
template<typename T>
class AtomicWrapper
{
private:

	T* data;
	std::mutex* mtx;
public:

	T& operator*()
	{
		return *data;
	}

	T* operator->()
	{
		return data;
	}

	bool is_null()
	{
		return data == nullptr;
	}


	AtomicWrapper(T* ndata, std::mutex* nmtx)
	{
		mtx = nmtx;
		data = ndata;
	}

	AtomicWrapper()
	{
		data = NULL;
		mtx = NULL;
	}

	~AtomicWrapper()
	{
		if (mtx != NULL)
		{
			mtx->unlock();
		}
	}

};



template<typename T>
class Atomic
{
private:

	T data;
	std::mutex mtx;

public:

	// Locks until mutex can be obtained, once it is obtained
	// returns a wrapper which will automatically free the mutex
	AtomicWrapper<T> get()
	{
		mtx.lock();
		return AtomicWrapper<T>(&data, &mtx);
	}

	// Returns null wrapper if mutex is locked
	AtomicWrapper<T> try_get()
	{
		if (mtx.try_lock())
		{
			return AtomicWrapper<T>(&data, &mtx);
		}
		else
		{
			return AtomicWrapper<T>();
		}
	}

	// Skips threading, useful for GUIs and 
	// other non-thread-disruptive accesses
	T* get_unsafe()
	{
		return &data;
	}

};

