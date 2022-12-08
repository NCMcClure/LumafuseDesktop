// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiFrameLoop3DTask.h"

UMultiFrameLoop3DTask::UMultiFrameLoop3DTask()
{
	bIsTickable = true;
}

UMultiFrameLoop3DTask::~UMultiFrameLoop3DTask()
{
}

bool UMultiFrameLoop3DTask::Start()
{
	if ((XSize * YSize * ZSize) > 0 && IterationsPerTick >= 1 && Delay >= 0.0f)
	{
		TimeRemaining = 0.0f;
		bStarted = true;
		return true;
	}
	return false;
}

void UMultiFrameLoop3DTask::TaskBody_Implementation(int32 X, int32 Y, int32 Z)
{
}

bool UMultiFrameLoop3DTask::IsRunning()
{
	if (!IsCanceled() && bStarted)
	{
		const int32 Size = XSize * YSize * ZSize;
		return CurrentIndex < Size;
	}
	return false;
}

void UMultiFrameLoop3DTask::Tick(float DeltaTime)
{
	if (bStarted && !IsCanceled())
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = Delay;
			const int32 Size = XSize * YSize * ZSize;
			for (int32 CurrentIt = 0; CurrentIndex < Size && CurrentIt < IterationsPerTick; CurrentIt++, CurrentIndex++)
			{
				if (IsCanceled())
				{
					break;
				}
				else {
					const int32 CurrentX = CurrentIndex % XSize;
					const int32 CurrentY = (CurrentIndex / XSize) % YSize;
					const int32 CurrentZ = CurrentIndex / (XSize * YSize);
					TaskBody(CurrentX, CurrentY, CurrentZ);
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
