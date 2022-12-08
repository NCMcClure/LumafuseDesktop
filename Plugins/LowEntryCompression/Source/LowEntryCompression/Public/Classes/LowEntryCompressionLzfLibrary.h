// Copyright Low Entry. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"


class ULowEntryCompressionByteArrayBuffer;


class ULowEntryCompressionLzfLibrary
{
private:
	const static int32	SKIP_LENGTH = 15;

	const static int32	CACHED_HASH_SIZE = 1 << 16;
	const static int32	NONCACHED_HASH_SIZE = 1 << 10;
	const static int32	MAX_LITERAL = 1 << 5;
	const static int32	MAX_OFF = 1 << 13;
	const static int32	MAX_REF = (1 << 8) + (1 << 3);

	static int32*		cachedHashTab;


private:
	FORCEINLINE static int32 UintByteCount(int32 Value);
	FORCEINLINE static void UintToBytes(ULowEntryCompressionByteArrayBuffer& Array, int32 ArrayOffset, int32 Value);
	FORCEINLINE static int32 BytesToUint(const TArray<uint8>& Array, int32 ArrayOffset);


public:
	static TArray<uint8> Compress(const TArray<uint8>& Bytes, const bool ThreadSafe = false);
	static TArray<uint8> Decompress(const TArray<uint8>& Bytes);
};
