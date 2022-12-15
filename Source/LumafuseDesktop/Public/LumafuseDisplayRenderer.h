// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "ScreenRendering.h"
#include "CommonRenderResources.h"
#include "Templates/UnrealTemplate.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LumafuseDisplayRenderer.generated.h"

UCLASS()
class LUMAFUSEDESKTOP_API ALumafuseDisplayRenderer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALumafuseDisplayRenderer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Lumafuse")
	void FlushRenderThreadCommands()
	{
		FlushRenderingCommands();
	}
	
};
