// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "UrlToPixelDataTask.h"
#include "HAL/UnrealMemory.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "MultiTaskThreadPool.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#include "Modules/ModuleManager.h"
bool UUrlToPixelDataTask::Start()
{

    bCanceled = false;

    ImageRequest = FHttpModule::Get().CreateRequest();

    UUrlToPixelDataTask* Worker = this;

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

void UUrlToPixelDataTask::Cancel()
{
    if (IsRunning() && !IsCanceled())
    {
        ImageRequest->CancelRequest();
    }
    Super::Cancel();
}

void UUrlToPixelDataTask::TaskBody_Implementation()
{
    UUrlToPixelDataTask* Worker = this;
    auto OnResponseReceived = [Worker](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (Worker != nullptr && Worker->IsValidLowLevel() && !Worker->IsPendingKillOrUnreachable())
        {
            if (bWasSuccessful && Response.IsValid())
            {
                TArray<uint8> RawFileData = Response->GetContent();

                IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
                EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawFileData.GetData(), RawFileData.Num());

                if (ImageFormat == EImageFormat::Invalid)
                {
                    return;
                }

                TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

                if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
                {
                    TArray64<uint8>& Array = Worker->PixelData.Data;
                    if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, Array))
                    {
                        Worker->PixelData.SetSize(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
                    }
                }

            }
            if (Worker->ImageRequest.IsValid())
            {
                Worker->ImageRequest.Reset();
            }

        }
       
    };
    
    ImageRequest->SetVerb("GET");
    ImageRequest->SetURL(URL);

    if (!FMath::IsNearlyZero(Timeout))
    {
        ImageRequest->SetTimeout(Timeout);
    }

    ImageRequest->OnProcessRequestComplete().BindLambda(OnResponseReceived);
    ImageRequest->ProcessRequest();

}

bool UUrlToPixelDataTask::IsRunning()
{
    return ImageRequest.IsValid();
}
