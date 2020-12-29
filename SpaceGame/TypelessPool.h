#pragma once

#include <cassert>
#include <vector>
#include <stdexcept>

namespace utilities
{
	/*
		Non-templated Memory Pool.
		This allocates memory and then creates objects of a given type.
	*/
	class TypelessPool
	{
		//Deleted functionality
	public:
		TypelessPool(const TypelessPool&) = delete;
		TypelessPool(TypelessPool&&) = delete;
		TypelessPool& operator =(const TypelessPool&) = delete;
		TypelessPool& operator =(TypelessPool&&) = delete;

	public:

		TypelessPool(std::uint32_t size, std::uint32_t typeSize);

	public:

		/*
		 * Returns whether or not this memory pool is currently full.
		 */
		bool isFull() const;

		/*
		 * Return the current amount of objects residing in this pool.
		 */
		std::uint32_t getObjectCount() const;

		/*
		 * Return the amount of free slots in this pool.
		 */
		std::uint32_t getFreeSlots() const;

		/*
			Allocate memory of the given type in the pool.
		*/
		template<class T, class... Args>
		T* allocate(Args&& ... args);

		/*
		 * Free memory in the pool.
		 * Make sure that the type provided is actually the type stored.
		 *
		 * This calls the destructor of the given type.
		 */
		template<class T>
		void free(T* data);

		/*
			Allocate memory in the pool without knowing the type.
			This obviously does not call any constructors.
		*/
		void* allocate();

		/*
			Free memory in the pool.
			This does NOT call any destructors!
		*/
		void free(void* data);

		/*
			Get the size of the type specified.
		*/
		std::uint16_t getTypeSize() const;

		/*
		 * Get the total size of the pool in terms of objects.
		 */
		std::uint16_t getSize() const;

		/*
			Free the allocated memory again.
		*/
		~TypelessPool();

	private:
		//Pool stored as a pointer to uchar.
		std::uint8_t* pool;

		//Vector containing the index of the next element.
		std::vector<std::int32_t> next;

		//Vector containing bools to see if an index is free or not.
		std::vector<bool> isFree;

		//Current index.
		std::int32_t current;

		//Last index.
		std::int32_t last;

		//Size of the pool in terms of objects, and size of the data it holds (per object) in bytes.
		std::uint32_t size;
		std::uint32_t typeSize;

		//Amount of objects being held.
		std::uint32_t objectCount;
	};

	template<class T, class ...Args>
	inline T* TypelessPool::allocate(Args&& ...args)
	{
		//Make sure there is free space in the pool
		if (current < 0)
		{
			throw std::runtime_error("Pool out of memory!");
		}

		//Make sure the size of the type specified matches this memory pool's specified size.
		assert(sizeof(T) == typeSize && "Trying to allocate memory in pool for a type that is not of the specified size!");

		//Make sure the given slot is actually free.
		assert(isFree[current] && "Trying to allocate memory that was not free!");

		//Mark the memory as taken
		isFree[current] = false;

		//Get the adress of the to-overwrite thing.
		std::uint8_t* memory = pool + (static_cast<std::uint64_t>(typeSize) * static_cast<std::uint64_t>(current));

		//Set the index to override next time allocating.
		current = next[current];

		//Increase object count.
		++objectCount;

		//Override the data at the memory address.
		return new (memory) T(std::forward<Args>(args)...);
	}

	template<class T>
	inline void TypelessPool::free(T* data)
	{
		//Make sure this data is part of the pool.
		assert(reinterpret_cast<std::uint64_t>(data) >= reinterpret_cast<std::uint64_t>(pool));
		assert(reinterpret_cast<std::uint64_t>(data) <= reinterpret_cast<std::uint64_t>(pool) + (static_cast<std::uint64_t>(size) * static_cast<std::uint64_t>(typeSize)));

		//Index in the array
		const auto index = static_cast<std::uint64_t>((reinterpret_cast<std::uint8_t*>(data) - pool) / static_cast<std::uint64_t>(typeSize));

		//Make sure this data is not free already
		assert(isFree[index] == false && "Trying to free memory that was not allocated!");

		//Call the destructor.
		data->~T();

		//Make the last free element point to the now free one.

		next[last] = index;

		//The now free element is the last in the list, nullptr makes sure it asserts if it tries to allocate more.
		next[index] = -1;

		//Set the thing to be free
		isFree[index] = true;

		//Set lastFree to now point to the new last element
		last = index;

		//If the pool was full, and the current next pointer is nullptr, make it point to the correct address now.
		if (current < 0)
		{
			current = last;
		}

		//Decrease object count.
		--objectCount;
	}
}