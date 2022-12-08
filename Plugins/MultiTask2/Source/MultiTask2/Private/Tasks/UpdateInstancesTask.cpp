// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "UpdateInstancesTask.h"
#include "MultiTaskThreadPool.h"
#include "MultiThreadTaskLibrary.h"
#include "Engine/StaticMesh.h"
#include "Engine/InstancedStaticMesh.h"

bool UUpdateInstancesTask::Start()
{
    if (IsRunning())
    {
        return false;
    }

    if (!(StartIndex < HISM->PerInstanceSMData.Num() && StartIndex >= 0))
    {
        return false;
    }

    if (TransformArraySize > 0)
    {
        const int32 MaxCustomDataSize = HISM->PerInstanceSMData.Num() - StartIndex;
        TransformArraySize = FMath::Clamp(MaxCustomDataSize, 0, TransformArraySize);
        if (bCreateInternalDataCopies)
        {
            InstancesTransforms.SetNum(TransformArraySize);
        }

        PerInstanceSMData.SetNumZeroed(TransformArraySize);
    }


    if (GetCustomDataArraySize() > 0)
    {
        if (GetCustomDataSize(CustomDataType) != HISM->NumCustomDataFloats)
        {
            return false;
        }

        const int32 MaxCustomDataSize = HISM->PerInstanceSMCustomData.Num() - (StartIndex * GetCustomDataSize(CustomDataType));
        CustomDataArraySize = FMath::Clamp(MaxCustomDataSize, 0, CustomDataArraySize);
        if (bCreateInternalDataCopies)
        {
            CustomData.SetNum(CustomDataArraySize);
        }

    }

    const int32 MaxCount = FMath::Max(TransformArraySize, GetCustomDataArraySize());

    if (MaxCount <= 0)
    {
        return false;
    }

    if (MaxCount < TaskCount)
    {
        return false;
    }

    if (TaskCount <= 0)
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

    UnbuiltInstanceBounds.Init();
    BuiltInstanceBounds.Init();

    HISMTransform = HISM->GetComponentTransform();
    MeshBounds = HISM->GetStaticMesh()->GetBounds().GetBox();


    const int32 ChunkSize = FMath::FloorToInt(((float)(MaxCount) / (float)TaskCount));
    const int32 LastChunkSize = MaxCount - (ChunkSize * TaskCount);
    const int32 Chunks = LastChunkSize > 0 ? TaskCount + 1 : TaskCount;

    UUpdateInstancesTask* Worker = this;

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


int32 UUpdateInstancesTask::GetCustomDataArraySize() const
{
    return CustomDataArraySize / GetCustomDataSize(CustomDataType);
}

void UUpdateInstancesTask::TaskBody(int32 IterationSize, int32 ChunkIndex, int32 ChunkSize)
{
    if (HISM->GetStaticMesh())
    {
        TArray<FInstancedStaticMeshInstanceData> LocalPerInstanceSMData;
        TArray<FInstanceUpdateCmdBuffer::FInstanceUpdateCommand> LocalCmds;
        TArray<FBox> LocalUnbuiltInstanceBoundsList;
        FBox LocalUnbuiltInstanceBounds;
        FBox LocalBuiltInstanceBounds;
        LocalUnbuiltInstanceBounds.Init();
        LocalBuiltInstanceBounds.Init();

        const int32 CustomDataArrSize = GetCustomDataArraySize();
        const int32 CustomDataSize = GetCustomDataSize(CustomDataType);


        for (int32 X = 0; X < IterationSize; ++X)
        {
            if (!(HISM != nullptr && HISM->IsValidLowLevel() && !HISM->IsPendingKillOrUnreachable() && !IsCanceled()))
            {
                return;
            }

            const int32 CurrentIndex = (ChunkIndex * ChunkSize) + X;
            const int32 InstanceIndex = StartIndex + CurrentIndex;

            if (!HISM->PerInstanceSMData.IsValidIndex(InstanceIndex))
            {
                break;
            }

            const int32 RenderIndex = HISM->InstanceReorderTable.IsValidIndex(InstanceIndex) ? HISM->InstanceReorderTable[InstanceIndex] : InstanceIndex;
            const bool bIsOmittedInstance = (RenderIndex == INDEX_NONE);

            if (CurrentIndex < TransformArraySize && TransformArraySize > 0)
            {
                const FMatrix OldTransform = HISM->PerInstanceSMData[InstanceIndex].Transform;

                const FTransform& NewLocalTransform = bWorldSpace ? TransformPtr[CurrentIndex].GetRelativeTransform(HISMTransform) : TransformPtr[CurrentIndex];
                const FMatrix NewMatrix = NewLocalTransform.ToMatrixWithScale();

                const FVector NewLocalLocation = NewLocalTransform.GetTranslation();

                // if we are only updating rotation/scale we update the instance directly in the cluster tree

                const bool bIsBuiltInstance = !bIsOmittedInstance && RenderIndex < HISM->NumBuiltRenderInstances;
                const bool bDoInPlaceUpdate = bIsBuiltInstance && NewLocalLocation.Equals(OldTransform.GetOrigin()) && (HISM->PerInstanceRenderData.IsValid() && HISM->PerInstanceRenderData->InstanceBuffer.RequireCPUAccess);
                const FBox NewInstanceBounds = MeshBounds.TransformBy(NewLocalTransform);

                LocalPerInstanceSMData.Add(FInstancedStaticMeshInstanceData(NewMatrix));

                if (!bIsOmittedInstance)
                {
                    FInstanceUpdateCmdBuffer::FInstanceUpdateCommand& Cmd = LocalCmds.AddDefaulted_GetRef();
                    Cmd.InstanceIndex = RenderIndex;
                    Cmd.Type = FInstanceUpdateCmdBuffer::Update;
                    Cmd.XForm = NewMatrix;
                }

                if (bDoInPlaceUpdate)
                {
                    // If the new bounds are larger than the old ones, then expand the bounds on the tree to make sure culling works correctly
                    const FBox OldInstanceBounds = MeshBounds.TransformBy(OldTransform);
                    if (!OldInstanceBounds.IsInside(NewInstanceBounds))
                    {
                        LocalBuiltInstanceBounds += NewInstanceBounds;
                    }
                }
                else
                {
                    LocalUnbuiltInstanceBounds += NewInstanceBounds;
                    LocalUnbuiltInstanceBoundsList.Add(NewInstanceBounds);
                }
            }


            if (CurrentIndex < CustomDataArrSize && CustomDataArrSize > 0)
            {

                if (!bIsOmittedInstance)
                {
                    FInstanceUpdateCmdBuffer::FInstanceUpdateCommand& Cmd = LocalCmds.AddDefaulted_GetRef();
                    Cmd.InstanceIndex = RenderIndex;
                    Cmd.Type = FInstanceUpdateCmdBuffer::CustomData;
                    Cmd.CustomDataFloats = TArray<float>(&CustomDataPtr[CurrentIndex * CustomDataSize], CustomDataSize);
                }
            }
        }

        Mutex.Lock();
        if (LocalPerInstanceSMData.Num() > 0)
        {
            FMemory::Memcpy(&PerInstanceSMData[ChunkIndex * ChunkSize], LocalPerInstanceSMData.GetData(), sizeof(FInstancedStaticMeshInstanceData) * LocalPerInstanceSMData.Num());
            BuiltInstanceBounds += LocalBuiltInstanceBounds;
            UnbuiltInstanceBounds += LocalUnbuiltInstanceBounds;
            UnbuiltInstanceBoundsList.Append(LocalUnbuiltInstanceBoundsList);
        }

        if (LocalCmds.Num() > 0)
        {
            Cmds.Append(LocalCmds);
        }

        Mutex.Unlock();
    }
}

void UUpdateInstancesTask::UpdatePhysicsBodies()
{
    check(IsInGameThread());
    //Instance Physics Bodies get updated on Game Thread and at this point there's no way to avoid this.
    if (HISM->IsPhysicsStateCreated())
    {
        UBodySetup* BodySetup = HISM->GetBodySetup();
        if (BodySetup)
        {
            for (int32 X = 0; X < PerInstanceSMData.Num(); ++X)
            {
                const int32 InstanceIndex = StartIndex + X;

                if (!HISM->InstanceBodies.IsValidIndex(InstanceIndex))
                {
                    break;
                }

                const FTransform InstanceTransform = FTransform(PerInstanceSMData[X].Transform) * HISM->GetComponentTransform();
#if WITH_PHYSX

                if (InstanceTransform.GetScale3D().IsNearlyZero())
                {
                    if (HISM->InstanceBodies[InstanceIndex])
                    {
                        // delete BodyInstance
                        HISM->InstanceBodies[InstanceIndex]->TermBody();
                        delete HISM->InstanceBodies[InstanceIndex];
                        HISM->InstanceBodies[InstanceIndex] = nullptr;
                    }
                }
                else
                {
                    if (HISM->InstanceBodies[InstanceIndex])
                    {
                        // Update existing BodyInstance
                        HISM->InstanceBodies[InstanceIndex]->SetBodyTransform(InstanceTransform, TeleportFlagToEnum(bTeleport));
                        HISM->InstanceBodies[InstanceIndex]->UpdateBodyScale(InstanceTransform.GetScale3D());
                    }
                    else
                    {
                        // create new BodyInstance
                        HISM->InstanceBodies[InstanceIndex] = new FBodyInstance();
                        HISM->InstanceBodies[InstanceIndex]->CopyBodyInstancePropertiesFrom(&HISM->BodyInstance);
                        HISM->InstanceBodies[InstanceIndex]->InstanceBodyIndex = InstanceIndex;

                        HISM->InstanceBodies[InstanceIndex]->bSimulatePhysics = false;

                        // Create physics body instance.
                        HISM->InstanceBodies[InstanceIndex]->bAutoWeld = false;	//We don't support this for instanced meshes.

                        HISM->InstanceBodies[InstanceIndex]->InitBody(BodySetup, InstanceTransform, HISM, HISM->GetWorld()->GetPhysicsScene(), nullptr);
                    }
                }
#endif //WITH_PHYSX

            }
        }
    }
    //End of Physics Bodies update
}
