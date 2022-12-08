// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Delegates/IDelegateInstance.h"
#include "Tickable.h"
#include "HAL/ThreadSafeBool.h"
#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "Templates/SubclassOf.h"
#include "UObject/Package.h"
#include "MultiTask2UtilitiesLibrary.h"
#include "MultiTaskBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FMultiTaskOnCancelDelegate);

UCLASS(HideDropdown, BlueprintType, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UMultiTaskBase : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
    UMultiTaskBase();
    ~UMultiTaskBase();

    /**
    * Attempts to start the Task. 
    * @return Return False if Task already running.
    */
    virtual bool Start();

    /**
    * Attempts to Cancel the Task. 
    */
    virtual void Cancel();

    /**
    * Check whether the task is in progress.
    */
    virtual bool IsRunning();

    /**
    * Check whether the task is canceled.
    */
    virtual bool IsCanceled();

    /**
    * Called immediately on Game Thread when the Task is cancelled. 
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "On Cancel"), Category = "Events")
        void OnCancel();
    virtual void OnCancel_Implementation();

    /**
    * Called immediately on Game Thread when the Task is completed. 
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "On Complete"), Category = "Events")
        void OnComplete();
    virtual void OnComplete_Implementation();

    /**
    * Called on Tick (if enabled) on Game Thread. 
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "On Tick"), Category = "Events")
        void OnTick(float DeltaTime);
    virtual void OnTick_Implementation(float DeltaTime);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick")
        bool bIsTickable = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick")
        bool bIsTickableInEditor = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick")
        bool bIsTickableWhenPaused = false;

    FMultiTaskOnCancelDelegate OnCancelDelegate;
    TFunction<void()> BodyFunction;
protected:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual bool IsTickableInEditor() const override;
    virtual bool IsTickableWhenPaused() const override;
    virtual UWorld* GetTickableGameObjectWorld() const override;
    virtual UWorld* GetWorld() const override;

private:
    virtual TStatId GetStatId() const override;

protected:
	FThreadSafeBool bCanceled = false;

};


UENUM(BlueprintType)
enum class EMultiTask2Branches : uint8
{
	OnStart,
	OnCompleted,
	OnCanceled,
};

UENUM(BlueprintType)
enum class EMultiTask2BranchesWithBody : uint8
{
    OnStart,
    OnTaskBody,
    OnCompleted,
    OnCanceled,
};

UENUM(BlueprintType)
enum class EMultiTask2BranchesNoCancel : uint8
{
    OnStart,
    OnCompleted,
};

UENUM(BlueprintType)
enum class EMultiTask2BranchesNoComplete : uint8
{
    OnStart,
    OnCanceled,
};

UENUM(BlueprintType)
enum class EMultiTask2BranchesNoCompleteWithBody : uint8
{
    OnStart,
    OnTaskBody,
    OnCanceled,
};


class MULTITASK2_API FSingleTaskActionBase : public FPendingLatentAction
{
protected:
    UObject* Object;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
    UMultiTaskBase* Task = nullptr;
public:
    FSingleTaskActionBase(UObject* InObject, const FLatentActionInfo& LatentInfo, TSubclassOf<class UMultiTaskBase> TaskClass)
        : Object(InObject)
        , ExecutionFunction(LatentInfo.ExecutionFunction)
        , OutputLink(LatentInfo.Linkage)
        , CallbackTarget(LatentInfo.CallbackTarget)
	{
        UMultiTask2UtilitiesLibrary::TaskIndex++;
        const FString Name = "MultiTaskJob" + FString::FromInt(UMultiTask2UtilitiesLibrary::TaskIndex);
        Task = NewObject<UMultiTaskBase>(InObject, TaskClass, FName(*Name), RF_Transient);
        UMultiTask2UtilitiesLibrary::AddToRoot(Task);

        if (UObject* CallbackObject = CallbackTarget.Get())
        {
            if (UFunction* Function = CallbackObject->FindFunction(ExecutionFunction))
            {
                checkf(CallbackObject && Function, TEXT("Something went horibly wrong."));
                int32 LocalOutputLink = OutputLink;
                Task->BodyFunction = [CallbackObject, Function, LocalOutputLink]()
                {
                    int32 FinalOutputLink = LocalOutputLink;
                    if (CallbackObject && Function)
                    {
                        CallbackObject->ProcessEvent(Function, &(FinalOutputLink));
                    }
                };
            }
        }
    }

	virtual ~FSingleTaskActionBase()
	{
        if (Task!= nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
        {
            Task->Cancel();
            UMultiTask2UtilitiesLibrary::RemoveFromRoot(Task);
        }
	}

	virtual bool IsRunning()
	{
        if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
        {
            return Task->IsRunning();
        }
        return false;
	}

	virtual bool IsCanceled()
	{
        if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
        {
            return Task->IsCanceled();
        }
        return true;
	}
};

class MULTITASK2_API FMultiTaskActionBase : public FPendingLatentAction
{
protected:
    UObject* Object;
    FName ExecutionFunction;
    int32 OutputLink;
    FWeakObjectPtr CallbackTarget;
    TArray<UMultiTaskBase*> Tasks;
    TFunction<void()> BodyFunction;
public:
    FMultiTaskActionBase(UObject* InObject, const FLatentActionInfo& LatentInfo, TSubclassOf<class UMultiTaskBase> TaskClass, int32 Count)
        : Object(InObject)
        , ExecutionFunction(LatentInfo.ExecutionFunction)
        , OutputLink(LatentInfo.Linkage)
        , CallbackTarget(LatentInfo.CallbackTarget)
    {
        Tasks.SetNumZeroed(Count);

        if (TaskClass && Count > 0)
        {
            for (int32 X = 0; X < Count; ++X)
            {
                UMultiTask2UtilitiesLibrary::TaskIndex++;
                const FString Name = "MultiTaskJob" + FString::FromInt(UMultiTask2UtilitiesLibrary::TaskIndex);
                Tasks[X] = NewObject<UMultiTaskBase>(InObject, TaskClass, FName(*Name), RF_Transient);
                UMultiTask2UtilitiesLibrary::AddToRoot(Tasks[X]);
            }
        }

        if (UObject* CallbackObject = CallbackTarget.Get())
        {
            if (UFunction* Function = CallbackObject->FindFunction(ExecutionFunction))
            {
                checkf(CallbackObject && Function, TEXT("Something went horibly wrong."));
                int32 LocalOutputLink = OutputLink;
                BodyFunction = [CallbackObject, Function, LocalOutputLink]()
                {
                    int32 FinalOutputLink = LocalOutputLink;
                    if (CallbackObject && Function)
                    {
                        CallbackObject->ProcessEvent(Function, &(FinalOutputLink));
                    }
                };
            }
        }
    }

    virtual ~FMultiTaskActionBase()
    {
        for (auto Task : Tasks)
        {
            if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
            {
                Task->Cancel();
                UMultiTask2UtilitiesLibrary::RemoveFromRoot(Task);
            }
        }
    }

    virtual bool IsRunning()
    {
        for (auto Task : Tasks)
        {
            if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
            {
                if (Task->IsRunning())
                {
                    return true;
                }
            }
        }
        return false;
    }
};