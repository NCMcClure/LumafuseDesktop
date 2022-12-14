// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "Templates/UnrealTemplate.h"
#include "LumafuseTextureUtilities.generated.h"

/**
 * 
 */
UCLASS()
class LUMAFUSEDESKTOP_API ULumafuseTextureUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	ULumafuseTextureUtilities();
	~ULumafuseTextureUtilities();

	UFUNCTION(BlueprintPure, Category = "Lumafuse|Texture Utilities")
	static UTexture2D* CreateTexture(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable, Category = "Lumafuse|Texture Utilities")
	static void UpdateTexture(UTexture2D* Texture, const TArray<uint8>& PixelsBuffer, int32 SourcePitch);
	
};
