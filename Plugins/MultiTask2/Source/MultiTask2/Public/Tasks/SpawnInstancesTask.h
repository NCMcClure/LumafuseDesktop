// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiThreadTask.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "AI/NavigationSystemBase.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#include "SpawnInstancesTask.generated.h"



UCLASS(HideDropdown, NotBlueprintable, NotBlueprintType, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API USpawnInstancesTask : public UMultiThreadTask
{
	friend class FSpawnInstancesTaskAction;
	GENERATED_BODY()

public:

	virtual bool Start() override;

	/**
	* Called on Background Thread when the Task is executed.
	*/
	virtual void TaskBody(int32 IterationSize, int32 ChunkIndex, int32 ChunkSize);

private:
	void CreatePhysicsBodies();

private:

	TArray<FTransform> InstancesTransforms;
	int32 TransformArraySize = 0;
	FTransform* TransformPtr = NULL;

	UHierarchicalInstancedStaticMeshComponent* HISM = nullptr;
	bool bCreateInternalDataCopies;
	bool bWorldSpace = true;
	int32 TaskCount = 1;

	FTransform HISMTransform;
	FBox MeshBounds;

	TArray<FInstancedStaticMeshInstanceData> PerInstanceSMData;
	TArray<FBodyInstance*> InstanceBodies;
	TArray<int32> InstanceReorderTable;
	TArray<FBox> UnbuiltInstanceBoundsList;
	TArray<FInstanceUpdateCmdBuffer::FInstanceUpdateCommand> Cmds;
	FBox UnbuiltInstanceBounds;
	TArray<int32>* NewInstances;
};


class MULTITASK2_API FSpawnInstancesTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	bool bStarted;
	bool bCreatePhysicsBodies;
public:
	FSpawnInstancesTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, int32 TaskCount, UHierarchicalInstancedStaticMeshComponent* HISM, const TArray<FTransform>& InstancesTransforms, bool bWorldSpace, bool InCreatePhysicsBodies, bool bCreateInternalDataCopies, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask, TArray<int32>& NewInstances)
		: FSingleTaskActionBase(InObject, LatentInfo, USpawnInstancesTask::StaticClass())
		, Branches(InBranches)
		, bStarted(false)
		, bCreatePhysicsBodies(InCreatePhysicsBodies)
	{
		OutTask = Task;

		USpawnInstancesTask* LocalTask = Cast<USpawnInstancesTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();
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
			LocalTask->bCreateInternalDataCopies = bCreateInternalDataCopies;
			NewInstances.Empty();
			LocalTask->HISM = HISM;
			LocalTask->bWorldSpace = bWorldSpace;
			LocalTask->TaskCount = TaskCount;
			LocalTask->ExecutionType = InExecutionType;
			LocalTask->ThreadPool = ThreadPool;
			LocalTask->NewInstances = &NewInstances;
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
					USpawnInstancesTask* LocalTask = Cast<USpawnInstancesTask>(Task);
					if (LocalTask)
					{
						if (LocalTask->HISM)
						{
							LocalTask->Modify();
							if (bCreatePhysicsBodies)
							{
								LocalTask->CreatePhysicsBodies();
							}

							LocalTask->HISM->PerInstanceSMCustomData.AddZeroed(LocalTask->HISM->NumCustomDataFloats * LocalTask->TransformArraySize);
#if WITH_EDITOR
							LocalTask->HISM->SelectedInstances.Add(false, LocalTask->TransformArraySize);
#endif
							LocalTask->HISM->PerInstanceSMData.Append(LocalTask->PerInstanceSMData);
							LocalTask->HISM->InstanceBodies.Append(LocalTask->InstanceBodies);
							LocalTask->HISM->InstanceReorderTable.Append(LocalTask->InstanceReorderTable);
							LocalTask->HISM->UnbuiltInstanceBounds += LocalTask->UnbuiltInstanceBounds;
							LocalTask->HISM->UnbuiltInstanceBoundsList.Append(LocalTask->UnbuiltInstanceBoundsList);
							LocalTask->HISM->InstanceUpdateCmdBuffer.Cmds.Append(LocalTask->Cmds);
							LocalTask->HISM->InstanceUpdateCmdBuffer.NumAdds += LocalTask->Cmds.Num();
							LocalTask->HISM->InstanceUpdateCmdBuffer.NumEdits++;

							if (LocalTask->UnbuiltInstanceBoundsList.Num() > 0)
							{
								LocalTask->HISM->BuildTreeIfOutdated(true, false);
							}
							FNavigationSystem::UpdateComponentData(*LocalTask->HISM);
						}

						if (LocalTask->bCreateInternalDataCopies)
						{
							LocalTask->InstancesTransforms.Empty();
						}
						LocalTask->PerInstanceSMData.Empty();
						LocalTask->InstanceBodies.Empty();
						LocalTask->InstanceReorderTable.Empty();
						LocalTask->UnbuiltInstanceBoundsList.Empty();
						LocalTask->Cmds.Empty();

					}
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				USpawnInstancesTask* LocalTask = Cast<USpawnInstancesTask>(Task);
				if (LocalTask)
				{
					if (LocalTask->bCreateInternalDataCopies)
					{
						LocalTask->InstancesTransforms.Empty();
					}
					LocalTask->PerInstanceSMData.Empty();
					LocalTask->InstanceBodies.Empty();
					LocalTask->InstanceReorderTable.Empty();
					LocalTask->UnbuiltInstanceBoundsList.Empty();
					LocalTask->Cmds.Empty();
				}
				Branches = EMultiTask2Branches::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			USpawnInstancesTask* LocalTask = Cast<USpawnInstancesTask>(Task);
			if (LocalTask)
			{
				if (LocalTask->bCreateInternalDataCopies)
				{
					LocalTask->InstancesTransforms.Empty();
				}
				LocalTask->PerInstanceSMData.Empty();
				LocalTask->InstanceBodies.Empty();
				LocalTask->InstanceReorderTable.Empty();
				LocalTask->UnbuiltInstanceBoundsList.Empty();
				LocalTask->Cmds.Empty();
			}
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2Branches::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};