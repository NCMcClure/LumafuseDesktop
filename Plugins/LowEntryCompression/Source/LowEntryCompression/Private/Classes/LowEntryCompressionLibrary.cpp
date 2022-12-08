// Copyright Low Entry. All Rights Reserved.

#include "LowEntryCompressionLibrary.h"
#include "LowEntryCompressionLzfLibrary.h"


// init >>
	ULowEntryCompressionLibrary::ULowEntryCompressionLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
	}
// init <<


TArray<uint8> ULowEntryCompressionLibrary::CompressLzf(const TArray<uint8>& Data)
{
	return ULowEntryCompressionLzfLibrary::Compress(Data);
}

TArray<uint8> ULowEntryCompressionLibrary::CompressLzfThreadSafe(const TArray<uint8>& Data)
{
	return ULowEntryCompressionLzfLibrary::Compress(Data, true);
}

TArray<uint8> ULowEntryCompressionLibrary::DecompressLzf(const TArray<uint8>& CompressedData)
{
	return ULowEntryCompressionLzfLibrary::Decompress(CompressedData);
}
