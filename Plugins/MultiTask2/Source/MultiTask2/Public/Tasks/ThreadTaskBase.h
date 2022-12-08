// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiTaskBase.h"
#include "Async/Async.h"
#include "ThreadTaskBase.generated.h"

UENUM(BlueprintType)
enum class ETaskExecutionType : uint8
{
    /** Execute in Task Graph (for short running tasks). */
    TaskGraph,

    /** Execute in separate thread (for long running tasks). */
    Thread,

    /** Execute in global queued thread pool. */
    ThreadPool
};

class UMultiTaskThreadPool;

UCLASS(NotBlueprintType, NotBlueprintable)
class MULTITASK2_API UThreadTaskBase : public UMultiTaskBase
{
	GENERATED_BODY()

public:
    UThreadTaskBase();
    ~UThreadTaskBase();

    /**
    * Check whether the job is in progress.
    */
    virtual bool IsRunning() override;

	/**
    * Wait for work job to complete.
    */
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Wait To Finish"), Category = "Task")
        virtual void WaitToFinish();

private:
    void OnEndPIE(bool bIsSimulating);
    void OnPreExit();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
        ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
        UMultiTaskThreadPool* ThreadPool;
protected:
    TArray<TFuture<void>> Tasks;

private:
#if WITH_EDITOR
    FDelegateHandle EndPIEHandle;
#else
    FDelegateHandle PreExitHandle;
#endif
};