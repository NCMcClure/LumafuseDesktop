// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiTaskMutex.h"

bool UMultiTaskMutex::TryLock()
{
	return Section.TryLock();
}

void UMultiTaskMutex::Lock()
{
	Section.Lock();
}

void UMultiTaskMutex::Unlock()
{
	Section.Unlock();
}
