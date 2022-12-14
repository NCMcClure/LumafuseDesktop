// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LumafuseFramePacket.h"
#include "SocketServerPluginUDPServer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/IntRect.h"
#include "LumafuseStreamingUtilities.generated.h"


/**
 * 
 */
UCLASS()
class LUMAFUSEDESKTOP_API ULumafuseStreamingUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "LumafuseStreamingUtilities|FrameDataConstruction")
	static void DistillChunkPacketsAndSendToClient(uint8 DisplayID, uint8 FrameID, int32 SplitSize, int32 ChunkIndex, const TArray<uint8>& BufferChunk, USocketServerBPLibrary* ServerTarget, FString ClientSessionID
											, FString OptionalServerID);

	UFUNCTION(BlueprintCallable, Category = "LumafuseStreamingUtilities|FrameDataConstruction")
	static void OptimizeAndSendFrameChunkPackets(const TArray<FLumafuseFramePacket>& frameBufferDataChunk,
	                                        USocketServerBPLibrary* serverTarget, FString clientSessionID,
	                                        FString messageToSend, FString optionalServerID);

	//Write a new function that takes in a TArray of uint8s and removes every 4th element from the array
	UFUNCTION(BlueprintCallable, Category = "LumafuseStreamingUtilities|FrameDataConstruction")
	static void TrimAlphaFromPacket(const TArray<uint8>& OriginalPacket, UPARAM(ref)TArray<uint8>& TrimmedPacket);
};
