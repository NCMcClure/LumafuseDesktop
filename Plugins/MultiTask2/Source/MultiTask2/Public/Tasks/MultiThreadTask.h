// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ThreadTaskBase.h"
#include "MultiThreadTask.generated.h"

DECLARE_MULTICAST_DELEGATE(FMultiThreadTaskDelegate);

UCLASS(HideDropdown, Blueprintable, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UMultiThreadTask : public UThreadTaskBase
{

    GENERATED_BODY()

public:

    virtual bool Start() override;

	/**
	* Called on Background Thread when the Task is executed.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "Task Body"), Category = "Events")
		void TaskBody();
	virtual void TaskBody_Implementation();
public:
	FMultiThreadTaskDelegate TaskDelegate;
};

class UMultiTaskThreadPool;

class MULTITASK2_API FSingleThreadTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	bool bStarted;
public:
	FSingleThreadTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class UMultiThreadTask> TaskClass, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, TaskClass)
		, Branches(InBranches)
		, bStarted(false)
	{
		OutTask = Task;

		UMultiThreadTask* LocalTask = Cast<UMultiThreadTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();

			LocalTask->ExecutionType = InExecutionType;
			LocalTask->ThreadPool = ThreadPool;
			bStarted = Task->Start();
		}
		else {
			return;
		}
	}


	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bStarted)
		{
			if (!IsCanceled())
			{
				if (!IsRunning())
				{
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				Branches = EMultiTask2Branches::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2Branches::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};

class MULTITASK2_API FSingleThreadTaskWithBodyAction : public FSingleTaskActionBase
{
	EMultiTask2BranchesWithBody& Branches;
	bool bStarted;
public:
	FSingleThreadTaskWithBodyAction(UObject* InObject, EMultiTask2BranchesWithBody& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class UMultiThreadTask> TaskClass, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, TaskClass)
		, Branches(InBranches)
		, bStarted(false)
	{
		OutTask = Task;

		UMultiThreadTask* LocalTask = Cast<UMultiThreadTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2BranchesWithBody::OnStart;
			LocalTask->BodyFunction();
			LocalTask->ExecutionType = InExecutionType;
			LocalTask->ThreadPool = ThreadPool;

			LocalTask->TaskDelegate.AddLambda([LocalTask, &InBranches]
			{ 
				InBranches = EMultiTask2BranchesWithBody::OnTaskBody;
				LocalTask->BodyFunction();
			});
			LocalTask->OnCancelDelegate.AddLambda([LocalTask, &InBranches]
			{
				InBranches = EMultiTask2BranchesWithBody::OnCanceled;
				LocalTask->BodyFunction();
			});

			bStarted = Task->Start();
		}
		else {
			return;
		}
	}

	virtual ~FSingleThreadTaskWithBodyAction()
	{
		if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
		{
			UMultiThreadTask* LocalTask = Cast<UMultiThreadTask>(Task);
			if (LocalTask)
			{
				LocalTask->TaskDelegate.RemoveAll(this);
				LocalTask->OnCancelDelegate.RemoveAll(this);
			}
		}
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bStarted)
		{
			if (!IsCanceled())
			{
				if (!IsRunning())
				{
					Branches = EMultiTask2BranchesWithBody::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				Response.DoneIf(true);
			}
		}
		else {
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2BranchesWithBody::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};


class MULTITASK2_API FMultiThreadTaskAction : public FMultiTaskActionBase
{
	EMultiTask2BranchesNoCancel& Branches;
	bool bStarted;
public:
	FMultiThreadTaskAction(UObject* InObject, EMultiTask2BranchesNoCancel& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class UMultiThreadTask> TaskClass, int32 Count, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, TArray<UMultiTaskBase*>& OutTasks)
		: FMultiTaskActionBase(InObject, LatentInfo, TaskClass, Count)
		, Branches(InBranches)
		, bStarted(false)
	{
		OutTasks = Tasks;

		Branches = EMultiTask2BranchesNoCancel::OnStart;
		BodyFunction();

		int32 TasksStarted = 0;
		for (auto Task : Tasks)
		{
			if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
			{
				UMultiThreadTask* LocalTask = Cast<UMultiThreadTask>(Task);
				if (LocalTask)
				{
					LocalTask->ExecutionType = InExecutionType;
					LocalTask->ThreadPool = ThreadPool;
					if (Task->Start())
					{
						TasksStarted++;
					}

				}
			}
		}
		if (TasksStarted > 0)
		{
			bStarted = true;
		}

	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bStarted)
		{
			if (!IsRunning())
			{
				Branches = EMultiTask2BranchesNoCancel::OnCompleted;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2BranchesNoCancel::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};