// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/MultiThreadTask.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "ScreenRendering.h"
#include "CommonRenderResources.h"
#include "SocketServerPluginUDPServer.h"
#include "LowEntryExtendedStandardLibrary/Public/Classes/LowEntryExtendedStandardLibrary.h"
#include "LowEntryCompression/Public/Classes/LowEntryCompressionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LumafuseBufferBlockWorker.generated.h"

/**
 * 
 */
UCLASS()
class LUMAFUSEDESKTOP_API ULumafuseBufferBlockWorker : public UMultiThreadTask
{
	GENERATED_BODY()

	void CopyTextureBlock(const FTexture2DRHIRef& SourceTexture, FTexture2DRHIRef& DestinationTexture, FIntPoint BlockPosition, FIntPoint GridLayout);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse | Multithreading | Rendering")
	void GetPixelBufferBlockFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget, FIntPoint BlockPosition, FIntPoint GridLayout, UPARAM(ref)TArray<uint8>& Buffer, int32 CompressionQuality);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse | Multithreading | Rendering")
	void GetPixelBufferFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget,UPARAM(ref)TArray<uint8>& Buffer, int32 CompressionQuality);
	
	void CompressPixelsToBuffer(TArray<FColor>& SurfaceData, TArray<uint8>& Buffer, int32 SizeX, int32 SizeY, int32 CompressionQuality);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse | Networking | Display")
	void SeparateAndSendBufferBlock(uint8 DisplayID, uint8 FrameID, FIntPoint BlockCoordinate, const TArray<uint8>& BufferBlock, USocketServerBPLibrary* ServerTarget, FString ClientSessionID
											, FString OptionalServerID);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse | Multithreading | Rendering")
	void FlushRenderThreadCommands(bool bFlushDeferredDeletes)
	{
		FlushRenderingCommands(bFlushDeferredDeletes);
	}
	
};
