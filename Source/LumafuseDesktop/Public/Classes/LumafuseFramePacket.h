// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LumafuseFramePacket.generated.h"

USTRUCT(BlueprintType)
struct FLumafuseFramePacket
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	uint8 displayID;

	UPROPERTY(BlueprintReadWrite)
	uint8 frameID;

	UPROPERTY(BlueprintReadWrite)
	int32 index;

	UPROPERTY(BlueprintReadWrite)
	int32 packetSize;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> pixelPacket;
	
};
