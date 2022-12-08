// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiTask2TraceLibrary.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "UMultiTask2TraceLibrary"

FCollisionQueryParams ConfigureCollisionParam(const FName TraceTag, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore)
{
    FCollisionQueryParams Params(TraceTag, SCENE_QUERY_STAT_ONLY(KismetTraceUtils), bTraceComplex);
    Params.bReturnPhysicalMaterial = true;
    Params.bReturnFaceIndex = !UPhysicsSettings::Get()->bSuppressFaceRemapTable;
    // Ask for face index, as long as we didn't disable globally
    Params.AddIgnoredActors(ActorsToIgnore);
    return Params;
}

FCollisionObjectQueryParams ConfigureCollisionObjectParam(const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes)
{
    TArray<TEnumAsByte<ECollisionChannel>> CollisionObjectTraces;
    CollisionObjectTraces.AddUninitialized(ObjectTypes.Num());

    for (auto Iter = ObjectTypes.CreateConstIterator(); Iter; ++Iter)
    {
        CollisionObjectTraces[Iter.GetIndex()] = UEngineTypes::ConvertToCollisionChannel(*Iter);
    }

    FCollisionObjectQueryParams ObjectParams;
    for (auto Iter = CollisionObjectTraces.CreateConstIterator(); Iter; ++Iter)
    {
        const ECollisionChannel& Channel = (*Iter);
        if (FCollisionObjectQueryParams::IsValidObjectQuery(Channel))
        {
            ObjectParams.AddObjectTypesToQuery(Channel);
        }
        else
        {
            UE_LOG(LogBlueprintUserMessages, Warning, TEXT("%d isn't valid object type"), (int32)Channel);
        }
    }

    return ObjectParams;
}


bool UMultiTask2TraceLibrary::LineTraceSingleThreadSafe(UObject* World, const FVector Start,
    const FVector End, ETraceTypeQuery TraceChannel,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    static const FName LineTraceSingleName(TEXT("LineTraceSingleThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(LineTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld ? LocalWorld->LineTraceSingleByChannel(OutHit, Start, End, CollisionChannel, Params) : false;
    return bHit;
}

bool UMultiTask2TraceLibrary::LineTraceMultiThreadSafe(UObject* World, const FVector Start,
    const FVector End, ETraceTypeQuery TraceChannel,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    static const FName LineTraceMultiName(TEXT("LineTraceMultiThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(LineTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld ? LocalWorld->LineTraceMultiByChannel(OutHits, Start, End, CollisionChannel, Params) : false;
    return bHit;
}

bool UMultiTask2TraceLibrary::BoxTraceSingleThreadSafe(UObject* World, const FVector Start,
    const FVector End, const FVector HalfSize,
    const FRotator Orientation, ETraceTypeQuery TraceChannel,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName BoxTraceSingleName(TEXT("BoxTraceSingleThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(BoxTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByChannel(OutHit, Start, End, Orientation.Quaternion(),
            UEngineTypes::ConvertToCollisionChannel(TraceChannel),
            FCollisionShape::MakeBox(HalfSize), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::BoxTraceMultiThreadSafe(UObject* World, const FVector Start,
    const FVector End, const FVector HalfSize,
    const FRotator Orientation, ETraceTypeQuery TraceChannel,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName BoxTraceMultiName(TEXT("BoxTraceMultiThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(BoxTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByChannel(OutHits, Start, End, Orientation.Quaternion(),
            UEngineTypes::ConvertToCollisionChannel(TraceChannel),
            FCollisionShape::MakeBox(HalfSize), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::SphereTraceSingleThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, ETraceTypeQuery TraceChannel,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    static const FName SphereTraceSingleName(TEXT("SphereTraceSingleThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(SphereTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, CollisionChannel,
            FCollisionShape::MakeSphere(Radius), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::SphereTraceMultiThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, ETraceTypeQuery TraceChannel,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    static const FName SphereTraceMultiName(TEXT("SphereTraceMultiThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(SphereTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, CollisionChannel,
            FCollisionShape::MakeSphere(Radius), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::CapsuleTraceSingleThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, float HalfHeight,
    ETraceTypeQuery TraceChannel, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit)
{
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    static const FName CapsuleTraceSingleName(TEXT("CapsuleTraceSingleThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(CapsuleTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, CollisionChannel,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params)
        : false;

    return bHit;
}


bool UMultiTask2TraceLibrary::CapsuleTraceMultiThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, float HalfHeight,
    ETraceTypeQuery TraceChannel, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    static const FName CapsuleTraceMultiName(TEXT("CapsuleTraceMultiThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(CapsuleTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, CollisionChannel,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params)
        : false;

    return bHit;
}

/** Object Query functions **/
bool UMultiTask2TraceLibrary::LineTraceSingleForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName LineTraceSingleName(TEXT("LineTraceSingleForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(LineTraceSingleName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld ? LocalWorld->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, Params) : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::LineTraceMultiForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName LineTraceMultiName(TEXT("LineTraceMultiForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(LineTraceMultiName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld ? LocalWorld->LineTraceMultiByObjectType(OutHits, Start, End, ObjectParams, Params) : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::SphereTraceSingleForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName SphereTraceSingleName(TEXT("SphereTraceSingleForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(SphereTraceSingleName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectParams,
            FCollisionShape::MakeSphere(Radius), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::SphereTraceMultiForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName SphereTraceMultiName(TEXT("SphereTraceMultiForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(SphereTraceMultiName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByObjectType(OutHits, Start, End, FQuat::Identity, ObjectParams,
            FCollisionShape::MakeSphere(Radius), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::BoxTraceSingleForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End, const FVector HalfSize,
    const FRotator Orientation,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName BoxTraceSingleName(TEXT("BoxTraceSingleForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(BoxTraceSingleName, bTraceComplex, ActorsToIgnore);

    TArray<TEnumAsByte<ECollisionChannel>> CollisionObjectTraces;
    CollisionObjectTraces.AddUninitialized(ObjectTypes.Num());

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByObjectType(OutHit, Start, End, Orientation.Quaternion(), ObjectParams,
            FCollisionShape::MakeBox(HalfSize), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::BoxTraceMultiForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End, const FVector HalfSize,
    const FRotator Orientation,
    const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
    bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName BoxTraceMultiName(TEXT("BoxTraceMultiForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(BoxTraceMultiName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByObjectType(OutHits, Start, End, Orientation.Quaternion(), ObjectParams,
            FCollisionShape::MakeBox(HalfSize), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::CapsuleTraceSingleForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, float HalfHeight,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName CapsuleTraceSingleName(TEXT("CapsuleTraceSingleForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(CapsuleTraceSingleName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectParams,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::CapsuleTraceMultiForObjectsThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, float HalfHeight,
    const TArray<TEnumAsByte<EObjectTypeQuery>>&
    ObjectTypes, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName CapsuleTraceMultiName(TEXT("CapsuleTraceMultiForObjectsThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(CapsuleTraceMultiName, bTraceComplex, ActorsToIgnore);

    const FCollisionObjectQueryParams ObjectParams = ConfigureCollisionObjectParam(ObjectTypes);
    if (ObjectParams.IsValid() == false)
    {
        UE_LOG(LogBlueprintUserMessages, Warning, TEXT("Invalid object types"));
        return false;
    }

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByObjectType(OutHits, Start, End, FQuat::Identity, ObjectParams,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params)
        : false;

    return bHit;
}


bool UMultiTask2TraceLibrary::LineTraceSingleByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, FName ProfileName,
    bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName LineTraceSingleName(TEXT("LineTraceSingleByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(LineTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld ? LocalWorld->LineTraceSingleByProfile(OutHit, Start, End, ProfileName, Params) : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::LineTraceMultiByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, FName ProfileName, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName LineTraceMultiName(TEXT("LineTraceMultiByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(LineTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld ? LocalWorld->LineTraceMultiByProfile(OutHits, Start, End, ProfileName, Params) : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::BoxTraceSingleByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, const FVector HalfSize,
    const FRotator Orientation, FName ProfileName,
    bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName BoxTraceSingleName(TEXT("BoxTraceSingleByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(BoxTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);

    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByProfile(OutHit, Start, End, Orientation.Quaternion(), ProfileName,
            FCollisionShape::MakeBox(HalfSize), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::BoxTraceMultiByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, const FVector HalfSize,
    const FRotator Orientation, FName ProfileName,
    bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName BoxTraceMultiName(TEXT("BoxTraceMultiByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(BoxTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByProfile(OutHits, Start, End, Orientation.Quaternion(), ProfileName,
            FCollisionShape::MakeBox(HalfSize), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::SphereTraceSingleByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, FName ProfileName,
    bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName SphereTraceSingleName(TEXT("SphereTraceSingleByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(SphereTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByProfile(OutHit, Start, End, FQuat::Identity, ProfileName,
            FCollisionShape::MakeSphere(Radius), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::SphereTraceMultiByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, FName ProfileName,
    bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName SphereTraceMultiName(TEXT("SphereTraceMultiByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(SphereTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByProfile(OutHits, Start, End, FQuat::Identity, ProfileName,
            FCollisionShape::MakeSphere(Radius), Params)
        : false;

    return bHit;
}

bool UMultiTask2TraceLibrary::CapsuleTraceSingleByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, float HalfHeight,
    FName ProfileName, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    FHitResult& OutHit)
{
    static const FName CapsuleTraceSingleName(TEXT("CapsuleTraceSingleByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(CapsuleTraceSingleName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepSingleByProfile(OutHit, Start, End, FQuat::Identity, ProfileName,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params)
        : false;

    return bHit;
}


bool UMultiTask2TraceLibrary::CapsuleTraceMultiByProfileThreadSafe(UObject* World, const FVector Start,
    const FVector End, float Radius, float HalfHeight,
    FName ProfileName, bool bTraceComplex,
    const TArray<AActor*>& ActorsToIgnore,
    TArray<FHitResult>& OutHits)
{
    static const FName CapsuleTraceMultiName(TEXT("CapsuleTraceMultiByProfileThreadSafe"));
    const FCollisionQueryParams Params = ConfigureCollisionParam(CapsuleTraceMultiName, bTraceComplex, ActorsToIgnore);

    UWorld* LocalWorld = Cast<UWorld>(World);
    bool const bHit = LocalWorld
        ? LocalWorld->SweepMultiByProfile(OutHits, Start, End, FQuat::Identity, ProfileName,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params)
        : false;

    return bHit;
}

#undef LOCTEXT_NAMESPACE