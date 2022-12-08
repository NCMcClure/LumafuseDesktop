// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiTaskThreadPool.h"

bool UMultiTaskThreadPool::Create(uint32 InNumQueuedThreads, uint32 StackSize, EThreadPriority ThreadPriority, const FString Name)
{
	Obj = MakeShareable<FQueuedThreadPool>(FQueuedThreadPool::Allocate());
	const bool bResult = Obj->Create(InNumQueuedThreads, StackSize, ThreadPriority, *Name);
	if (bResult)
	{
		return true;
	}
	else {
		Obj = nullptr;
		return false;
	}
}

int32 UMultiTaskThreadPool::GetThreadsNum()
{
	if (Obj.IsValid())
	{
		return Obj->GetNumThreads();
	}
	return 0;
}
