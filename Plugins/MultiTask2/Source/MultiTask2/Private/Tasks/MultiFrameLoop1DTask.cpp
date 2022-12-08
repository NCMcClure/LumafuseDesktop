// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiFrameLoop1DTask.h"

UMultiFrameLoop1DTask::UMultiFrameLoop1DTask()
{
	bIsTickable = true;
}

UMultiFrameLoop1DTask::~UMultiFrameLoop1DTask()
{
}

bool UMultiFrameLoop1DTask::Start()
{
	if (XSize > 0 && IterationsPerTick >= 1 && Delay >= 0.0f)
	{
		TimeRemaining = 0.0f;
		bStarted = true;
		return true;
	}
	return false;
}

void UMultiFrameLoop1DTask::TaskBody_Implementation(int32 X)
{
}

bool UMultiFrameLoop1DTask::IsRunning()
{
	if (!IsCanceled() && bStarted)
	{
		return CurrentIndex < XSize;
	}
	return false;
}

void UMultiFrameLoop1DTask::Tick(float DeltaTime)
{
	if (bStarted && !IsCanceled())
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = Delay;
			for (int32 CurrentIt = 0; CurrentIndex < XSize && CurrentIt < IterationsPerTick; CurrentIt++, CurrentIndex++)
			{
				if (IsCanceled())
				{
					break;
				}
				else {
					const int32 CurrentX = CurrentIndex % XSize;
					TaskBody(CurrentX);
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
