// Copyright Low Entry. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ELowEntryCompressionResult.h"

#include "LowEntryCompressionLibrary.generated.h"


UCLASS()
class LOWENTRYCOMPRESSION_API ULowEntryCompressionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


public:
	/**
	* Compresses a Byte Array with LZF.
	*/
	UFUNCTION(BlueprintPure, Category = "Low Entry|Compression", Meta = (DisplayName = "Compress (Lzf)"))
		static TArray<uint8> CompressLzf(const TArray<uint8>& Data);
		
	static TArray<uint8> CompressLzfThreadSafe(const TArray<uint8>& Data);

	/**
	* Tries to decompress a Byte Array using the LZF algorithm. Will return an empty Array on failure.
	*/
	UFUNCTION(BlueprintPure, Category = "Low Entry|Compression", Meta = (DisplayName = "Decompress (Lzf)"))
		static TArray<uint8> DecompressLzf(const TArray<uint8>& CompressedData);
};
