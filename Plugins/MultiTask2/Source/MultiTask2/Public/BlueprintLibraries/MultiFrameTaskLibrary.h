// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiTaskBase.h"
#include "MultiFrameTaskLibrary.generated.h"

UCLASS()
class MULTITASK2_API UMultiFrameTaskLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    /**
    * Spread a task over multiple frames.
	* This executes on Game Thread.
	* @param IterationsPerTick	The amount of iterations processed per frame.
	* @param Delay				Amount of seconds to wait after IterationsPerTick is reached.
	* @param Task				Running Task.
    */
	UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Multi-Frame", Meta = (DisplayName = "Do Async Task", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void DoAsyncTask(UObject* WorldContextObject, EMultiTask2BranchesNoCompleteWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameAsyncTask> Class, UMultiTaskBase*& Task, int32 IterationsPerTick = 1, float Delay = 0.0f);

	/**
	* Spread a 1D Loop over multiple frames.
	* This executes on Game Thread.
	* @param XSize				X Dimension size.
	* @param IterationsPerTick	The amount of iterations processed per frame.
	* @param Delay				Amount of seconds to wait after IterationsPerTick is reached.
	* @param Task				Running Task.
	*/
	UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Multi-Frame", Meta = (DisplayName = "Do Loop 1D Task", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void DoLoop1DTask(UObject* WorldContextObject, EMultiTask2BranchesWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameLoop1DTask> Class, int32& X, UMultiTaskBase*& Task, int32 XSize = 1, int32 IterationsPerTick = 1, float Delay = 0.0f);

	/**
    * Spread a 2D Loop over multiple frames.
    * This executes on Game Thread.
    * @param XSize				X Dimension size.
    * @param YSize				Y Dimension size.
    * @param IterationsPerTick	The amount of iterations processed per frame.
    * @param Delay				Amount of seconds to wait after IterationsPerTick is reached.
	* @param Task				Running Task.
    */
	UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Multi-Frame", Meta = (DisplayName = "Do Loop 2D Task", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void DoLoop2DTask(UObject* WorldContextObject, EMultiTask2BranchesWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameLoop2DTask> Class, int32& X, int32& Y, UMultiTaskBase*& Task, int32 XSize = 1, int32 YSize = 1, int32 IterationsPerTick = 1, float Delay = 0.0f);

	/**
	* Spread a 3D Loop over multiple frames.
	* This executes on Game Thread.
	* @param XSize				X Dimension size.
	* @param YSize				Y Dimension size.
	* @param ZSize				Z Dimension size.
	* @param IterationsPerTick	The amount of iterations processed per frame.
	* @param Delay				Amount of seconds to wait after IterationsPerTick is reached.
	* @param Task				Running Task.
	*/
	UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Multi-Frame", Meta = (DisplayName = "Do Loop 3D Task", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void DoLoop3DTask(UObject* WorldContextObject, EMultiTask2BranchesWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameLoop3DTask> Class, int32& X, int32& Y, int32& Z, UMultiTaskBase*& Task, int32 XSize = 1, int32 YSize = 1, int32 ZSize = 1, int32 IterationsPerTick = 1, float Delay = 0.0f);

};