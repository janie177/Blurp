#pragma once

#include <set>
#include <memory>
#include <mutex>

#include "TypelessPool.h"

namespace utilities
{
	template<typename T>
	class MP_Ptr;

	template<typename T>
	class MemoryPool;

	/*
	 * MP_Ptr stands for "Memory Pool Pointer".
	 * It acts like a unique pointer, essentially owning part of the memory in a memory pool.
	 * Memory is automatically freed during destruction or on command.
	 * 
	 * This is thread safe, unless the pool is deallocated while another thread is still using the pointers.
	 * In that case honestly just rethink what you're doing.
	 */
	template<typename T>
	class MP_Ptr
	{
		//Friend
	public:
		friend class MemoryPool<T>;

		//Construction
	public:
		MP_Ptr();
		MP_Ptr(T* data, MemoryPool<T>* pool);
		MP_Ptr(MP_Ptr&& other) noexcept;
		MP_Ptr& operator=(MP_Ptr&& other) noexcept;

		/*
		 * Free memory in the pool.
		 */
		~MP_Ptr();

		//Deleted Functionality
	public:
		MP_Ptr(const MP_Ptr&) = delete;
		MP_Ptr& operator =(const MP_Ptr&) = delete;

		//Normal functionality
	public:

		/*
		 * Directly access the inner object.
		 */
		T* operator->();

		/*
		 * Direct access using the * operator.
		 */
		T& operator*();

		/*
		 * Free the memory owned by this pointer.
		 */
		void free();

		/*
		 * Releases ownership of the pointer that was contained by this MP_Ptr.
		 * 
		 * The pointer is returned and has to be manually deleted.
		 * The state of the MP_Ptr is set to invalid.
		 */
		T* release();

		/*
		 * Get the raw pointer contained by this MP_Ptr.
		 */
		T* get();

		/*
		 * See if this pointer has valid data.
		 */
		bool isValid() const;

	private:
		/*
		 * This is called when the memory pool owning the memory is deallocated.
		 */
		void setPoolDead();

	private:
		T* data;
		MemoryPool<T>* pool;
	};


	/*
	 * MemoryPool is a chunk of memory that is allocated to contain a specific type.
	 * It is thread safe, and allows the allocation of objects.
	 * A MP_Ptr object is returned, which owns the new object.
	 * When the MP_Ptr object is destroyed, the memory is automatically recycled.
	 */
	template<typename T>
	class MemoryPool
	{
		//Make sure instances can be made of T.
		static_assert(!std::is_abstract<T>::value);

		//Friend class
	public:
		friend class MP_Ptr<T>;

		//Construction and destruction
	public:
		explicit MemoryPool(std::uint32_t size);

		MemoryPool(const MemoryPool&) = delete;
		MemoryPool(MemoryPool&&) = delete;
		MemoryPool operator =(MemoryPool&&) = delete;
		MemoryPool operator =(const MemoryPool&) = delete;

		~MemoryPool();

	public:
		/*
		 * Allocate memory for a new object using the provided constructor arguments.
		 */
		template<class... Args>
		MP_Ptr<T> allocate(Args&& ... args);

		/*
		 * Free the memory held in this pointer.
		 */
		void free(T* data);

		/*
		 * Returns whether or not this memory pool is currently full.
		 */
		bool isFull();

		/*
		 * Return the current amount of objects residing in this pool.
		 */
		std::uint32_t getObjectCount();

		/*
		 * Return the amount of free slots in this pool.
		 */
		std::uint32_t getFreeSlots();

	private:

		/*
		 * Register a pointer with this memory pool.
		 */
		void registerPtr(MP_Ptr<T>* ptr);

		/*
		 * Unregister a pointer with this memory pool.
		 */
		void removePtr(MP_Ptr<T>* ptr);

	private:
		//Keep an internal typeless pool that actually holds the memory.
		std::unique_ptr<TypelessPool> innerPool;
		std::set<MP_Ptr<T>*> pointers;

		std::mutex poolMutex;
		std::mutex ptrMutex;
	};



	/*
	 * IMPLEMENTATION OF MP_Ptr
	 */


	template <typename T>
	MP_Ptr<T>::MP_Ptr() : data(nullptr), pool(nullptr)
	{

	}

	template <typename T>
	MP_Ptr<T>::MP_Ptr(T* data, MemoryPool<T>* pool) : data(data), pool(pool)
	{
		if (pool != nullptr)
		{
			pool->registerPtr(this);
		}
	}

	template <typename T>
	MP_Ptr<T>::MP_Ptr(MP_Ptr&& other) noexcept : pool(nullptr), data(nullptr)
	{
		//Delete old memory that is being overwritten.
		free();

		//Take values from other object.
		pool = other.pool;
		data = other.data;

		//Set other objects owned data to null.
		other.data = nullptr;

		//If this MP_Ptr now belongs to a pool, register with it.
		//The moved Ptr will automatically de-register with the pool in its destructor.
		if (pool != nullptr)
		{
			pool->registerPtr(this);
		}
	}

	template <typename T>
	MP_Ptr<T>& MP_Ptr<T>::operator=(MP_Ptr&& other) noexcept
	{
		if (&other != this)
		{
			//Delete old memory that is being overwritten.
			free();

			//Set data to other object's data.
			pool = other.pool;
			data = other.data;

			//Remove ownership from other object.
			other.data = nullptr;

			//If this MP_Ptr now belongs to a pool, register with it.
			if (pool != nullptr)
			{
				pool->registerPtr(this);
			}
		}

		return *this;
	}

	template <typename T>
	MP_Ptr<T>::~MP_Ptr()
	{
		free();
	}

	template <typename T>
	T* MP_Ptr<T>::operator->()
	{
#ifdef MAUS_DEBUG
		if (!isValid())
		{
			assert(0 && "Trying to access invalid MP_Ptr.");
			ErrorUtil::crash("Trying to access invalid MP_Ptr.");
		}
#endif

		return data;
	}

	template <typename T>
	T& MP_Ptr<T>::operator*()
	{
		return *data;
	}

	template <typename T>
	void MP_Ptr<T>::free()
	{
		//If this MP_Ptr has a valid pool, de-register with it.
		if (pool != nullptr)
		{
			//If this MP_Ptr still owns data, mark it for destruction.
			if (data != nullptr)
			{
				pool->free(data);
				data = nullptr;
			}

			pool->removePtr(this);
			pool = nullptr;
		}
	}

	template <typename T>
	T* MP_Ptr<T>::release()
	{
		T* ptr = data;
		data = nullptr;

		if(pool != nullptr)
		{
			pool->removePtr(this);
			pool = nullptr;
		}

		return ptr;
	}

	template <typename T>
	T* MP_Ptr<T>::get()
	{
		return data;
	}

	template <typename T>
	bool MP_Ptr<T>::isValid() const
	{
		return data != nullptr;
	}

	template <typename T>
	void MP_Ptr<T>::setPoolDead()
	{
		pool = nullptr;
		data = nullptr;
	}






	/*
	 * IMPLEMENTATION OF MemoryPool
	 */

	template <typename T>
	MemoryPool<T>::MemoryPool(std::uint32_t size) : innerPool(std::make_unique<TypelessPool>(size, static_cast<std::uint32_t>(sizeof(T))))
	{

	}

	template <typename T>
	MemoryPool<T>::~MemoryPool()
	{
		//Mark all pointers for dead.
		for (auto& ptr : pointers)
		{
			ptr->setPoolDead();
		}
	}

	template <typename T>
	template <class ... Args>
	MP_Ptr<T> MemoryPool<T>::allocate(Args&& ... args)
	{
		std::lock_guard<std::mutex> lock(poolMutex);

		T* ptr = innerPool->allocate<T>(std::forward<Args>(args)...);
		return MP_Ptr<T>(ptr, this);
	}

	template <typename T>
	bool MemoryPool<T>::isFull()
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		return innerPool->isFull();
	}

	template <typename T>
	std::uint32_t MemoryPool<T>::getObjectCount()
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		return innerPool->getObjectCount();
	}

	template <typename T>
	std::uint32_t MemoryPool<T>::getFreeSlots()
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		return innerPool->getFreeSlots();
	}

	template <typename T>
	void MemoryPool<T>::free(T* data)
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		innerPool->free<T>(data);
	}

	template <typename T>
	void MemoryPool<T>::registerPtr(MP_Ptr<T>* ptr)
	{
		std::lock_guard<std::mutex> lock(ptrMutex);
		pointers.emplace(ptr);
	}

	template <typename T>
	void MemoryPool<T>::removePtr(MP_Ptr<T>* ptr)
	{
		std::lock_guard<std::mutex> lock(ptrMutex);
		auto found = std::find(pointers.begin(), pointers.end(), ptr);
		if (found != pointers.end())
		{
			pointers.erase(found);
		}
	}

}
