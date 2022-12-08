// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "PixelReaderLibrary.h"
#include "FileToPixelDataTask.h"
#include "UrlToPixelDataTask.h"
#include "Engine/Texture2D.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Engine/Texture.h"
#include "Engine/Engine.h"

bool UPixelReaderLibrary::GetPixel(const FPixelData& PixelData, const FIntPoint& Coordinates, FColor& Pixel)
{
	return PixelData.GetPixel(Coordinates, Pixel);
}

bool UPixelReaderLibrary::IsPixelDataValid(const FPixelData& PixelData)
{
	return PixelData.IsValid();
}

int32 UPixelReaderLibrary::GetWidth(const FPixelData& PixelData)
{
	return PixelData.GetWidth();
}

int32 UPixelReaderLibrary::GetHeight(const FPixelData& PixelData)
{
	return PixelData.GetHeight();
}

void UPixelReaderLibrary::SetSize(UPARAM(ref)FPixelData& PixelData, int32 Width, int32 Height)
{
	PixelData.SetSize(Width, Height);
}

bool UPixelReaderLibrary::SetPixel(UPARAM(ref)FPixelData& PixelData, const FIntPoint& Coordinates, const FColor& Pixel)
{
	return PixelData.SetPixel(Coordinates, Pixel);
}

void UPixelReaderLibrary::SetsRGB(UPARAM(ref)FPixelData& PixelData, bool NewValue)
{
	PixelData.sRGB = NewValue;
}

void UPixelReaderLibrary::DoReadPixelsTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EMultiTask2BranchesNoCancel& Out, UTexture* Texture, FPixelData& PixelData)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadPixelsTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Texture)
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadPixelsTask: Invalid Texture Object. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	FTextureResource* TextureResource = Texture->Resource;
	const FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;

	if (!Texture2D)
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadPixelsTask: Invalid Texture Object. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	const int32 SizeX = Texture2D->GetSizeX();
	const int32 SizeY = Texture2D->GetSizeY();

	if (SizeX * SizeY <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadPixelsTask: Invalid Texture Size. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	EPixelFormat Format;
	Format = Texture2D->GetFormat();

	EPixelDataType Type;

	switch (Format)
	{
	default:
		FFrame::KismetExecutionMessage(TEXT("DoReadPixelsTask: Unsupported texture format."), ELogVerbosity::Error);
		return;

	case PF_FloatRGBA:
		Type = EPixelDataType::FloatColor;
		break;
	case PF_A32B32G32R32F:
		Type = EPixelDataType::LinearColor;
		break;
	case PF_R8G8B8A8:
		Type = EPixelDataType::ColorRGBA;
		break;
	case PF_B8G8R8A8:
		Type = EPixelDataType::ColorBGRA;
		break;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FPixelReaderTaskAction* Action = LatentActionManager.FindExistingAction<FPixelReaderTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoReadPixelsTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FPixelReaderTaskAction(WorldContextObject, Out, LatentInfo, Texture, PixelData);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}


void UPixelReaderLibrary::DoReadFileToPixelDataTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EMultiTask2BranchesNoCancel& Out, FString File, FPixelData& PixelData, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadFileToPixelDataTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (!FPaths::FileExists(File))
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadFileToPixelDataTask: File does not exist."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FFileToPixelDataTaskAction* Action = LatentActionManager.FindExistingAction<FFileToPixelDataTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoReadFileToPixelDataTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FFileToPixelDataTaskAction(WorldContextObject, Out, LatentInfo, File, ExecutionType, ThreadPool, PixelData);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UPixelReaderLibrary::DoReadUrlToPixelDataTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EMultiTask2Branches& Out, FString URL, float Timeout, FPixelData& PixelData, UMultiTaskBase*& Task, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadUrlToPixelDataTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (URL.IsEmpty())
	{
		FFrame::KismetExecutionMessage(TEXT("DoReadUrlToPixelDataTask: Invalid URL."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FUrlToPixelDataTaskAction* Action = LatentActionManager.FindExistingAction<FUrlToPixelDataTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoReadUrlToPixelDataTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FUrlToPixelDataTaskAction(WorldContextObject, Out, LatentInfo, URL, Timeout, ExecutionType, ThreadPool, Task, PixelData);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}


UTexture2D* UPixelReaderLibrary::CreateTextureFromPixelData(UObject* Outer, const FPixelData& PixelData)
{
	if (PixelData.IsValid())
	{
		bool bPowerOfTwo = true;
		if (!FMath::IsPowerOfTwo(PixelData.GetWidth()))
		{
			bPowerOfTwo = false;
		}

		if (!FMath::IsPowerOfTwo(PixelData.GetHeight()))
		{
			bPowerOfTwo = false;
		}


		EPixelFormat PixelFormat = EPixelFormat::PF_FloatRGBA;
		ETextureSourceFormat SourceFormat = ETextureSourceFormat::TSF_RGBA16F;
		bool bWriteSource = false;
		switch (PixelData.Type)
		{
		case EPixelDataType::FloatColor:
			PixelFormat = EPixelFormat::PF_FloatRGBA;
			SourceFormat = ETextureSourceFormat::TSF_RGBA16F;
			bWriteSource = true;
			break;
		case EPixelDataType::LinearColor:
			PixelFormat = EPixelFormat::PF_A32B32G32R32F;
			break;
		case EPixelDataType::ColorRGBA:
			PixelFormat = EPixelFormat::PF_R8G8B8A8;
			SourceFormat = ETextureSourceFormat::TSF_RGBA8;
			bWriteSource = true;
			break;
		case EPixelDataType::ColorBGRA:
			PixelFormat = EPixelFormat::PF_B8G8R8A8;
			SourceFormat = ETextureSourceFormat::TSF_BGRA8;
			bWriteSource = true;
			break;
		default:
			FFrame::KismetExecutionMessage(TEXT("CreateTextureFromPixelData: Invalid Pixel Data type."), ELogVerbosity::Error);
			return nullptr;
		}


		UTexture2D* NewTexture = NewObject<UTexture2D>(Outer);

		NewTexture->PlatformData = new FTexturePlatformData();	// Then we initialize the PlatformData
		NewTexture->PlatformData->SizeX = PixelData.SizeX;
		NewTexture->PlatformData->SizeY = PixelData.SizeY;
		NewTexture->PlatformData->SetNumSlices(1);
		NewTexture->PlatformData->PixelFormat = PixelFormat;

		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		NewTexture->PlatformData->Mips.Add(Mip);
		Mip->SizeX = PixelData.SizeX;
		Mip->SizeY = PixelData.SizeY;


		const int32 Size = (PixelData.Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((PixelData.Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
		const int32 ExpectedDataSize = PixelData.SizeX * PixelData.SizeY * Size;
		Mip->BulkData.Lock(LOCK_READ_WRITE);
		uint8* TextureData = (uint8*)Mip->BulkData.Realloc(ExpectedDataSize);
		FMemory::Memcpy(TextureData, PixelData.Data.GetData(), ExpectedDataSize);
		Mip->BulkData.Unlock();

#if WITH_EDITOR
		if (!bPowerOfTwo)
		{
			NewTexture->MipGenSettings = TMGS_NoMipmaps;
		}
		if (bWriteSource)
		{
			NewTexture->Source.Init(PixelData.SizeX, PixelData.SizeY, 1, 1, SourceFormat, PixelData.Data.GetData());
		}
#endif

		NewTexture->UpdateResource();
		return NewTexture;
	}
	else {
		FFrame::KismetExecutionMessage(TEXT("CreateTextureFromPixelData: Invalid Pixel Data size."), ELogVerbosity::Error);
	}

	return nullptr;
}
