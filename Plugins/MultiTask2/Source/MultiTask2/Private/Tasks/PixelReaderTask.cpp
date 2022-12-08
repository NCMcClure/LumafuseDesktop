// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "PixelReaderTask.h"

#include "Engine/Texture.h"
#include "RHIResources.h"
#include "TextureResource.h"
#include "RHI.h"

#include "RHICommandList.h"
#include "HAL/UnrealMemory.h"
#include "Math/IntRect.h"
#include "ImagePixelData.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
bool UPixelReaderTask::Start()
{

    bCanceled = false;

    UPixelReaderTask* Worker = this;

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
    Tasks[0] = Async(EAsyncExecution::TaskGraphMainThread, TUniqueFunction<void()>(BodyFunc), TUniqueFunction<void()>(OnCompleteFunc));

    return true;
}

void UPixelReaderTask::TaskBody_Implementation()
{
	UPixelReaderTask* Worker = this;
	ENQUEUE_RENDER_COMMAND(ResolvePixelData)([Worker](FRHICommandListImmediate& RHICmdList)
	{
		FTextureResource* TextureResource = Worker->TextureObj->Resource;
		const FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;

		if (!Texture2D)
		{
			Worker->bCompleted = true;
			return;
		}

		EPixelFormat Format = Texture2D->GetFormat();

		switch (Format)
		{
		case PF_FloatRGBA:
			Worker->PixelData.Type = EPixelDataType::FloatColor;
			break;
		case PF_A32B32G32R32F:
			Worker->PixelData.Type = EPixelDataType::LinearColor;
			break;
		case PF_R8G8B8A8:
			Worker->PixelData.Type = EPixelDataType::ColorRGBA;
			break;
		case PF_B8G8R8A8:
			Worker->PixelData.Type = EPixelDataType::ColorBGRA;
			break;
		default:
			Worker->bCompleted = true;
			return;
		}

		const int32 LocalSizeX = Texture2D->GetSizeX();
		const int32 LocalSizeY = Texture2D->GetSizeY();

		if (LocalSizeX * LocalSizeY <= 0)
		{
			Worker->bCompleted = true;
			return;
		}

		Worker->PixelData.SizeX = LocalSizeX;
		Worker->PixelData.SizeY = LocalSizeY;

		const FIntRect SourceRect(0, 0, Texture2D->GetSizeX(), Texture2D->GetSizeY());
		switch (Texture2D->GetFormat())
		{
		case PF_FloatRGBA:
		{
			TArray<FFloat16Color> RawPixels;
			RawPixels.SetNum(SourceRect.Width() * SourceRect.Height());
			RHICmdList.ReadSurfaceFloatData(Texture2D, SourceRect, RawPixels, (ECubeFace)0, 0, 0);

			TUniquePtr<TImagePixelData<FFloat16Color>> LocalPixelData = MakeUnique<TImagePixelData<FFloat16Color>>(SourceRect.Size(), TArray64<FFloat16Color>(MoveTemp(RawPixels)));

			if (LocalPixelData->IsDataWellFormed())
			{
				Worker->PixelData.Data.SetNumUninitialized(LocalPixelData->Pixels.Num() * sizeof(FFloat16Color));
				FMemory::Memcpy(Worker->PixelData.Data.GetData(), LocalPixelData->Pixels.GetData(), sizeof(FFloat16Color));
			}
			break;
		}

		case PF_A32B32G32R32F:
		{
			FReadSurfaceDataFlags ReadDataFlags(RCM_MinMax);
			ReadDataFlags.SetLinearToGamma(false);


			TArray<FLinearColor> RawPixels;
			RawPixels.SetNum(SourceRect.Width() * SourceRect.Height());
			RHICmdList.ReadSurfaceData(Texture2D, SourceRect, RawPixels, ReadDataFlags);

			TUniquePtr<TImagePixelData<FLinearColor>> LocalPixelData = MakeUnique<TImagePixelData<FLinearColor>>(SourceRect.Size(), TArray64<FLinearColor>(MoveTemp(RawPixels)));

			if (LocalPixelData->IsDataWellFormed())
			{
				Worker->PixelData.Data.SetNumUninitialized(LocalPixelData->Pixels.Num() * sizeof(FLinearColor));
				FMemory::Memcpy(Worker->PixelData.Data.GetData(), LocalPixelData->Pixels.GetData(), sizeof(FLinearColor));
			}
			break;
		}

		case PF_R8G8B8A8:
		case PF_B8G8R8A8:
		{
			FReadSurfaceDataFlags ReadDataFlags;
			ReadDataFlags.SetLinearToGamma(false);

			TArray<FColor> RawPixels;
			RawPixels.SetNum(SourceRect.Width() * SourceRect.Height());
			RHICmdList.ReadSurfaceData(Texture2D, SourceRect, RawPixels, ReadDataFlags);

			TUniquePtr<TImagePixelData<FColor>> LocalPixelData = MakeUnique<TImagePixelData<FColor>>(SourceRect.Size(), TArray64<FColor>(MoveTemp(RawPixels)));


			if (LocalPixelData->IsDataWellFormed())
			{
				Worker->PixelData.Data.SetNumUninitialized(LocalPixelData->Pixels.Num() * sizeof(FColor));
				FMemory::Memcpy(Worker->PixelData.Data.GetData(), LocalPixelData->Pixels.GetData(), sizeof(FColor));
			}
			break;
		}

		default:
			break;
		}

		Worker->bCompleted = true;
	});
}

bool UPixelReaderTask::IsRunning()
{
	return bCompleted ? false : true;
}