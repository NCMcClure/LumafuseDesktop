// Copyright Low Entry. All Rights Reserved.

#include "LowEntryCompressionByteArrayBuffer.h"


ULowEntryCompressionByteArrayBuffer::ULowEntryCompressionByteArrayBuffer()
{
	buffer.SetNum(32);
}

ULowEntryCompressionByteArrayBuffer::ULowEntryCompressionByteArrayBuffer(int64 initialSize)
{
	buffer.SetNum((initialSize > 0) ? initialSize : 32);
}

ULowEntryCompressionByteArrayBuffer::ULowEntryCompressionByteArrayBuffer(int64 initialSize, int64 maxInitialSize)
{
	initialSize = (initialSize <= maxInitialSize) ? initialSize : maxInitialSize;
	buffer.SetNum((initialSize > 0) ? initialSize : 32);
}
