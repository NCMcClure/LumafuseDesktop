// Copyright Low Entry. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"


class ULowEntryCompressionByteArrayBuffer
{
public:
	ULowEntryCompressionByteArrayBuffer();
	ULowEntryCompressionByteArrayBuffer(int64 initialSize);
	ULowEntryCompressionByteArrayBuffer(int64 initialSize, int64 maxInitialSize);


private:
	TArray<uint8>	buffer;
	int64			length = 0;


private:
	FORCEINLINE void access(int64 index)
	{
		if(index >= buffer.Num())
		{
			int64 newSize = buffer.Num();
			while(index >= newSize)
			{
				newSize = (newSize >= 1073741823) ? 2147483647 : newSize * 2;
			}
			buffer.SetNumUninitialized(newSize);
		}
		if((index + 1) > length)
		{
			length = index + 1;
		}
	}


public:
	FORCEINLINE void set(int64 index, uint8 value)
	{
		access(index);
		buffer[index] = value;
	}

	FORCEINLINE void set(int64 index, const TArray<uint8>& values, int64 valuesOffset, int64 length0)
	{
		access(index + (length0 - 1));
		FMemory::Memcpy(buffer.GetData() + index, values.GetData() + valuesOffset, length0);
	}

	FORCEINLINE uint8 get(int64 index)
	{
		if(index >= buffer.Num())
		{
			return 0;
		}
		return buffer[index];
	}

	FORCEINLINE TArray<uint8> getData()
	{
		buffer.SetNum(length, true);
		return buffer;
	}

	FORCEINLINE TArray<uint8> getData(int64 length0)
	{
		access(length0 - 1);
		if(length0 < length)
		{
			TArray<uint8> copy;
			copy.Append(buffer.GetData(), length0);
			return copy;
		}
		buffer.SetNum(length0, true);
		return buffer;
	}
};
