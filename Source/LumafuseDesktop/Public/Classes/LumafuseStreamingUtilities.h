// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LumafuseFramePacket.h"
#include "SocketServerPluginUDPServer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LumafuseStreamingUtilities.generated.h"

/**
 * 
 */
UCLASS()
class LUMAFUSEDESKTOP_API ULumafuseStreamingUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "LumafuseStreamingUtilities|FrameDataConstruction")
	static void BuildFrameDataChunk(uint8 frameID, int32 splitSize, int32 chunkIndex, const TArray<uint8>& bufferChunk,
	                                UPARAM(ref)TArray<FLumafuseFramePacket>& frameData);

	UFUNCTION(BlueprintCallable, Category = "LumafuseStreamingUtilities|FrameDataConstruction")
	static void OptimizeAndSendFrameChunkPackets(const TArray<FLumafuseFramePacket>& frameBufferDataChunk,
	                                        USocketServerBPLibrary* serverTarget, FString clientSessionID,
	                                        FString messageToSend, FString optionalServerID);
	
};
