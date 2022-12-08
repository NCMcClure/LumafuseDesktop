// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "MultiThreadTask.h"
#include "PixelReaderTask.h"
#include "FileToPixelDataTask.generated.h"

UCLASS(HideDropdown, NotBlueprintable, NotBlueprintType, hidecategories = (Object, General), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UFileToPixelDataTask : public UMultiThreadTask
{

	GENERATED_BODY()

public:

	virtual bool Start() override;

	virtual void TaskBody_Implementation() override;

	//virtual bool IsRunning() override;

public:
	FString File;
	FPixelData PixelData;
};

class MULTITASK2_API FFileToPixelDataTaskAction : public FSingleTaskActionBase
{
	EMultiTask2BranchesNoCancel& Branches;
	FPixelData& PixelData;
	bool bStarted;
public:
	FFileToPixelDataTaskAction(UObject* InObject, EMultiTask2BranchesNoCancel& InBranches, const FLatentActionInfo& LatentInfo, const FString& File, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool/*, UMultiTaskBase*& OutTask*/, FPixelData& InPixelData)
		: FSingleTaskActionBase(InObject, LatentInfo, UFileToPixelDataTask::StaticClass())
		, Branches(InBranches)
		, PixelData(InPixelData)
		, bStarted(false)
	{
		//OutTask = Task;

		UFileToPixelDataTask* LocalTask = Cast<UFileToPixelDataTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2BranchesNoCancel::OnStart;
			LocalTask->BodyFunction();
			LocalTask->File = File;
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
			if (!IsRunning())
			{
				UFileToPixelDataTask* LocalTask = Cast<UFileToPixelDataTask>(Task);
				if (LocalTask)
				{
					PixelData = MoveTemp(LocalTask->PixelData);
				}
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