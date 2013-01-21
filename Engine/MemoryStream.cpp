#include "MemoryStream.h"
MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
{
}

MemoryWriteBuffer::~MemoryWriteBuffer()
{
	delete [] data;
}

void MemoryWriteBuffer::clear()
{
	currentSize = 0;
}

void MemoryWriteBuffer::readBuffer(void*, PxU32) const
{
	PX_ASSERT(0); 
}

PxStream& MemoryWriteBuffer::storeByte(PxU8 b)
{
	storeBuffer(&b, sizeof(PxU8));
	return *this;
}

PxStream& MemoryWriteBuffer::storeWord(PxU16 w)
{
	storeBuffer(&w, sizeof(PxU16));
	return *this;
}

PxStream& MemoryWriteBuffer::storeDword(PxU32 d)
{
	storeBuffer(&d, sizeof(PxU32));
	return *this;
}

PxStream& MemoryWriteBuffer::storeFloat(PxReal f)
{
	storeBuffer(&f, sizeof(PxReal));
	return *this;
}

PxStream& MemoryWriteBuffer::storeDouble(PxF64 f)
{
	storeBuffer(&f, sizeof(PxF64));
	return *this;
}

PxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, PxU32 size)
{
	PxU32 expectedSize = currentSize + size;
	if(expectedSize > maxSize)
	{
		maxSize = expectedSize + 4096;

		PxU8* newData = new PxU8[maxSize];
		PX_ASSERT(newData!=NULL);

		if(data)
		{
			Ps::memCopy(newData, data, currentSize);
			delete[] data;
		}
		data = newData;
	}
	Ps::memCopy(data+currentSize, buffer, size);
	currentSize += size;
	return *this;
}