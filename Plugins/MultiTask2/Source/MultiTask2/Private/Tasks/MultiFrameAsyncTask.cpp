// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiFrameAsyncTask.h"

UMultiFrameAsyncTask::UMultiFrameAsyncTask()
{
	bIsTickable = true;
}

UMultiFrameAsyncTask::~UMultiFrameAsyncTask()
{
}

bool UMultiFrameAsyncTask::Start()
{
	if (IterationsPerTick >= 1 && Delay >= 0.0f)
	{
		TimeRemaining = 0.0f;
		bStarted = true;
		return true;
	}
	return false;
}

void UMultiFrameAsyncTask::TaskBody_Implementation()
{
}

bool UMultiFrameAsyncTask::IsRunning()
{
	if (!IsCanceled() && bStarted)
	{
		return true;
	}
	return false;
}

void UMultiFrameAsyncTask::Tick(float DeltaTime)
{
	if (bStarted && !IsCanceled())
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = Delay;
			for (int32 CurrentIt = 0; CurrentIt < IterationsPerTick; CurrentIt++)
			{
				if (IsCanceled())
				{
					break;
				}
				else {
					TaskBody();
					if (TaskDelegate.IsBound())
					{
						TaskDelegate.Broadcast();
					}
				}
			}
		}
	}
	UMultiTaskBase::Tick(DeltaTime);
}
