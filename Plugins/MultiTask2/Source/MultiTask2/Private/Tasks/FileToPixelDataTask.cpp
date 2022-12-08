// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "FileToPixelDataTask.h"
#include "HAL/UnrealMemory.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Misc/FileHelper.h"
#include "MultiTaskThreadPool.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#include "Modules/ModuleManager.h"
bool UFileToPixelDataTask::Start()
{

    bCanceled = false;

    UFileToPixelDataTask* Worker = this;

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

    TFunction<void()> BodyFunc = [Worker]()
    {
        if (Worker != nullptr && Worker->IsValidLowLevel() && !Worker->IsPendingKillOrUnreachable())
        {
            Worker->TaskBody();
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

void UFileToPixelDataTask::TaskBody_Implementation()
{

    TArray64<uint8> RawFileData;

    if (FFileHelper::LoadFileToArray(RawFileData, *File))
    {
        IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

        EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawFileData.GetData(), RawFileData.Num());

        if (ImageFormat == EImageFormat::Invalid)
        {
            return;
        }

        TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

        if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
        {

            TArray64<uint8>& Array = PixelData.Data;
            
            if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, Array))
            {
                PixelData.SetSize(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
            }
        }
    }
}