// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/MultiThreadTask.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "ScreenRendering.h"
#include "CommonRenderResources.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LumafuseBufferQuadrantWorker.generated.h"

/**
 * 
 */
UCLASS()
class LUMAFUSEDESKTOP_API ULumafuseBufferQuadrantWorker : public UMultiThreadTask
{
	GENERATED_BODY()

	void CopyTextureBlock(const FTexture2DRHIRef& SourceTexture, FTexture2DRHIRef& DestinationTexture, FIntPoint BlockPosition, FIntPoint GridLayout);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse | Multithreading | Rendering")
	void GetPixelBufferBlockFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget, FIntPoint BlockPosition, FIntPoint GridLayout, UPARAM(ref)TArray<uint8>& Buffer, int32 CompressionQuality);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse | Multithreading | Rendering")
	void GetPixelBufferFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget,UPARAM(ref)TArray<uint8>& Buffer, int32 CompressionQuality);
	
	void CompressPixelsToBuffer(TArray<FColor>& SurfaceData, TArray<uint8>& Buffer, int32 SizeX, int32 SizeY, int32 CompressionQuality);
	
};
