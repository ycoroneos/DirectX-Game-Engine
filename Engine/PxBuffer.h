// ===============================================================================// File: NxBuffer.h//// Copyright © Steven Richard Batchelor. All rights reserved.//// ===============================================================================#include "NxPhysics.h"
#include "PxStream.h"
class NxBuffer : public PxStream
{public:	
	mutable	PxU32	loc;	
	PxU32					currentSize;	
	PxU32					maxSize;	
	PxU8*					data;	
	NxBuffer() : loc(0), currentSize(0), maxSize(0), data(NULL)	{	}	
	~NxBuffer()	{		PX_DELETE_ARRAY(data);	}	
	void clear()	{		currentSize = 0;	}	
	PxU8 readByte() const	
	{		NxU8 b;		
		memcpy(&b, data + loc, sizeof(NxU8));		
		loc += sizeof(NxU8);		
		return b;	
	}	
	PxU16 readWord() const	
	{		NxU16 w;		
			memcpy(&w, data + loc, sizeof(NxU16));		
			loc += sizeof(NxU16);		
			return w;	
	}	
	PxU32 readDword() const	
	{		NxU32 d;		
			memcpy(&d, data + loc, sizeof(NxU32));		
			loc += sizeof(NxU32);		
			return d;	
	}	
	float readFloat() const	
	{		float f;		
			memcpy(&f, data + loc, sizeof(float));		
			loc += sizeof(float);		
			return f;	
	}	
	double readDouble() const	
	{		double f;		
			memcpy(&f, data + loc, sizeof(double));		
			loc += sizeof(double);		
			return f;	
	}	
	void readBuffer(void* dest, NxU32 size) const	
	{		memcpy(dest, data + loc, size);		
			loc += size;	
	}	
	PxStream& storeByte(NxU8 b)	
	{		storeBuffer(&b, sizeof(NxU8));		
			return *this;	
	}	
	PxStream& storeWord(NxU16 w)	
	{		storeBuffer(&w, sizeof(NxU16));		
			return *this;	
	}	
	PxStream& storeDword(NxU32 d)	
	{		storeBuffer(&d, sizeof(NxU32));		
			return *this;	
	}	
	PxStream& storeFloat(NxReal f)	
	{		storeBuffer(&f, sizeof(NxReal));		
		return *this;	
	}	
	PxStream& storeDouble(NxF64 f)	
	{		storeBuffer(&f, sizeof(NxF64));		
			return *this;	
	}	
	PxStream& storeBuffer(const void* buffer, NxU32 size)	
	{		NxU32 expectedSize = currentSize + size;		
			if(expectedSize > maxSize)		
			{			maxSize = expectedSize + 4096;			
						NxU8* newData = new NxU8[maxSize];			
						NX_ASSERT(newData!=NULL);			
						if(data)			
						{				memcpy(newData, data, currentSize);				
										delete[] data;			
						}				data = newData;		
			}		
			memcpy(data+currentSize, buffer, size);		
			currentSize += size;		
			return *this;	
	}
};