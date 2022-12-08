// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Misc/QueuedThreadPool.h"
#include "UObject/Object.h"
#include "Templates/SharedPointer.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#include "MultiTaskThreadPool.generated.h"

UENUM(BlueprintType)
enum class EMultiTaskThreadPriority : uint8
{
	Normal,
	AboveNormal,
	BelowNormal,
	Highest,
	Lowest,
	SlightlyBelowNormal,
	TimeCritical,
};


UCLASS(BlueprintType, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UMultiTaskThreadPool : public UObject
{
	GENERATED_BODY()
public:


	bool Create(uint32 InNumQueuedThreads, uint32 StackSize = (32 * 1024), EThreadPriority ThreadPriority = TPri_Normal, const FString Name = "UnknownThreadPool");

	UFUNCTION(BlueprintPure, Category = "Thread Pool")
		int32 GetThreadsNum();

public:
	TSharedPtr <FQueuedThreadPool> Obj;
};