// Fill out your copyright notice in the Description page of Project Settings.


#include "LumafuseTextureUtilities.h"
#include "RenderingThread.h"
#include "LowEntryCompressionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LowEntryExtendedStandardLibrary/Public/Classes/LowEntryExtendedStandardLibrary.h"
#include "UnrealClient.h"

ULumafuseTextureUtilities::ULumafuseTextureUtilities()
{
}

ULumafuseTextureUtilities::~ULumafuseTextureUtilities()
{
}

UTexture2D* ULumafuseTextureUtilities::CreateTexture(int32 Width, int32 Height)
{
	auto Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	Texture->UpdateResource();

	return Texture;
}

void ULumafuseTextureUtilities::UpdateTexture(UTexture2D* Texture, const TArray<uint8>& PixelsBuffer, int32 SourcePitch)
{
	auto Region = new FUpdateTextureRegion2D(0, 0, 0, 0, Texture->GetSizeX(), Texture->GetSizeY());
	Texture->UpdateTextureRegions(0, 1, Region, SourcePitch * Texture->GetSizeX(), SourcePitch, (uint8*)PixelsBuffer.GetData());
}









