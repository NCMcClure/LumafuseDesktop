// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.

#include "MultiTask2UtilitiesLibrary.h"
#include "Engine/Engine.h"
#include "MultiTaskBase.h"
#if WITH_EDITOR
#include "Editor.h"
#else
#include "Misc/CoreDelegates.h"
#endif
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AI/NavigationSystemBase.h"

#define LOCTEXT_NAMESPACE "UMultiTask2UtilitiesLibrary"

#if WITH_EDITOR
FDelegateHandle UMultiTask2UtilitiesLibrary::EndPIEHandle = FDelegateHandle();
#else
FDelegateHandle UMultiTask2UtilitiesLibrary::PreExitHandle = FDelegateHandle();
#endif
TArray<UObject*> UMultiTask2UtilitiesLibrary::RootObjects = {};
int32 UMultiTask2UtilitiesLibrary::TaskIndex = 0;
int32 UMultiTask2UtilitiesLibrary::MutexIndex = 0;
int32 UMultiTask2UtilitiesLibrary::ThreadPoolIndex = 0;

UMultiTask2UtilitiesLibrary::UMultiTask2UtilitiesLibrary()
{
#if WITH_EDITOR
    EndPIEHandle = FEditorDelegates::PrePIEEnded.AddStatic(&UMultiTask2UtilitiesLibrary::OnEndPIE);
#else
    PreExitHandle = FCoreDelegates::OnPreExit.AddStatic(&UMultiTask2UtilitiesLibrary::OnPreExit);
#endif
}

UMultiTask2UtilitiesLibrary::~UMultiTask2UtilitiesLibrary()
{
#if WITH_EDITOR
    FEditorDelegates::PrePIEEnded.Remove(EndPIEHandle);
#else
    FCoreDelegates::OnPreExit.Remove(PreExitHandle);
#endif
    TaskIndex = 0;
    MutexIndex = 0;
    ThreadPoolIndex = 0;
}



void UMultiTask2UtilitiesLibrary::SetMaximumLoopIterations(const int32 MaximumLoopIterations)
{
#if DO_BLUEPRINT_GUARD
    FBlueprintCoreDelegates::SetScriptMaximumLoopIterations(MaximumLoopIterations);
#endif
}

void UMultiTask2UtilitiesLibrary::ResetRunaway()
{
#if DO_BLUEPRINT_GUARD
    FBlueprintContextTracker& BpET = FBlueprintContextTracker::Get();
    BpET.ResetRunaway();
#endif
}

UObject* UMultiTask2UtilitiesLibrary::GetContextWorld(UObject* WorldContextObject)
{
    return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
}


bool UMultiTask2UtilitiesLibrary::UpdateInstancePhysicsBody(UHierarchicalInstancedStaticMeshComponent* HISM, int32 InstanceIndex, bool bTeleport)
{
    if (IsInGameThread())
    {
        if (HISM->IsPhysicsStateCreated())
        {
            UBodySetup* BodySetup = HISM->GetBodySetup();
            if (BodySetup)
            {
                if (!HISM->PerInstanceSMData.IsValidIndex(InstanceIndex) || !HISM->InstanceBodies.IsValidIndex(InstanceIndex))
                {
                    return false;
                }

                const FTransform InstanceTransform = FTransform(HISM->PerInstanceSMData[InstanceIndex].Transform) * HISM->GetComponentTransform();
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
                return true;
#endif //WITH_PHYSX

            }
        }
    }
    return false;
}


void UMultiTask2UtilitiesLibrary::Cancel(UMultiTaskBase* Task)
{
    if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
    {
        Task->Cancel();
    }
}

bool UMultiTask2UtilitiesLibrary::IsRunning(UMultiTaskBase* Task)
{
    if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
    {
        return Task->IsRunning();
    }
    return false;
}

bool UMultiTask2UtilitiesLibrary::IsCanceled(UMultiTaskBase* Task)
{
    if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsPendingKillOrUnreachable())
    {
        return Task->IsCanceled();
    }
    return true;
}

int32 UMultiTask2UtilitiesLibrary::MixThreeIntegers(int32 Integer1, int32 Integer2, int32 Integer3)
{
    unsigned long a = (unsigned long)Integer1;
    unsigned long b = (unsigned long)Integer2;
    unsigned long c = (unsigned long)Integer3;

    a = a - b;  a = a - c;  a = a ^ (c >> 13);
    b = b - c;  b = b - a;  b = b ^ (a << 8);
    c = c - a;  c = c - b;  c = c ^ (b >> 13);
    a = a - b;  a = a - c;  a = a ^ (c >> 12);
    b = b - c;  b = b - a;  b = b ^ (a << 16);
    c = c - a;  c = c - b;  c = c ^ (b >> 5);
    a = a - b;  a = a - c;  a = a ^ (c >> 3);
    b = b - c;  b = b - a;  b = b ^ (a << 10);
    c = c - a;  c = c - b;  c = c ^ (b >> 15);
    return (int32)c;
}

void UMultiTask2UtilitiesLibrary::GetRandomPointInTriangle(const FVector& A, const FVector& B, const FVector& C, const FRandomStream& RandomStream, FVector& OutPoint)
{
    float a = RandomStream.GetFraction();
    float b = RandomStream.GetFraction();

    if (a + b > 1) {
        a = 1 - a;
        b = 1 - b;
    }

    const float c = 1 - a - b;

    OutPoint.X = ((a * A.X) + (b * B.X) + (c * C.X));
    OutPoint.Y = ((a * A.Y) + (b * B.Y) + (c * C.Y));

}

float UMultiTask2UtilitiesLibrary::GetTriangleAreaSize(const FVector& A, const FVector& B, const FVector& C)
{
    const float SizeX = (B - A).Size2D();
    const float SizeY = (C - A).Size2D();
    const float SizeZ = (C - B).Size2D();

    const float s = (SizeX + SizeY + SizeZ) / 2;

    return FMath::Sqrt(s * (s - SizeX) * (s - SizeY) * (s - SizeZ));
}

void UMultiTask2UtilitiesLibrary::GetRandomScale(const FVector& Min, const FVector& Max, EMultiTask2ScaleType Type, const FRandomStream& RandomStream, FVector& Scale)
{

    TFunction<float(float, int32)> Interpolate = [&](float Alpha, int32 Axis)
    {
        return Min[Axis] + (Alpha * (Max[Axis] - Min[Axis]));
    };

    Scale = FVector(1.0f);
    float LockRand = 0.0f;

    switch (Type)
    {
    case EMultiTask2ScaleType::Uniform:
        Scale.X = Interpolate(RandomStream.GetFraction(), 0);
        Scale.Y = Scale.X;
        Scale.Z = Scale.X;
        break;

    case EMultiTask2ScaleType::Free:
        Scale.X = Interpolate(RandomStream.GetFraction(), 0);
        Scale.Y = Interpolate(RandomStream.GetFraction(), 1);
        Scale.Z = Interpolate(RandomStream.GetFraction(), 2);
        break;

    case EMultiTask2ScaleType::LockXY:
        LockRand = RandomStream.GetFraction();
        Scale.X = Interpolate(LockRand, 0);
        Scale.Y = Interpolate(LockRand, 1);
        Scale.Z = Interpolate(RandomStream.GetFraction(), 2);
        break;

    case EMultiTask2ScaleType::LockXZ:
        LockRand = RandomStream.GetFraction();
        Scale.X = Interpolate(LockRand, 0);
        Scale.Y = Interpolate(RandomStream.GetFraction(), 1);
        Scale.Z = Interpolate(LockRand, 2);
        break;

    case EMultiTask2ScaleType::LockYZ:
        LockRand = RandomStream.GetFraction();
        Scale.X = Interpolate(RandomStream.GetFraction(), 0);
        Scale.Y = Interpolate(LockRand, 1);
        Scale.Z = Interpolate(LockRand, 2);
        break;
    }
}

bool UMultiTask2UtilitiesLibrary::SegmentTriangleIntersection(const FVector& StartPoint, const FVector& EndPoint, const FVector& A, const FVector& B, const FVector& C, FVector& OutIntersectPoint, FVector& OutTriangleNormal)
{
    return FMath::SegmentTriangleIntersection(StartPoint, EndPoint, A, B, C, OutIntersectPoint, OutTriangleNormal);
}

void UMultiTask2UtilitiesLibrary::UpdateNavigationData(UActorComponent* Component)
{
    if (Component)
    {
        if (Component->CanEverAffectNavigation() && Component->IsRegistered() && Component->GetWorld() && Component->GetWorld()->GetNavigationSystem() && FNavigationSystem::WantsComponentChangeNotifies())
        {
            Component->bNavigationRelevant = Component->IsNavigationRelevant();
            FNavigationSystem::UpdateComponentData(*Component);
        }
    }
}

void UMultiTask2UtilitiesLibrary::AddToRoot(UObject* Object)
{
    if (Object)
    {
        if (!Object->IsRooted())
        {
            if (!RootObjects.Contains(Object))
            {
                RootObjects.Add(Object);
                Object->AddToRoot();
            }
        }
    }
}

void UMultiTask2UtilitiesLibrary::RemoveFromRoot(UObject* Object)
{
    if (Object)
    {
        if (Object->IsRooted())
        {
            if (RootObjects.Contains(Object))
            {
                RootObjects.Remove(Object);
                Object->RemoveFromRoot();
            }
        }
    }
}

void UMultiTask2UtilitiesLibrary::OnEndPIE(const bool bIsSimulating)
{
    for (auto Object : RootObjects)
    {
        if (Object != nullptr && Object->IsValidLowLevel() && !Object->IsPendingKillOrUnreachable())
        {
            if (Object->IsRooted())
            {
                if (RootObjects.Contains(Object))
                {
                    Object->RemoveFromRoot();
                }
            }
        }
    }
    RootObjects.Empty();
}

void UMultiTask2UtilitiesLibrary::OnPreExit()
{
    for (auto Object : RootObjects)
    {
        if (Object)
        {
            if (Object->IsRooted())
            {
                if (RootObjects.Contains(Object))
                {
                    Object->RemoveFromRoot();
                }
            }
        }
    }
    RootObjects.Empty();
}


#undef LOCTEXT_NAMESPACE
