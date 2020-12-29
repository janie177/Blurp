#include "TypelessPool.h"
#include <malloc.h>
#include <stdexcept>

namespace utilities
{
	TypelessPool::TypelessPool(std::uint32_t size, std::uint32_t typeSize) : size(size), typeSize(typeSize), objectCount(0)
	{
		//Pool needs a size
		if(size == 0)
		{
			throw std::runtime_error("Cannot allocate memory pool with no memory.");
		}

		if (typeSize == 0)
		{
			throw std::runtime_error("Cannot allocate memory pool with type of no size.");
		}

		//reserve the memory so it is continuous.
		pool = static_cast<std::uint8_t*>(std::malloc(static_cast<std::uint64_t>(typeSize) * static_cast<std::uint64_t>(size)));

		//Reserve the data that applies to each element.
		next.reserve(size);
		isFree.reserve(size);

		//Set the pointer to the next free element.
		for (std::uint32_t i = 0; i < size; ++i)
		{
			next.push_back(i + 1);
			isFree.push_back(true);
		}

		//The last element doesn't have a next.
		last = static_cast<int>(size) - 1;
		next[last] = -1;

		//The next free element.
		current = 0;
	}

	bool TypelessPool::isFull() const
	{
		return objectCount < size;
	}

	std::uint32_t TypelessPool::getObjectCount() const
	{
		return objectCount;
	}

	std::uint32_t TypelessPool::getFreeSlots() const
	{
		return size - objectCount;
	}

	void* TypelessPool::allocate()
	{
		//Make sure there is free space in the pool
		if (current < 0)
		{
			throw std::runtime_error("Memory pool out of memory!");

		}

		//Make sure the given slot is actually free.
		assert(isFree[current] && "Trying to allocate memory that was not free!");

		//Get the adress of the to-overwrite thing.
		std::uint8_t* memory = pool + (static_cast<std::uint64_t>(current) * typeSize);

		//Mark the memory as taken
		isFree[current] = false;

		//Set the index to override next time allocating.
		current = next[current];

		//Incease object count
		++objectCount;

		//Override the data at the memory address.
		return static_cast<void*>(memory);
	}

	void TypelessPool::free(void* data)
	{
		//Make sure this data is part of the pool.
		assert(reinterpret_cast<std::uint64_t>(data) >= reinterpret_cast<std::uint64_t>(pool));
		assert(reinterpret_cast<std::uint64_t>(data) <= reinterpret_cast<std::uint64_t>(pool) + (static_cast<std::uint64_t>(size) * static_cast<std::uint64_t>(typeSize)));

		//Index in the array
		const auto index = static_cast<uint32_t>((static_cast<std::uint8_t*>(data) - pool) / typeSize);

		//Make sure this data is not free already
		assert(isFree[index] == false && "Trying to free memory that was not allocated!");

		//Make the last free element point to the now free one.
		next[last] = index;

		//The now free element is the last in the list, nullptr makes sure it asserts if it tries to allocate more.
		next[index] = -1;

		//Set the thing to be free
		isFree[index] = true;

		//Set lastFree to now point to the new last element
		last = index;

		//If the pool was full, and the current index is -1, make it point to the correct index now.
		if (current < 0)
		{
			current = last;
		}

		//Decrease object count.
		--objectCount;
	}

	std::uint16_t TypelessPool::getTypeSize() const
	{
		return typeSize;
	}

	std::uint16_t TypelessPool::getSize() const
	{
		return size;
	}

	TypelessPool::~TypelessPool()
	{
		//Free up the pool again.
		std::free(pool);
	}
}
