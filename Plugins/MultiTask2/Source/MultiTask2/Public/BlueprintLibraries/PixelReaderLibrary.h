// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PixelReaderTask.h"
#include "PixelReaderLibrary.generated.h"

class UTexture2D;

UCLASS()
class MULTITASK2_API UPixelReaderLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

	/**
	* Return Pixel at Texture Coordinates from Pixel Data.
	* @param PixelData			Raw Pixel Data.
	* @param Coordinates		Texture Coordinates.
	* @param Pixel				Retrieved Pixel.
	* @return Returns True if Pixel was successfully retrieved.
	*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Pixel"), Category = "Multi Task 2|Pixel Reader")
		static bool GetPixel(const FPixelData& PixelData, const FIntPoint& Coordinates, FColor& Pixel);

	/**
	* Checks whether the Pixel Data is valid.
	* @param PixelData			Raw Pixel Data.
	* @return Returns True if Pixel Data is valid.
	*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Pixel Data Valid"), Category = "Multi Task 2|Pixel Reader")
		static bool IsPixelDataValid(const FPixelData& PixelData);

	/**
	* Get the Width of the Texture the Pixel Data was read from.
	* @param PixelData			Raw Pixel Data.
	* @return Texture Width.
	*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Width"), Category = "Multi Task 2|Pixel Reader")
		static int32 GetWidth(const FPixelData& PixelData);

	/**
	* Get the Height of the Texture the Pixel Data was read from.
	* @param PixelData			Raw Pixel Data.
	* @return Texture Height.
	*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Height"), Category = "Multi Task 2|Pixel Reader")
		static int32 GetHeight(const FPixelData& PixelData);

	/**
	* Get the Height of the Texture the Pixel Data was read from.
	* @param PixelData			Raw Pixel Data.
	* @param Width				Texture Width.
	* @param Height				Texture Height.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Size"), Category = "Multi Task 2|Pixel Reader")
		static void SetSize(UPARAM(ref) FPixelData& PixelData, int32 Width, int32 Height);

	/**
	* Set Pixel at Texture Coordinates in Pixel Data.
	* @param PixelData			Raw Pixel Data.
	* @param Coordinates		Texture Coordinates.
	* @param Pixel				New Pixel.
	* @return Returns True if the pixel value was succesfully changed.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Pixel"), Category = "Multi Task 2|Pixel Reader")
		static bool SetPixel(UPARAM(ref)FPixelData& PixelData, const FIntPoint& Coordinates, const FColor& Pixel);

	/**
	* Update sRGB
	* @param PixelData			Raw Pixel Data.
	* @param NewValue			New sRGB value.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set sRGB"), Category = "Multi Task 2|Pixel Reader")
		static void SetsRGB(UPARAM(ref)FPixelData& PixelData, bool NewValue);

	/**
	* Read Pixel data from a Texture Object.
	* Texture Object can be UTexture2D or UTextureRenderTarget2D.
	* Reading is running on Render Thread.
	* @param Texture Texture Object.
	* @param PixelData	Output pixel data
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Read Pixels Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static void DoReadPixelsTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EMultiTask2BranchesNoCancel& Out, UTexture* Texture, FPixelData& PixelData);

	/**
	* Read an IMG File to Pixel Data.
	* Reading is running on background Thread.
	* @param File		PNG File.
	* @param PixelData	Output pixel data
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Read File to Pixel Data Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Pixel Reader")
		static void DoReadFileToPixelDataTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EMultiTask2BranchesNoCancel& Out, FString File, FPixelData& PixelData, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Read an URL to Pixel Data.
	* Reading is running on background Thread.
	* @param URL		URL.
	* @param Timeout	Optional timeout in seconds for this entire HTTP request to complete.
	* @param PixelData	Output pixel data
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Read URL to Pixel Data Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Pixel Reader")
		static void DoReadUrlToPixelDataTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EMultiTask2Branches& Out, FString URL, float Timeout, FPixelData& PixelData, UMultiTaskBase*& Task, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);


	/**
	* Create a new 2D Texture from Pixel Data.
	* @param Outer				Outer Object of the newly created texture.
	* @param PixelData			Raw Pixel Data.
	* @return Newly Created Texture.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create Texture from Pixel Data"), Category = "Multi Task 2|Pixel Reader")
		static UTexture2D* CreateTextureFromPixelData(UObject* Outer, const FPixelData& PixelData);
};