// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiThreadTask.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "AI/NavigationSystemBase.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#include "UpdateInstancesTask.generated.h"

enum class EMultiTaskCustomDataType : uint8
{
	One,
	Two,
	Three,
	Four
};

static int32 GetCustomDataSize(EMultiTaskCustomDataType DataSize)
{
	switch (DataSize)
	{
	case EMultiTaskCustomDataType::One:
		return 1;
	case EMultiTaskCustomDataType::Two:
		return 2;
	case EMultiTaskCustomDataType::Three:
		return 3;
	case EMultiTaskCustomDataType::Four:
		return 4;
	default:
		return 0;
	}
	return 0;
}

UCLASS(HideDropdown, NotBlueprintable, NotBlueprintType, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UUpdateInstancesTask : public UMultiThreadTask
{
	friend class FUpdateInstancesTaskAction;
	GENERATED_BODY()

public:

	virtual bool Start() override;


	int32 GetCustomDataArraySize() const;

	/**
	* Called on Background Thread when the Task is executed.
	*/
	void TaskBody(int32 IterationSize, int32 ChunkIndex, int32 ChunkSize);
private:
	void UpdatePhysicsBodies();
public:

private:
	TArray<FTransform> InstancesTransforms;
	int32 TransformArraySize = 0;
	FTransform* TransformPtr = NULL;

	TArray<float> CustomData;
	EMultiTaskCustomDataType CustomDataType;
	int32 CustomDataArraySize = 0;
	float* CustomDataPtr = NULL;
	bool bCreateInternalDataCopies;

	int32 StartIndex;
	int32 TaskCount;

	UHierarchicalInstancedStaticMeshComponent* HISM = nullptr;
	bool bWorldSpace = true;
	bool bTeleport = true;
	FTransform HISMTransform;

	FBox MeshBounds;
	TArray<FInstancedStaticMeshInstanceData> PerInstanceSMData;
	TArray<FBox> UnbuiltInstanceBoundsList;
	TArray<FInstanceUpdateCmdBuffer::FInstanceUpdateCommand> Cmds;
	FBox UnbuiltInstanceBounds;
	FBox BuiltInstanceBounds;

	FCriticalSection Mutex;
};


class MULTITASK2_API FUpdateInstancesTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	bool bStarted;
	bool bMarkRenderStateDirty;
	bool bUpdatePhysicsBodies;
public:
	FUpdateInstancesTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, int32 TaskCount, UHierarchicalInstancedStaticMeshComponent* HISM, const int32 StartIndex, const TArray<FTransform>& InstancesTransforms, const void* InCustomData, const int32& CustomDataArraySize, const EMultiTaskCustomDataType DataSize, const bool bWorldSpace, const bool bTeleport, const bool InUpdatePhysicsBodies, const bool InMarkRenderStateDirty, bool bCreateInternalDataCopies, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, UUpdateInstancesTask::StaticClass())
		, Branches(InBranches)
		, bStarted(false)
		, bMarkRenderStateDirty(InMarkRenderStateDirty)
		, bUpdatePhysicsBodies(InUpdatePhysicsBodies)
	{
		OutTask = Task;

		UUpdateInstancesTask* LocalTask = Cast<UUpdateInstancesTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();
			LocalTask->HISM = HISM;
			LocalTask->StartIndex = StartIndex;

			LocalTask->bCreateInternalDataCopies = bCreateInternalDataCopies;
			LocalTask->TaskCount = TaskCount;

			if (InstancesTransforms.Num() > 0)
			{
				LocalTask->TransformArraySize = InstancesTransforms.Num();
				if (bCreateInternalDataCopies)
				{
					LocalTask->InstancesTransforms.SetNumZeroed(InstancesTransforms.Num());
					FMemory::Memcpy(LocalTask->InstancesTransforms.GetData(), InstancesTransforms.GetData(), InstancesTransforms.Num() * sizeof(FTransform));
					LocalTask->TransformPtr = LocalTask->InstancesTransforms.GetData();
				}
				else {
					LocalTask->TransformPtr = const_cast<FTransform*>(InstancesTransforms.GetData());
				}
			}


			if (CustomDataArraySize > 0)
			{
				const int32 Size = CustomDataArraySize * GetCustomDataSize(DataSize);
				LocalTask->CustomDataType = DataSize;
				LocalTask->CustomDataArraySize = Size;

				if (bCreateInternalDataCopies)
				{
					LocalTask->CustomData.SetNumZeroed(Size);
					FMemory::Memcpy(LocalTask->CustomData.GetData(), InCustomData, Size * sizeof(float));
					LocalTask->CustomDataPtr = LocalTask->CustomData.GetData();
				}
				else {
					LocalTask->CustomDataPtr = (float*)(const_cast<void*>(InCustomData));
				}
			}

			LocalTask->bWorldSpace = bWorldSpace;
			LocalTask->bTeleport = bTeleport;
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
					UUpdateInstancesTask* LocalTask = Cast<UUpdateInstancesTask>(Task);
					if (LocalTask)
					{
						if (LocalTask->HISM)
						{
							LocalTask->Modify();
							if (LocalTask->PerInstanceSMData.Num() > 0)
							{
								if (bUpdatePhysicsBodies)
								{
									LocalTask->UpdatePhysicsBodies();
								}
								FMemory::Memcpy(&LocalTask->HISM->PerInstanceSMData[LocalTask->StartIndex], LocalTask->PerInstanceSMData.GetData(), sizeof(FInstancedStaticMeshInstanceData) * LocalTask->PerInstanceSMData.Num());
								LocalTask->HISM->BuiltInstanceBounds += LocalTask->BuiltInstanceBounds;
								LocalTask->HISM->UnbuiltInstanceBounds += LocalTask->UnbuiltInstanceBounds;
								LocalTask->HISM->UnbuiltInstanceBoundsList.Append(LocalTask->UnbuiltInstanceBoundsList);

							}


							if (LocalTask->CustomDataArraySize > 0)
							{
								FMemory::Memcpy(&LocalTask->HISM->PerInstanceSMCustomData[LocalTask->StartIndex * GetCustomDataSize(LocalTask->CustomDataType)], LocalTask->CustomDataPtr, LocalTask->CustomDataArraySize * sizeof(float));
							}

							if (LocalTask->Cmds.Num() > 0)
							{
								LocalTask->HISM->InstanceUpdateCmdBuffer.Cmds.Append(LocalTask->Cmds);
								LocalTask->HISM->InstanceUpdateCmdBuffer.NumEdits += LocalTask->Cmds.Num();
							}

							if (LocalTask->UnbuiltInstanceBoundsList.Num() > 0)
							{
								LocalTask->HISM->BuildTreeIfOutdated(true, false);
							}
							else {
								if (bMarkRenderStateDirty)
								{
									LocalTask->HISM->MarkRenderStateDirty();
								}
							}
							FNavigationSystem::UpdateComponentData(*LocalTask->HISM);
						}

						if (LocalTask->bCreateInternalDataCopies)
						{
							LocalTask->InstancesTransforms.Empty();
							LocalTask->CustomData.Empty();
						}

						LocalTask->PerInstanceSMData.Empty();
						LocalTask->UnbuiltInstanceBoundsList.Empty();
						LocalTask->Cmds.Empty();
					}
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				UUpdateInstancesTask* LocalTask = Cast<UUpdateInstancesTask>(Task);
				if (LocalTask)
				{
					if (LocalTask->bCreateInternalDataCopies)
					{
						LocalTask->InstancesTransforms.Empty();
						LocalTask->CustomData.Empty();
					}
					LocalTask->PerInstanceSMData.Empty();
					LocalTask->UnbuiltInstanceBoundsList.Empty();
					LocalTask->Cmds.Empty();
				}
				Branches = EMultiTask2Branches::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			UUpdateInstancesTask* LocalTask = Cast<UUpdateInstancesTask>(Task);
			if (LocalTask)
			{
				if (LocalTask->bCreateInternalDataCopies)
				{
					LocalTask->InstancesTransforms.Empty();
					LocalTask->CustomData.Empty();
				}
				LocalTask->PerInstanceSMData.Empty();
				LocalTask->UnbuiltInstanceBoundsList.Empty();
				LocalTask->Cmds.Empty();
			}
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2Branches::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};