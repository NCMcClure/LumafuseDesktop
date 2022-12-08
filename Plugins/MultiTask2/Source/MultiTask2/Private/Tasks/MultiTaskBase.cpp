// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiTaskBase.h"
#include "Async/Async.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif


UMultiTaskBase::UMultiTaskBase()
{

}

UMultiTaskBase::~UMultiTaskBase()
{
}

bool UMultiTaskBase::Start()
{
    
    unimplemented();
    return false;
}

void UMultiTaskBase::Cancel()
{
    if (IsRunning() && !IsCanceled())
    {
        bCanceled = true;

        UMultiTaskBase* Worker = this;

        AsyncTask(ENamedThreads::GameThread, [Worker]()
        {
            if (Worker != nullptr && Worker->IsValidLowLevel() && !Worker->IsPendingKillOrUnreachable())
            {
                Worker->OnCancel();
                if (Worker->OnCancelDelegate.IsBound())
                {
                    Worker->OnCancelDelegate.Broadcast();
                }
            }
        });
    }
}

bool UMultiTaskBase::IsRunning()
{
    unimplemented();
    return false;
}

bool UMultiTaskBase::IsCanceled()
{
    return bCanceled;
}

void UMultiTaskBase::OnCancel_Implementation()
{
}

void UMultiTaskBase::OnComplete_Implementation()
{
}

void UMultiTaskBase::OnTick_Implementation(float DeltaTime)
{
}

void UMultiTaskBase::Tick(float DeltaTime)
{
    OnTick(DeltaTime);
}

bool UMultiTaskBase::IsTickable() const
{
    return bIsTickable;
}

bool UMultiTaskBase::IsTickableInEditor() const
{
    return bIsTickableInEditor;
}

bool UMultiTaskBase::IsTickableWhenPaused() const
{
    return bIsTickableWhenPaused;
}

UWorld* UMultiTaskBase::GetTickableGameObjectWorld() const
{
    return GetWorld();
}

UWorld* UMultiTaskBase::GetWorld() const
{
    if (HasAllFlags(RF_ClassDefaultObject))
    {
        return nullptr;
    }

    if (IsValid(GetOuter()))
    {
        return GetOuter()->GetWorld();
    }
    return nullptr;
}

TStatId UMultiTaskBase::GetStatId() const
{
    return TStatId();
}



