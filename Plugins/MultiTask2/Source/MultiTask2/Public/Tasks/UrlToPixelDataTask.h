// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "MultiThreadTask.h"
#include "PixelReaderTask.h"
#include "Interfaces/IHttpRequest.h"
#include "UrlToPixelDataTask.generated.h"

UCLASS(HideDropdown, NotBlueprintable, NotBlueprintType, hidecategories = (Object, General), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UUrlToPixelDataTask : public UMultiThreadTask
{

	GENERATED_BODY()

public:

	virtual bool Start() override;

	virtual void Cancel() override;

	virtual void TaskBody_Implementation() override;

	virtual bool IsRunning() override;

public:
	FString URL;
	float Timeout = 0.0f;
	FPixelData PixelData;
private:
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ImageRequest;
};

class MULTITASK2_API FUrlToPixelDataTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	FPixelData& PixelData;
	bool bStarted;
public:
	FUrlToPixelDataTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, const FString& URL, float Timeout, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask, FPixelData& InPixelData)
		: FSingleTaskActionBase(InObject, LatentInfo, UUrlToPixelDataTask::StaticClass())
		, Branches(InBranches)
		, PixelData(InPixelData)
		, bStarted(false)
	{
		OutTask = Task;

		UUrlToPixelDataTask* LocalTask = Cast<UUrlToPixelDataTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();
			LocalTask->URL = URL;
			LocalTask->Timeout = FMath::Clamp(Timeout, 0.0f, Timeout);
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
					UUrlToPixelDataTask* LocalTask = Cast<UUrlToPixelDataTask>(Task);
					if (LocalTask)
					{
						PixelData = MoveTemp(LocalTask->PixelData);
					}
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else {
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