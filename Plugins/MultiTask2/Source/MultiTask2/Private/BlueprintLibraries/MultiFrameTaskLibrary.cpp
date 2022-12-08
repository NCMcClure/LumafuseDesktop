// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiFrameTaskLibrary.h"
#include "Engine/Engine.h"
#include "MultiFrameAsyncTask.h"
#include "MultiFrameLoop1DTask.h"
#include "MultiFrameLoop2DTask.h"
#include "MultiFrameLoop3DTask.h"

void UMultiFrameTaskLibrary::DoAsyncTask(UObject* WorldContextObject, EMultiTask2BranchesNoCompleteWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameAsyncTask> Class, UMultiTaskBase*& Task, int32 IterationsPerTick, float Delay)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoAsyncTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoAsyncTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (IterationsPerTick <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoAsyncTask: IterationsPerTick must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (Delay < 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("DoAsyncTask: Delay can't be lower than 0. We can't travel back in time, can we?"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FMultiFrameAsyncTaskAction* Action = LatentActionManager.FindExistingAction<FMultiFrameAsyncTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && !Action->IsCanceled())
		{
			FFrame::KismetExecutionMessage(TEXT("DoAsyncTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FMultiFrameAsyncTaskAction(WorldContextObject, Out, LatentInfo, Class, IterationsPerTick, Delay, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiFrameTaskLibrary::DoLoop1DTask(UObject* WorldContextObject, EMultiTask2BranchesWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameLoop1DTask> Class, int32& X, UMultiTaskBase*& Task, int32 XSize, int32 IterationsPerTick, float Delay)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop1DTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop1DTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (XSize <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop1DTask: X Size must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (IterationsPerTick <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop1DTask: IterationsPerTick must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (Delay < 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop1DTask: Delay can't be lower than 0. We can't travel back in time, can we?"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FMultiFrameLoop1DTaskAction* Action = LatentActionManager.FindExistingAction<FMultiFrameLoop1DTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && !Action->IsCanceled())
		{
			FFrame::KismetExecutionMessage(TEXT("DoLoop1DTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FMultiFrameLoop1DTaskAction(WorldContextObject, Out, LatentInfo, Class, X, XSize, IterationsPerTick, Delay, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiFrameTaskLibrary::DoLoop2DTask(UObject* WorldContextObject, EMultiTask2BranchesWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameLoop2DTask> Class, int32& X, int32& Y, UMultiTaskBase*& Task, int32 XSize, int32 YSize, int32 IterationsPerTick, float Delay)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (XSize <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: X Size must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (YSize <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: Y Size must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (IterationsPerTick <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: IterationsPerTick must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (Delay < 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: Delay can't be lower than 0. We can't travel back in time, can we?"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FMultiFrameLoop2DTaskAction* Action = LatentActionManager.FindExistingAction<FMultiFrameLoop2DTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && !Action->IsCanceled())
		{
			FFrame::KismetExecutionMessage(TEXT("DoLoop2DTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FMultiFrameLoop2DTaskAction(WorldContextObject, Out, LatentInfo, Class, X, Y, XSize, YSize, IterationsPerTick, Delay, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiFrameTaskLibrary::DoLoop3DTask(UObject* WorldContextObject, EMultiTask2BranchesWithBody& Out, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiFrameLoop3DTask> Class, int32& X, int32& Y, int32& Z, UMultiTaskBase*& Task, int32 XSize, int32 YSize, int32 ZSize, int32 IterationsPerTick, float Delay)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (XSize <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: X Size must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (YSize <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: Y Size must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (ZSize <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: Z Size must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (IterationsPerTick <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: IterationsPerTick must be higher than 0."), ELogVerbosity::Error);
		return;
	}

	if (Delay < 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: Delay can't be lower than 0. We can't travel back in time, can we?"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FMultiFrameLoop3DTaskAction* Action = LatentActionManager.FindExistingAction<FMultiFrameLoop3DTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && !Action->IsCanceled())
		{
			FFrame::KismetExecutionMessage(TEXT("DoLoop3DTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FMultiFrameLoop3DTaskAction(WorldContextObject, Out, LatentInfo, Class, X, Y, Z, XSize, YSize, ZSize, IterationsPerTick, Delay, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}
