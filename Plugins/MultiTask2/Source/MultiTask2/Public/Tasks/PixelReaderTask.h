// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiThreadTask.h"
#include "Math/Float16Color.h"
#include "Math/IntPoint.h"
#include "Math/Color.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#include "PixelReaderTask.generated.h"

enum class EPixelDataType : uint8
{
	FloatColor,
	LinearColor,
	ColorRGBA,
	ColorBGRA
};

USTRUCT(BlueprintType)
struct MULTITASK2_API FPixelData
{
	GENERATED_BODY()
	TArray64<uint8> Data;
	EPixelDataType Type;
	int32 SizeX;
	int32 SizeY;
	bool sRGB = false;

	FPixelData()
		: Type(EPixelDataType::ColorBGRA)
		, SizeX(0)
		, SizeY(0)
	{}

	int32 GetWidth() const
	{
		return SizeX;
	}

	int32 GetHeight() const
	{
		return SizeY;
	}

	void SetWidth(const int32& InWidth)
	{
		SizeX = FMath::Clamp(InWidth, 0, InWidth);
		const int32 Size = (Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
		const int32 ExpectedDataSize = SizeX * SizeY * Size;
		Data.SetNumZeroed(FMath::Clamp(ExpectedDataSize, 0, ExpectedDataSize));
	}

	void SetHeight(const int32& InHeight)
	{
		SizeY = FMath::Clamp(InHeight, 0, InHeight);
		const int32 Size = (Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
		const int32 ExpectedDataSize = SizeX * SizeY * Size;
		Data.SetNumZeroed(FMath::Clamp(ExpectedDataSize, 0, ExpectedDataSize));
	}

	void SetSize(const int32& InWidth, const int32& InHeight)
	{
		SizeX = FMath::Clamp(InWidth, 0, InWidth);
		SizeY = FMath::Clamp(InHeight, 0, InHeight);
		const int32 Size = (Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
		const int32 ExpectedDataSize = SizeX * SizeY * Size;
		Data.SetNumZeroed(FMath::Clamp(ExpectedDataSize, 0, ExpectedDataSize));
	}

	bool IsValid() const
	{
		if (SizeX > 0 && SizeY > 0)
		{
			const int32 Size = (Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
			const int32 ExpectedDataSize = SizeX * SizeY * Size;
			return Data.Num() == ExpectedDataSize && ExpectedDataSize > 0;
		}
		return false;
	}

	bool SetPixel(const FIntPoint& Coordinates, const FColor& Pixel)
	{
		if (Coordinates.X >= 0 && Coordinates.Y >= 0 && Coordinates.X < SizeX && Coordinates.Y < SizeY && IsValid())
		{
			const int32 Size = (Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
			const int32 CurrentPos = (SizeX * Coordinates.Y + Coordinates.X) * Size;
			if (CurrentPos + Size < Data.Num())
			{
				uint8* DataPtr = Data.GetData();
				FFloat16Color FloatColor;
				FLinearColor Color;
				switch (Type)
				{
					case EPixelDataType::FloatColor:
						FloatColor = FFloat16Color(FLinearColor(Pixel));
						FMemory::Memcpy(&DataPtr[CurrentPos], &FloatColor, Size);
						break;
					case EPixelDataType::LinearColor:
						Color = FLinearColor(Pixel);
						FMemory::Memcpy(&DataPtr[CurrentPos], &Pixel, Size);
						break;
					default:
						FMemory::Memcpy(&DataPtr[CurrentPos], &Pixel, Size);
						break;

				}
				return true;
			}
		}
		return false;
	}

	bool GetPixel(const FIntPoint& Coordinates, FColor& Pixel) const
	{
		if (Coordinates.X >= 0 && Coordinates.Y >= 0 && Coordinates.X < SizeX && Coordinates.Y < SizeY && IsValid())
		{
			const int32 Size = (Type == EPixelDataType::FloatColor) ? sizeof(FFloat16Color) : ((Type == EPixelDataType::LinearColor) ? sizeof(FLinearColor) : sizeof(FColor));
			const int32 CurrentPos = (SizeX * Coordinates.Y + Coordinates.X) * Size;
			if (CurrentPos + Size < Data.Num())
			{
				const uint8* DataPtr = Data.GetData();
				switch (Type)
				{
				case EPixelDataType::FloatColor:
				{
					FFloat16Color Output;
					FMemory::Memcpy(&Output, &DataPtr[CurrentPos], Size);
					Pixel = FLinearColor(Output).ToFColor(sRGB);
					break;
				}
				case EPixelDataType::LinearColor:
				{
					FLinearColor Output;
					FMemory::Memcpy(&Output, &DataPtr[CurrentPos], Size);
					Pixel = Output.ToFColor(sRGB);
					break;
				}
				default:
				{
					FColor Output;
					FMemory::Memcpy(&Pixel, &DataPtr[CurrentPos], Size);
					break;
				}
				}
				return true;
			}
		}

		return false;
	}

	friend FArchive& operator<<(FArchive& Ar, FPixelData& PixelData)
	{
		Ar << PixelData.Data;
		Ar << PixelData.Type;
		Ar << PixelData.SizeX;
		Ar << PixelData.SizeY;
		Ar << PixelData.sRGB;
		return Ar;
	}
};

class UTexture;

UCLASS(HideDropdown, NotBlueprintable, NotBlueprintType, hidecategories = (Object, General), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UPixelReaderTask : public UMultiThreadTask
{

	GENERATED_BODY()

public:

	virtual bool Start() override;

	virtual void TaskBody_Implementation() override;

	virtual bool IsRunning() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
		UTexture* TextureObj;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
		FPixelData PixelData;
protected:
	volatile bool bCompleted = false;
};

class MULTITASK2_API FPixelReaderTaskAction : public FSingleTaskActionBase
{
	EMultiTask2BranchesNoCancel& Branches;
	FPixelData& PixelData;
	bool bStarted;
public:
	FPixelReaderTaskAction(UObject* InObject, EMultiTask2BranchesNoCancel& InBranches, const FLatentActionInfo& LatentInfo, UTexture* InTextureObj, FPixelData& InPixelData)
		: FSingleTaskActionBase(InObject, LatentInfo, UPixelReaderTask::StaticClass())
		, Branches(InBranches)
		, PixelData(InPixelData)
		, bStarted(false)
	{
		UPixelReaderTask* LocalTask = Cast<UPixelReaderTask>(Task);
		if (LocalTask)
		{
			Branches = EMultiTask2BranchesNoCancel::OnStart;
			LocalTask->BodyFunction();
			LocalTask->TextureObj = InTextureObj;
			bStarted = Task->Start();
		}
		else {
			return;
		}
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bStarted)
		{
			if (!IsRunning())
			{
				UPixelReaderTask* LocalTask = Cast<UPixelReaderTask>(Task);
				if (LocalTask)
				{
					PixelData = MoveTemp(LocalTask->PixelData);
				}
				Branches = EMultiTask2BranchesNoCancel::OnCompleted;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2BranchesNoCancel::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};