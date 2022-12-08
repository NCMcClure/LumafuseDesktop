// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiThreadTask.h"
#include "MultiTaskThreadPool.h"

bool UMultiThreadTask::Start()
{
    if (IsRunning())
    {
        return false;
    }

    bCanceled = false;

    EAsyncExecution AsyncType = EAsyncExecution::ThreadPool;
    switch (ExecutionType)
    {
    case ETaskExecutionType::TaskGraph:
        AsyncType = EAsyncExecution::TaskGraph;
        break;
    case ETaskExecutionType::Thread:
        AsyncType = EAsyncExecution::Thread;
        break;
    case ETaskExecutionType::ThreadPool:
        AsyncType = EAsyncExecution::ThreadPool;
        break;
    }

    UMultiThreadTask* Worker = this;

    TFunction<void()> BodyFunc = [Worker]()
    {
        if (Worker != nullptr && Worker->IsValidLowLevel() && !Worker->IsPendingKillOrUnreachable())
        {
            Worker->TaskBody();
            if (Worker->TaskDelegate.IsBound())
            {
                Worker->TaskDelegate.Broadcast();
            }
        }
    };

    TFunction<void()> OnCompleteFunc = [Worker]()
    {
        AsyncTask(ENamedThreads::GameThread, [Worker]()
        {
            if (Worker != nullptr && Worker->IsValidLowLevel() && !Worker->IsPendingKillOrUnreachable())
            {
                if (!Worker->IsCanceled())
                {
                    Worker->OnComplete();
                }
            }
        });
    };

    Tasks.SetNumZeroed(1);
    if (AsyncType == EAsyncExecution::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() > 0)
    {
        Tasks[0] = AsyncPool(ThreadPool->Obj.ToSharedRef().Get(), TUniqueFunction<void()>(BodyFunc), TUniqueFunction<void()>(OnCompleteFunc));
    }
    else {
        Tasks[0] = Async(AsyncType, TUniqueFunction<void()>(BodyFunc), TUniqueFunction<void()>(OnCompleteFunc));
    }


    return true;
}

void UMultiThreadTask::TaskBody_Implementation()
{
}
