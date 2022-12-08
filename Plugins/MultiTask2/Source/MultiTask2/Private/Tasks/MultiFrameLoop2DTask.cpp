// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiFrameLoop2DTask.h"

UMultiFrameLoop2DTask::UMultiFrameLoop2DTask()
{
	bIsTickable = true;
}

UMultiFrameLoop2DTask::~UMultiFrameLoop2DTask()
{
}

bool UMultiFrameLoop2DTask::Start()
{
	if ((XSize * YSize) > 0 && IterationsPerTick >= 1 && Delay >= 0.0f)
	{
		TimeRemaining = 0.0f;
		bStarted = true;
		return true;
	}
	return false;
}

void UMultiFrameLoop2DTask::TaskBody_Implementation(int32 X, int32 Y)
{
}

bool UMultiFrameLoop2DTask::IsRunning()
{
	if (!IsCanceled() && bStarted)
	{
		const int32 Size = XSize * YSize;
		return CurrentIndex < Size;
	}
	return false;
}

void UMultiFrameLoop2DTask::Tick(float DeltaTime)
{
	if (bStarted && !IsCanceled())
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = Delay;
			const int32 Size = XSize * YSize;
			for (int32 CurrentIt = 0; CurrentIndex < Size && CurrentIt < IterationsPerTick; CurrentIt++, CurrentIndex++)
			{
				if (IsCanceled())
				{
					break;
				}
				else {
					const int32 CurrentX = CurrentIndex % XSize;
					const int32 CurrentY = (CurrentIndex / XSize) % YSize;
					TaskBody(CurrentX, CurrentY);
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
