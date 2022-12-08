// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiThreadTask.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#include "DelaunayTriangulation2DTask.generated.h"

USTRUCT(BlueprintType)
struct MULTITASK2_API FMultiTask2Delaunay2DTriangle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DelaunayTriangle)
		int32 a;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DelaunayTriangle)
		int32 b;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DelaunayTriangle)
		int32 c;

	FMultiTask2Delaunay2DTriangle()
		: a(0)
		, b(0)
		, c(0)
	{}

	FMultiTask2Delaunay2DTriangle(const int32& v1, const int32& v2, const int32& v3)
		: a(v1)
		, b(v2)
		, c(v3)
	{

	}

	bool ContainsVertex(const TArray<FVector2D>& Vertices, const int32& v) const;
	bool CircumCircleContains(const TArray<FVector2D>& Vertices, const int32& v) const;

};


UCLASS(HideDropdown, NotBlueprintable, NotBlueprintType, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UDelaunayTriangulation2DTask : public UMultiThreadTask
{
	friend class FDelaunayTriangulation2DTaskAction;
	GENERATED_BODY()

public:

	virtual bool Start() override;

	/**
	* Called on Background Thread when the Task is executed.
	*/
	virtual void TaskBody_Implementation() override;

private:

	TArray<FVector2D> Vertices;
	TArray<FMultiTask2Delaunay2DTriangle> Triangles;
};

class MULTITASK2_API FDelaunayTriangulation2DTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	TArray<FMultiTask2Delaunay2DTriangle>& Triangles;
	bool bStarted;
public:
	FDelaunayTriangulation2DTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, TArray<FVector2D>& Vertices, TArray<FMultiTask2Delaunay2DTriangle>& OutTriangles, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, UDelaunayTriangulation2DTask::StaticClass())
		, Branches(InBranches)
		, Triangles(OutTriangles)
		, bStarted(false)
	{
		OutTask = Task;

		UDelaunayTriangulation2DTask* LocalTask = Cast<UDelaunayTriangulation2DTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();
			if (Vertices.Num() > 0)
			{
				LocalTask->Vertices = Vertices;
			}
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
					UDelaunayTriangulation2DTask* LocalTask = Cast<UDelaunayTriangulation2DTask>(Task);
					if (LocalTask)
					{
						Triangles = MoveTemp(LocalTask->Triangles);
						LocalTask->Vertices.Empty();
						LocalTask->Triangles.Empty();
					}
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				UDelaunayTriangulation2DTask* LocalTask = Cast<UDelaunayTriangulation2DTask>(Task);
				if (LocalTask)
				{
					LocalTask->Vertices.Empty();
					LocalTask->Triangles.Empty();
				}
				Branches = EMultiTask2Branches::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			UDelaunayTriangulation2DTask* LocalTask = Cast<UDelaunayTriangulation2DTask>(Task);
			if (LocalTask)
			{
				LocalTask->Vertices.Empty();
				LocalTask->Triangles.Empty();
			}
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2Branches::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};