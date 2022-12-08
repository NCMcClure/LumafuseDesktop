// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "SpawnInstancesTask.h"
#include "MultiTaskThreadPool.h"
#include "MultiThreadTaskLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

bool USpawnInstancesTask::Start()
{
    if (IsRunning())
    {
        return false;
    }

    if (TransformArraySize <= 0)
    {
        return false;
    }

    if (!(HISM != nullptr && HISM->IsValidLowLevel() && !HISM->IsPendingKillOrUnreachable()))
    {
        return false;
    }

    if (!HISM->GetStaticMesh())
    {
        return false;
    }

    if (TaskCount <= 0)
    {
        return false;
    }

    bCanceled = false;

    EAsyncExecution AsyncType = EAsyncExecution::ThreadPool;
    switch (ExecutionType)
    {
    case ETaskExecutionType::TaskGraph:
        AsyncType = EAsyncExecution::TaskGraph;
        break;
    case ETaskExecutionType::Thread:
        AsyncType = EAsyncExecution::Thread;
        break;
    case ETaskExecutionType::ThreadPool:
        AsyncType = EAsyncExecution::ThreadPool;
        break;
    }

    PerInstanceSMData.SetNumZeroed(TransformArraySize);
    InstanceBodies.SetNumZeroed(TransformArraySize);
    InstanceReorderTable.SetNumZeroed(TransformArraySize);
    UnbuiltInstanceBoundsList.SetNumZeroed(TransformArraySize);
    Cmds.SetNumZeroed(TransformArraySize);
    NewInstances->SetNumZeroed(TransformArraySize);
    UnbuiltInstanceBounds.Init();

    HISMTransform = HISM->GetComponentTransform();
    MeshBounds = HISM->GetStaticMesh()->GetBounds().GetBox();

    USpawnInstancesTask* Worker = this;

    const int32 ChunkSize = FMath::FloorToInt(((float)TransformArraySize / (float)TaskCount));
    const int32 LastChunkSize = TransformArraySize - (ChunkSize * TaskCount);
    const int32 Chunks = LastChunkSize > 0 ? TaskCount + 1 : TaskCount;

    Tasks.SetNumZeroed(Chunks);

    for (int32 ChunkIndex = 0; ChunkIndex < Chunks; ++ChunkIndex)
    {
        const int32 IterationSize = ((LastChunkSize > 0) && (ChunkIndex == (Chunks - 1))) ? LastChunkSize : ChunkSize;

        TFunction<void()> BodyFunc = [Worker, IterationSize, ChunkIndex, ChunkSize]()
        {
            if (!(Worker != nullptr && Worker->IsValidLowLevel() && !Worker->IsPendingKillOrUnreachable() && !Worker->IsCanceled()))
            {
                return;
            }
            Worker->TaskBody(IterationSize, ChunkIndex, ChunkSize);
        };

        if (AsyncType == EAsyncExecution::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() > 0)
        {
            Tasks[ChunkIndex] = AsyncPool(ThreadPool->Obj.ToSharedRef().Get(), TUniqueFunction<void()>(BodyFunc));
        }
        else {
            Tasks[ChunkIndex] = Async(AsyncType, TUniqueFunction<void()>(BodyFunc));
        }

    }

    return true;
}

void USpawnInstancesTask::TaskBody(int32 IterationSize, int32 ChunkIndex, int32 ChunkSize)
{
    if (HISM->GetStaticMesh() && HISM->GetStaticMesh()->HasValidRenderData())
    {
        const int32 BaseIndex = HISM->PerInstanceSMData.Num();
        const int32 InitialBufferOffset = HISM->InstanceCountToRender - HISM->InstanceReorderTable.Num();

        for (int32 X = 0; X < IterationSize; ++X)
        {
            if (!(HISM != nullptr && HISM->IsValidLowLevel() && !HISM->IsPendingKillOrUnreachable() && !IsCanceled()))
            {
                return;
            }

            const int32 Index = (ChunkIndex * ChunkSize) + X;

            const FTransform& NewLocalTransform = bWorldSpace ? TransformPtr[Index].GetRelativeTransform(HISMTransform) : TransformPtr[Index];
            const FMatrix NewMatrix = NewLocalTransform.ToMatrixWithScale();

            PerInstanceSMData[Index].Transform = NewMatrix;

            const FBox NewInstanceBounds = MeshBounds.TransformBy(NewLocalTransform);
            UnbuiltInstanceBounds += NewInstanceBounds;
            InstanceReorderTable[Index] = InitialBufferOffset + Index;

            UnbuiltInstanceBoundsList[Index] = NewInstanceBounds;

            FInstanceUpdateCmdBuffer::FInstanceUpdateCommand& Cmd = Cmds[Index];
            Cmd.InstanceIndex = INDEX_NONE;
            Cmd.Type = FInstanceUpdateCmdBuffer::Add;
            Cmd.XForm = NewMatrix;

            NewInstances->GetData()[Index] = Index;
        }
    }
}

void USpawnInstancesTask::CreatePhysicsBodies()
{
    check(IsInGameThread());
    //Instance Physics Bodies get created on Game Thread and at this point there's no way to avoid this.
    if (HISM->IsPhysicsStateCreated())
    {
        UBodySetup* BodySetup = HISM->GetBodySetup();
        if (BodySetup)
        {
            const int32 BaseIndex = HISM->PerInstanceSMData.Num();
            for (int32 X = 0; X < InstanceBodies.Num(); ++X)
            {
                const FTransform InstanceTransform = FTransform(PerInstanceSMData[X].Transform) * HISM->GetComponentTransform();

                if (!InstanceTransform.GetScale3D().IsNearlyZero())
                {
#if WITH_PHYSX
                    InstanceBodies[X] = new FBodyInstance();
                    InstanceBodies[X]->CopyBodyInstancePropertiesFrom(&HISM->BodyInstance);
                    InstanceBodies[X]->InstanceBodyIndex = BaseIndex + X;

                    InstanceBodies[X]->bSimulatePhysics = false;


                    // Create physics body instance.
                    InstanceBodies[X]->bAutoWeld = false;	//We don't support this for instanced meshes.

                    InstanceBodies[X]->InitBody(BodySetup, InstanceTransform, HISM, HISM->GetWorld()->GetPhysicsScene(), nullptr);
#endif
                }
            }
        }
    }
    //End of Physics Bodies creation
}