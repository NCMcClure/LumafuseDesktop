// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiTaskMutex.generated.h"


UCLASS(BlueprintType, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UMultiTaskMutex : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Mutex")
		bool TryLock();

	UFUNCTION(BlueprintCallable, Category = "Mutex")
		void Lock();

	UFUNCTION(BlueprintCallable, Category = "Mutex")
		void Unlock();

public:
	FCriticalSection Section;
};