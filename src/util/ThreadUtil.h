#pragma once
#include <mutex>
#include <string>

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

	// Move constructor
	AtomicWrapper(AtomicWrapper&& b)
	{
		this->mtx = b.mtx;
		this->data = b.data;
		b.mtx = nullptr;
		b.data = nullptr;
	}

	AtomicWrapper& operator=(const AtomicWrapper& b)
	{
		this->mtx = b.mtx;
		this->data = b.data;
		b.mtx = nullptr;
		b.data = nullptr;
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
		return std::move(AtomicWrapper<T>(&data, &mtx));
	}

	// Returns null wrapper if mutex is locked
	AtomicWrapper<T> try_get()
	{
		if (mtx.try_lock())
		{
			return std::move(AtomicWrapper<T>(&data, &mtx));
		}
		else
		{
			return std::move(AtomicWrapper<T>());
		}
	}

	// Skips threading, useful for GUIs and 
	// other non-thread-disruptive accesses
	T* get_unsafe()
	{
		return &data;
	}

};

// Use to give meaningful names to thread to aid debugging
// Call on the thread you want to name
// May not work on some platforms!
// Maximum of 15 characters for name
void set_this_thread_name(const std::string& str);

