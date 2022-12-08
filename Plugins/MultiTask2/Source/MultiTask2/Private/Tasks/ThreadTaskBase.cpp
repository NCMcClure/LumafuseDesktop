// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "ThreadTaskBase.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#if WITH_EDITOR
#include "Editor.h"
#else
#include "Misc/CoreDelegates.h"
#endif


UThreadTaskBase::UThreadTaskBase()
{
#if WITH_EDITOR
    EndPIEHandle = FEditorDelegates::PrePIEEnded.AddUObject(this, &UThreadTaskBase::OnEndPIE);
#else
    PreExitHandle = FCoreDelegates::OnPreExit.AddUObject(this, &UThreadTaskBase::OnPreExit);
#endif
}

UThreadTaskBase::~UThreadTaskBase()
{
#if WITH_EDITOR
    FEditorDelegates::PrePIEEnded.Remove(EndPIEHandle);
#else
    FCoreDelegates::OnPreExit.Remove(PreExitHandle);
#endif
    if (IsRunning())
    {
        WaitToFinish();
    }
}

bool UThreadTaskBase::IsRunning()
{
    if (Tasks.Num() > 0)
    {
        for (auto& Task : Tasks)
        {
            if ((Task.IsValid() && !Task.IsReady()))
            {
                return true;
            }
        }
    }
    return false;
}


void UThreadTaskBase::WaitToFinish()
{
    while (IsRunning());
}

void UThreadTaskBase::OnEndPIE(const bool bIsSimulating)
{
    if (IsRunning())
    {
        Cancel();
    }
}

void UThreadTaskBase::OnPreExit()
{
    if (IsRunning())
    {
        Cancel();
    }
}
