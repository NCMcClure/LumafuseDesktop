// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "MultiTask2VoxelLibrary.h"
#include "Engine/Engine.h"
#include "UObject/Script.h"
#include "Misc/CoreMisc.h"
#include "CoreGlobals.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshResources.h"
#include "ContentStreaming.h"

void UMultiTask2VoxelLibrary::GetChunkSlotFromLocation(const FVector& Location, const FMarchingCubesSettings& Settings, FIntVector& ReturnValue)
{
	const FVector ChunkOffSet = (FVector(FMath::Clamp(Settings.WorldSize.X, 1, Settings.WorldSize.X), FMath::Clamp(Settings.WorldSize.Y, 1, Settings.WorldSize.Y), FMath::Clamp(Settings.WorldSize.Z, 1, Settings.WorldSize.Z)) * (FVector(Settings.Units) - VOXELMARGIN)) * (Settings.Resolution * 0.5f);
	const FVector RelativeLocation = (Location + ChunkOffSet) / ((FVector(Settings.Units) - VOXELMARGIN) * Settings.Resolution);
	ReturnValue = FIntVector(FMath::FloorToInt(RelativeLocation.X), FMath::FloorToInt(RelativeLocation.Y), FMath::FloorToInt(RelativeLocation.Z));
}

void UMultiTask2VoxelLibrary::GetLocationFromChunkSlot(const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, FVector& ReturnValue)
{
	const FVector ChunkOffSet = (FVector(FMath::Clamp(Settings.WorldSize.X, 1, Settings.WorldSize.X), FMath::Clamp(Settings.WorldSize.Y, 1, Settings.WorldSize.Y), FMath::Clamp(Settings.WorldSize.Z, 1, Settings.WorldSize.Z)) * (FVector(Settings.Units) - VOXELMARGIN)) * (Settings.Resolution * 0.5f);
	ReturnValue = (FVector(ChunkSlot) + FVector(0.5f)) * (FVector(Settings.Units) - VOXELMARGIN) * Settings.Resolution - ChunkOffSet;
}

void UMultiTask2VoxelLibrary::VoxelCoordinatesToLocalLocation(const FVector& VoxelCoordinates, const FMarchingCubesSettings& Settings, FVector& ReturnValue)
{
	ReturnValue = VoxelCoordinates * Settings.Resolution - ((FVector(Settings.Units) - VOXELMARGIN) * (Settings.Resolution * 0.5f));
}

void UMultiTask2VoxelLibrary::GetRelevantSlots(const FIntVector& RootSlot, int32 Radius, const TArray<FIntVector>& ExistingChunks, const FMarchingCubesSettings& Settings, TArray<FIntVector>& NewSlots, TArray<FIntVector>& OldRelevantSlots)
{
	const int32 MinX = RootSlot.X - Radius;
	const int32 MaxX = RootSlot.X + Radius;

	const int32 MinY = RootSlot.Y - Radius;
	const int32 MaxY = RootSlot.Y + Radius;

	const int32 MinZ = RootSlot.Z - Radius;
	const int32 MaxZ = RootSlot.Z + Radius;

	for (int32 x = MinX; x <= MaxX; ++x)
	{
		for (int32 y = MinY; y <= MaxY; ++y)
		{
			for (int32 z = MinZ; z <= MaxZ; ++z)
			{
				const FIntVector NewSlot = FIntVector(x, y, z);
				if (ExistingChunks.Contains(NewSlot))
				{
					OldRelevantSlots.AddUnique(NewSlot);
				}
				else {
					NewSlots.AddUnique(NewSlot);
				}
			}
		}
	}

}

bool UMultiTask2VoxelLibrary::IsInRange(const FIntVector& VoxelCoordinates, const FMarchingCubesSettings& Settings)
{
	return VoxelCoordinates.X >= 0 && VoxelCoordinates.X < Settings.Units.X&& VoxelCoordinates.Y >= 0 && VoxelCoordinates.Y < Settings.Units.Y&& VoxelCoordinates.Z >= 0 && VoxelCoordinates.Z < Settings.Units.Z;
}

void UMultiTask2VoxelLibrary::GetVoxelCoordinatesAtLocation(const FTransform& GeometryTransform, const FVector& Location, const FMarchingCubesSettings& Settings, FIntVector& ReturnValue)
{
	const FVector GeometryLocation = GeometryTransform.GetTranslation();
	const FVector UnRotatedLocation = GeometryTransform.GetRotation().UnrotateVector(Location - GeometryLocation) + GeometryLocation;
	const FVector HalfSlot = FVector(Settings.Units) * (Settings.Resolution * 0.5f);
	ReturnValue = FIntVector((UnRotatedLocation - (GeometryLocation - HalfSlot)) / Settings.Resolution);
}

bool UMultiTask2VoxelLibrary::GetNeighbor(const FIntVector& VoxelCoordinates, const FIntVector& Direction, const FMarchingCubesSettings& Settings, FIntVector& Neighbor)
{
	Neighbor = VoxelCoordinates + Direction;
	return IsInRange(Neighbor, Settings);
}

void UMultiTask2VoxelLibrary::GetVoxelCoordinatesInRadius(const FTransform& GeometryTransform, const FVector& BrushLocation, const FVector& Radius, const FMarchingCubesSettings& Settings, TArray<FIntVector>& ReturnValue)
{
	ReturnValue.Empty();

	const FVector HalfRadius = (Radius / Settings.Resolution) * 0.5f;

	FIntVector VoxelCoordinate;
	GetVoxelCoordinatesAtLocation(GeometryTransform, BrushLocation, Settings, VoxelCoordinate);

	const int32 MinX = FMath::Clamp(FMath::FloorToInt((float)VoxelCoordinate.X - HalfRadius.X), 0, Settings.Units.X);
	const int32 MaxX = FMath::Clamp(FMath::CeilToInt((float)VoxelCoordinate.X + HalfRadius.X), 0, Settings.Units.X);

	const int32 MinY = FMath::Clamp(FMath::FloorToInt((float)VoxelCoordinate.Y - HalfRadius.Y), 0, Settings.Units.Y);
	const int32 MaxY = FMath::Clamp(FMath::CeilToInt((float)VoxelCoordinate.Y + HalfRadius.Y), 0, Settings.Units.Y);

	const int32 MinZ = FMath::Clamp(FMath::FloorToInt((float)VoxelCoordinate.Z - HalfRadius.Z), 0, Settings.Units.Z);
	const int32 MaxZ = FMath::Clamp(FMath::CeilToInt((float)VoxelCoordinate.Z + HalfRadius.Z), 0, Settings.Units.Z);

	const int32 XSize = MaxX - MinX;
	const int32 YSize = MaxY - MinY;
	const int32 ZSize = MaxZ - MinZ;

	const int32 CoordAmount = XSize * YSize * ZSize;

	if (CoordAmount > 0)
	{
		ReturnValue.SetNumZeroed(CoordAmount);
		int32 CurrentIndex = -1;
		for (int32 X = MinX; X < MaxX; ++X)
		{
			for (int32 Y = MinY; Y < MaxY; ++Y)
			{
				for (int32 Z = MinZ; Z < MaxZ; ++Z)
				{
					const FIntVector CurrentCoordinates(X, Y, Z);
					CurrentIndex++;
					ReturnValue[CurrentIndex] = CurrentCoordinates;
				}
			}
		}
		if (CurrentIndex >= 0 && ((CurrentIndex + 1) != CoordAmount))
		{
			ReturnValue.SetNum(CurrentIndex + 1);
		}
	}
}

int32 UMultiTask2VoxelLibrary::VoxelCoordinatesToLinearIndex(const FIntVector& VoxelCoordinates, const FMarchingCubesSettings& Settings)
{
	return FMath::Clamp(VoxelCoordinates.X + Settings.Units.X * VoxelCoordinates.Y + Settings.Units.X * Settings.Units.Y * VoxelCoordinates.Z, -1, (Settings.Units.X * Settings.Units.Y * Settings.Units.Z));
}

void UMultiTask2VoxelLibrary::LinearIndexToVoxelCoordinates(const int32& LinearIndex, const FMarchingCubesSettings& Settings, FIntVector& VoxelCoordinates)
{
	VoxelCoordinates.X = (LinearIndex % Settings.Units.X);
	VoxelCoordinates.Y = ((LinearIndex / Settings.Units.X) % Settings.Units.Y);
	VoxelCoordinates.Z = (LinearIndex / (Settings.Units.X * Settings.Units.Y));
}

int32 UMultiTask2VoxelLibrary::GetVerticeCount_VoxelData(const FMarchingCubesData& VoxelData)
{
	return VoxelData.Data.VertexCount();
}

int32 UMultiTask2VoxelLibrary::GetTriangleCount_VoxelData(const FMarchingCubesData& VoxelData)
{
	return VoxelData.Data.TriangleCount();
}

bool UMultiTask2VoxelLibrary::GetVertice_VoxelData(const FMarchingCubesData& VoxelData, int32 VerticeIndex, FVector& OutValue)
{
	if (VoxelData.Data.IsVertex(VerticeIndex))
	{
		OutValue = FVector(VoxelData.Data.GetVertex(VerticeIndex));
		return true;
	}
	return false;
}

bool UMultiTask2VoxelLibrary::GetTriangle_VoxelData(const FMarchingCubesData& VoxelData, int32 TriangleIndex, int32& A, int32& B, int32& C)
{
	if (VoxelData.Data.IsTriangle(TriangleIndex))
	{
		const FIndex3i Triangle = VoxelData.Data.GetTriangle(TriangleIndex);
		A = Triangle[0];
		B = Triangle[1];
		C = Triangle[2];
		return true;
	}
	return false;
}

void UMultiTask2VoxelLibrary::DoGenerateMarchingCubesTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UGenerateMarchingCubesTask> Class, const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, EMarchingCubesAlgorithm Algorithm, bool bForceManifold, bool bUseSharedPoints, UMultiTaskBase*& Task, FMarchingCubesData& VoxelData, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{

	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (!(Settings.Units.X > VOXELMARGIN))
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Units X Size must be higher than 4."), ELogVerbosity::Error);
		return;
	}

	if (!(Settings.Units.Y > VOXELMARGIN))
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Units Y Size must be higher than 4."), ELogVerbosity::Error);
		return;
	}

	if (!(Settings.Units.Z > VOXELMARGIN))
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Units Z Size must be higher than 4."), ELogVerbosity::Error);
		return;
	}

	if (FMath::IsNearlyZero(Settings.Resolution))
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Resolution is too low."), ELogVerbosity::Error);
		return;
	}


	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FGenerateMarchingCubesTaskAction* Action = LatentActionManager.FindExistingAction<FGenerateMarchingCubesTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoGenerateMarchingCubesTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FGenerateMarchingCubesTaskAction(WorldContextObject, Out, LatentInfo, Class, ChunkSlot, Settings, Algorithm, bForceManifold, bUseSharedPoints, VoxelData, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiTask2VoxelLibrary::DoConvertVoxelDataToMeshDataTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UGenerateMarchingCubesTask> Class, const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, EMarchingCubesNormal NormalType, bool bUseFlatShading, UPARAM(ref)FMarchingCubesData& VoxelData, const TArray<FMarchingCubesSimplifierSettings>& SimplifierSettings, UMultiTaskBase*& Task, TArray<FMarchingCubesMeshData>& MeshData, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType, UMultiTaskThreadPool* ThreadPool)
{
	if (nullptr == WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Invalid WorldContextObject. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (!(Settings.Units.X > VOXELMARGIN))
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Units X Size must be higher than 4."), ELogVerbosity::Error);
		return;
	}

	if (!(Settings.Units.Y > VOXELMARGIN))
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Units Y Size must be higher than 4."), ELogVerbosity::Error);
		return;
	}

	if (!(Settings.Units.Z > VOXELMARGIN))
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Units Z Size must be higher than 4."), ELogVerbosity::Error);
		return;
	}

	if (FMath::IsNearlyZero(Settings.Resolution))
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Resolution is too low."), ELogVerbosity::Error);
		return;
	}

	if (ExecutionType == ETaskExecutionType::ThreadPool && ThreadPool && ThreadPool->GetThreadsNum() <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: Invalid Thread Pool"), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FVoxelDataToMeshDataTaskAction* Action = LatentActionManager.FindExistingAction<FVoxelDataToMeshDataTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && Action->IsRunning())
		{
			FFrame::KismetExecutionMessage(TEXT("DoConvertVoxelDataToMeshDataTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else {
			Action = new FVoxelDataToMeshDataTaskAction(WorldContextObject, Out, LatentInfo, Class, ChunkSlot, Settings, NormalType, bUseFlatShading, VoxelData, SimplifierSettings, MeshData, ExecutionType, ThreadPool, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

