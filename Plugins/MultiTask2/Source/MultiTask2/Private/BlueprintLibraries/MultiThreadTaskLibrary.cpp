// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiThreadTaskLibrary.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "RHIResources.h"
#include "TextureResource.h"
#include "UObject/Script.h"
#include "Misc/CoreMisc.h"
#include "MultiTaskMutex.h"
#include "SpawnInstancesTask.h"
#include "UpdateInstancesTask.h"
#include "MultiTask2UtilitiesLibrary.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "CoreGlobals.h"
#include "Components/StaticMeshComponent.h"

void UMultiThreadTaskLibrary::DoSingleThreadTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiThreadTask> Class, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FSingleThreadTaskAction* Action = LatentActionManager.FindExistingAction<FSingleThreadTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FSingleThreadTaskAction(WorldContextObject, Out, LatentInfo, Class, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoSingleThreadTask2(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UMultiTaskBase*& Task, EMultiTask2BranchesWithBody& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask2: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask2: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FSingleThreadTaskWithBodyAction* Action = LatentActionManager.FindExistingAction<FSingleThreadTaskWithBodyAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoSingleThreadTask2: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FSingleThreadTaskWithBodyAction(WorldContextObject, Out, LatentInfo, UMultiThreadTask::StaticClass(), ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoMultiThreadTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiThreadTask> Class, int32 Count, TArray<UMultiTaskBase*>& Tasks, EMultiTask2BranchesNoCancel& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{

	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoMultiThreadTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoMultiThreadTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoMultiThreadTask: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (Count <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoMultiThreadTask: Count has to be >= 1."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FMultiThreadTaskAction* Action = LatentActionManager.FindExistingAction<FMultiThreadTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoMultiThreadTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FMultiThreadTaskAction(WorldContextObject, Out, LatentInfo, Class, Count, ExecutionType, ThreadPool, Tasks);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoSpawnInstances(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, const TArray<FTransform>& Transforms, int32 Chunks, bool bWorldSpace, bool bCreatePhysicsBodies, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, TArray<int32>& NewInstances, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSpawnInstances: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (Transforms.Num() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSpawnInstances: Nothing to spawn. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (Chunks < 1)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSpawnInstances: Chunks needs to be >= 1."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSpawnInstances: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (nullptr == HISM)
	{
		FFrame::KismetExecutionMessage(TEXT("DoSpawnInstances: Invalid HISM Component. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FSpawnInstancesTaskAction* Action = LatentActionManager.FindExistingAction<FSpawnInstancesTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoSpawnInstances: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FSpawnInstancesTaskAction(WorldContextObject, Out, LatentInfo, Chunks, HISM, Transforms, bWorldSpace, bCreatePhysicsBodies, bCreateInternalDataCopies, ExecutionType, ThreadPool, Task, NewInstances);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoUpdateInstances(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<float>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 MaxCount = FMath::Max(Transforms.Num(), CustomData.Num());

	if (MaxCount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Nothing to update."), ELogVerbosity::Error);
		return;
	}

	if (!(StartIndex < HISM->PerInstanceSMData.Num() && StartIndex >= 0))
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid StartIndex."), ELogVerbosity::Error);
		return;
	}

	if (MaxCount < Chunks)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Not enough Data for the selected amount of chunks. Transforms.Num() or CustomData.Num() must be >= Chunks."), ELogVerbosity::Error);
		return;
	}


	if (Chunks < 1)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Chunks needs to be >= 1."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (nullptr == HISM)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid HISM Component. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 CustomDataSize = 1;
	EMultiTaskCustomDataType DataSizeType = EMultiTaskCustomDataType::One;


	if (CustomData.Num() > 0)
	{
		if (CustomDataSize != HISM->NumCustomDataFloats)
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Custom Data type."), ELogVerbosity::Error);
			return;
		}
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FUpdateInstancesTaskAction* Action = LatentActionManager.FindExistingAction<FUpdateInstancesTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FUpdateInstancesTaskAction(WorldContextObject, Out, LatentInfo, Chunks, HISM, StartIndex, Transforms, CustomData.GetData(), CustomData.Num(), DataSizeType, bWorldSpace, bTeleport, bUpdatePhysicsBodies, bMarkRenderStateDirty, bCreateInternalDataCopies, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoUpdateInstances2(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<FVector2D>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 MaxCount = FMath::Max(Transforms.Num(), CustomData.Num());

	if (MaxCount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Nothing to update."), ELogVerbosity::Error);
		return;
	}

	if (!(StartIndex < HISM->PerInstanceSMData.Num() && StartIndex >= 0))
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid StartIndex."), ELogVerbosity::Error);
		return;
	}

	if (MaxCount < Chunks)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Not enough Data for the selected amount of chunks. Transforms.Num() or CustomData.Num() must be >= Chunks."), ELogVerbosity::Error);
		return;
	}


	if (Chunks < 1)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Chunks needs to be >= 1."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (nullptr == HISM)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid HISM Component. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 CustomDataSize = 2;
	EMultiTaskCustomDataType DataSizeType = EMultiTaskCustomDataType::Two;


	if (CustomData.Num() > 0)
	{
		if (CustomDataSize != HISM->NumCustomDataFloats)
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Custom Data type."), ELogVerbosity::Error);
			return;
		}
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FUpdateInstancesTaskAction* Action = LatentActionManager.FindExistingAction<FUpdateInstancesTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FUpdateInstancesTaskAction(WorldContextObject, Out, LatentInfo, Chunks, HISM, StartIndex, Transforms, CustomData.GetData(), CustomData.Num(), DataSizeType, bWorldSpace, bTeleport, bUpdatePhysicsBodies, bMarkRenderStateDirty, bCreateInternalDataCopies, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoUpdateInstances3(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<FVector>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 MaxCount = FMath::Max(Transforms.Num(), CustomData.Num());

	if (MaxCount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Nothing to update."), ELogVerbosity::Error);
		return;
	}

	if (!(StartIndex < HISM->PerInstanceSMData.Num() && StartIndex >= 0))
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid StartIndex."), ELogVerbosity::Error);
		return;
	}

	if (MaxCount < Chunks)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Not enough Data for the selected amount of chunks. Transforms.Num() or CustomData.Num() must be >= Chunks."), ELogVerbosity::Error);
		return;
	}


	if (Chunks < 1)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Chunks needs to be >= 1."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (nullptr == HISM)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid HISM Component. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 CustomDataSize = 3;
	EMultiTaskCustomDataType DataSizeType = EMultiTaskCustomDataType::Three;


	if (CustomData.Num() > 0)
	{
		if (CustomDataSize != HISM->NumCustomDataFloats)
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Custom Data type."), ELogVerbosity::Error);
			return;
		}
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FUpdateInstancesTaskAction* Action = LatentActionManager.FindExistingAction<FUpdateInstancesTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FUpdateInstancesTaskAction(WorldContextObject, Out, LatentInfo, Chunks, HISM, StartIndex, Transforms, CustomData.GetData(), CustomData.Num(), DataSizeType, bWorldSpace, bTeleport, bUpdatePhysicsBodies, bMarkRenderStateDirty, bCreateInternalDataCopies, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoUpdateInstances4(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<FVector4>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 MaxCount = FMath::Max(Transforms.Num(), CustomData.Num());

	if (MaxCount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Nothing to update."), ELogVerbosity::Error);
		return;
	}

	if (!(StartIndex < HISM->PerInstanceSMData.Num() && StartIndex >= 0))
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid StartIndex."), ELogVerbosity::Error);
		return;
	}

	if (MaxCount < Chunks)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Not enough Data for the selected amount of chunks. Transforms.Num() or CustomData.Num() must be >= Chunks."), ELogVerbosity::Error);
		return;
	}


	if (Chunks < 1)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Chunks needs to be >= 1."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (nullptr == HISM)
	{
		FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid HISM Component. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 CustomDataSize = 4;
	EMultiTaskCustomDataType DataSizeType = EMultiTaskCustomDataType::Four;


	if (CustomData.Num() > 0)
	{
		if (CustomDataSize != HISM->NumCustomDataFloats)
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: Invalid Custom Data type."), ELogVerbosity::Error);
			return;
		}
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FUpdateInstancesTaskAction* Action = LatentActionManager.FindExistingAction<FUpdateInstancesTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoUpdateInstances: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FUpdateInstancesTaskAction(WorldContextObject, Out, LatentInfo, Chunks, HISM, StartIndex, Transforms, CustomData.GetData(), CustomData.Num(), DataSizeType, bWorldSpace, bTeleport, bUpdatePhysicsBodies, bMarkRenderStateDirty, bCreateInternalDataCopies, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiThreadTaskLibrary::DoDelaunayTriangulation2D(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UPARAM(ref)TArray<FVector2D>& Points, UMultiTaskBase*& Task, TArray<FMultiTask2Delaunay2DTriangle>& Triangles, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoDelaunayTriangulation2D: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (Points.Num() < 3)
	{
		FFrame::KismetExecutionMessage(TEXT("DoDelaunayTriangulation2D: Nothing to triangulate or not enough data. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FDelaunayTriangulation2DTaskAction* Action = LatentActionManager.FindExistingAction<FDelaunayTriangulation2DTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoDelaunayTriangulation2D: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FDelaunayTriangulation2DTaskAction(WorldContextObject, Out, LatentInfo, Points, Triangles, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

UMultiTaskThreadPool* UMultiThreadTaskLibrary::CreateThreadPool(UObject* WorldContextObject, int32 NumQueuedThreads, int32 StackSize, EMultiTaskThreadPriority ThreadPriority, FString Name)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("CreateThreadPool: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return nullptr;
	}

	if (NumQueuedThreads <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("CreateThreadPool: NumQueuedThreads must be >= 1."), ELogVerbosity::Error);
		return nullptr;
	}

	if (NumQueuedThreads <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("CreateThreadPool: StackSize must be >= 1."), ELogVerbosity::Error);
		return nullptr;
	}

	UMultiTaskThreadPool* ThreadPool;

	UMultiTask2UtilitiesLibrary::ThreadPoolIndex++;
	const FString PoolName = "MultiTaskThreadPool" + FString::FromInt(UMultiTask2UtilitiesLibrary::ThreadPoolIndex);

	ThreadPool = NewObject<UMultiTaskThreadPool>(WorldContextObject, FName(*PoolName), RF_Transient);
	if (ThreadPool)
	{
		EThreadPriority LocalThreadPriority = EThreadPriority::TPri_Normal;
		switch (ThreadPriority)
		{
		case EMultiTaskThreadPriority::AboveNormal:
			LocalThreadPriority = EThreadPriority::TPri_AboveNormal;
			break;
		case EMultiTaskThreadPriority::BelowNormal:
			LocalThreadPriority = EThreadPriority::TPri_BelowNormal;
			break;
		case EMultiTaskThreadPriority::Highest:
			LocalThreadPriority = EThreadPriority::TPri_Highest;
			break;
		case EMultiTaskThreadPriority::Lowest:
			LocalThreadPriority = EThreadPriority::TPri_Lowest;
			break;
		case EMultiTaskThreadPriority::Normal:
			LocalThreadPriority = EThreadPriority::TPri_Normal;
			break;
		case EMultiTaskThreadPriority::SlightlyBelowNormal:
			LocalThreadPriority = EThreadPriority::TPri_SlightlyBelowNormal;
			break;
		case EMultiTaskThreadPriority::TimeCritical:
			LocalThreadPriority = EThreadPriority::TPri_TimeCritical;
			break;
		default:
			LocalThreadPriority = EThreadPriority::TPri_Normal;
		}

		const bool bResult = ThreadPool->Create((uint32)NumQueuedThreads, (uint32)StackSize, LocalThreadPriority, Name);

		if (bResult)
		{
			return ThreadPool;
		}
		else {
			FFrame::KismetExecutionMessage(TEXT("CreateThreadPool: Thread Pool could not be created."), ELogVerbosity::Error);
		}
	}
	if (ThreadPool)
	{
		ThreadPool->ConditionalBeginDestroy();
	}
	return nullptr;
}

void UMultiThreadTaskLibrary::DestroyThreadPoolImmediately(UMultiTaskThreadPool* ThreadPool)
{
	UMultiTask2UtilitiesLibrary::RemoveFromRoot(ThreadPool);
	if (ThreadPool->Obj.IsValid())
	{
		ThreadPool->Obj.Reset();
	}
}

UMultiTaskMutex* UMultiThreadTaskLibrary::CreateMutex(UObject* WorldContextObject)
{
	UMultiTask2UtilitiesLibrary::MutexIndex++;
	const FString Name = "MultiTaskMutex" + FString::FromInt(UMultiTask2UtilitiesLibrary::MutexIndex);
	return NewObject<UMultiTaskMutex>(WorldContextObject, FName(*Name), RF_Transient);
}

void UMultiThreadTaskLibrary::Sleep(float Seconds)
{
	if (!IsInGameThread())
	{
		FPlatformProcess::Sleep(Seconds);
	}
}

int32 UMultiThreadTaskLibrary::GetNumberOfCores()
{
	return FPlatformMisc::NumberOfCores();
}

int32 UMultiThreadTaskLibrary::GetNumberOfCoresIncludingHyperthreads()
{
	return FPlatformMisc::NumberOfCoresIncludingHyperthreads();
}

bool UMultiThreadTaskLibrary::IsInGameThread()
{
	if (GIsGameThreadIdInitialized)
	{
		const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		return CurrentThreadId == GGameThreadId;
	}

	return true;
}