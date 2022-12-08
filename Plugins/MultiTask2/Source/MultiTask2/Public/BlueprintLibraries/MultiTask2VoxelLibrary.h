// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiThreadTask.h"
#include "MultiTaskThreadPool.h"
#include "GenerateMarchingCubesTask.h"
#include "Components/StaticMeshComponent.h"
#include "MultiTask2VoxelLibrary.generated.h"

UENUM()
enum class ERuntimeStaticMeshCollisionType : uint8
{
	None,
	Simple,
	Convex
};

UCLASS()
class MULTITASK2_API UMultiTask2VoxelLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	* Convert location to chunk slot.
	* @param Location			World Location.
	* @param Settings			Generic settings for Marching Cubes algorithm.
	* @param ChunkRadius		Amount of chunks displayed at once in each direction.
	* @return					{X, Y, Z} coordinates of the geometry in the chunk grid.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities")
		static void GetChunkSlotFromLocation(const FVector& Location, const FMarchingCubesSettings& Settings, FIntVector& ReturnValue);

	/**
	* Convert chunk slot to location.
	* @param ChunkSlot			{X, Y, Z} coordinates of the geometry in the chunk grid. If not using a chunk system, could use {1, 1, 1}.
	* @param Settings			Generic settings for Marching Cubes algorithm.
	* @param ChunkRadius		Amount of chunks displayed at once in each direction.
	* @return					Relative Location.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities")
		static void GetLocationFromChunkSlot(const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, FVector& ReturnValue);

	/**
	* Convert voxel coordinates to Local Space location.
	* @param VoxelCoordinates	{X, Y, Z} coordinates in the voxel grid.
	* @param Settings			Generic settings for Marching Cubes algorithm.
	* @return					Local Space location.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities")
		static void VoxelCoordinatesToLocalLocation(const FVector& VoxelCoordinates, const FMarchingCubesSettings& Settings, FVector& ReturnValue);

	/**
	* Get relevant surrounding slots.
	* @param    RootSlot			Root slot to get the surrounding relevant slots for.
	* @param	Radius				Amount of chunks in each direction.
	* @param    ExistingChunks		Array of existing slots. This should hold slots that you already spawned. If a relevant slot is contained by ExistingChunks then it will be added to OldRelevantSlots array.
	* @param	Settings			Generic settings for Marching Cubes algorithm.
	* @param	NewSlots			Array of slots that are relevant and require spawning.
	* @param	OldRelevantSlots	Array of relevant slots that are relevant but you already spawned.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities", Meta = (AutoCreateRefTerm = "ExistingChunks"))
		static void GetRelevantSlots(const FIntVector& RootSlot, int32 Radius, const TArray<FIntVector>& ExistingChunks, const FMarchingCubesSettings& Settings, TArray<FIntVector>& NewSlots, TArray<FIntVector>& OldRelevantSlots);

	/**
	* Check if a Voxel Coordinate is in range.
	* @param    VoxelCoordinates					{X, Y, Z} coordinates in the voxel grid.
	* @param	Settings					Generic settings for Marching Cubes algorithm.
	* @return								Returning false if it goes out of bounds.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities")
		static bool IsInRange(const FIntVector& VoxelCoordinates, const FMarchingCubesSettings& Settings);

	/**
	* Get Voxel Coordinates at World Location
	* @param    GeometryTransform			The Transform of the Geometry/Chunk.
	* @param    BrushLocation				Location in World Space (Usually Hit Location returned by a Line Trace).
	* @param	Settings					Generic settings for Marching Cubes algorithm.
	* @param	ReturnValue					Voxel coordinates.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities")
		static void GetVoxelCoordinatesAtLocation(const FTransform& GeometryTransform, const FVector& Location, const FMarchingCubesSettings& Settings, FIntVector& ReturnValue);


	/**
	* Get the neighbor Voxel Coordinate in the given direction.
	* @param	VoxelCoordinates	{X, Y, Z} coordinates in the voxel grid.
	* @param	Direction			Direction.
	* @param	Settings		Generic settings for Marching Cubes algorithm.
	* @param	ReturnValue		Voxel coordinates.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities")
		static bool GetNeighbor(const FIntVector& VoxelCoordinates, const FIntVector& Direction, const FMarchingCubesSettings& Settings, FIntVector& Neighbor);

	/**
	* Get Voxel Coordinates in Radius
	* @param    GeometryTransform			The Transform of the Geometry/Chunk.
	* @param    BrushLocation				Location of the Brush in World Space (Usually Hit Location returned by a Line Trace).
	* @param	Radius						Brush Radius
	* @param	Settings					Generic settings for Marching Cubes algorithm.
	* @param	ReturnValue					Array of Voxel coordinates in Radius.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities", Meta = (AutoCreateRefTerm = "Offset"))
		static void GetVoxelCoordinatesInRadius(const FTransform& GeometryTransform, const FVector& BrushLocation, const FVector& Radius, const FMarchingCubesSettings& Settings, TArray<FIntVector>& ReturnValue);

	/**
	* Convert Voxel Coordinates to a Linear Index
	* (This can be used to treat 1D Arrays as 3D)
	* @param	VoxelCoordinates	{X, Y, Z} coordinates in the voxel grid.
	* @param	Settings			Generic settings for Marching Cubes algorithm.
	* @param	ReturnValue			Computed Linear Index.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities", Meta = (DisplayName = "Voxel Coordinates To Linear Index"))
		static int32 VoxelCoordinatesToLinearIndex(const FIntVector& VoxelCoordinates, const FMarchingCubesSettings& Settings);

	/**
	* Convert a Linear Index to Voxel Coordinates
	* (This can be used to treat 1D Arrays as 3D)
	* @param	VoxelCoordinates			{X, Y, Z} coordinates in the voxel grid.
	* @param	Settings					Generic settings for Marching Cubes algorithm.
	* @param	VoxelCoordinates			Computed Voxel coordinates.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Utilities", Meta = (DisplayName = "Linear Index To Voxel Coordinates"))
		static void LinearIndexToVoxelCoordinates(const int32& LinearIndex, const FMarchingCubesSettings& Settings, FIntVector& VoxelCoordinates);

	/**
	* Retrieve Vertice Count in Voxel Data
	* @param	VoxelData					Voxel Data.
	* @return	Vertice Count.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Voxel Data", Meta = (DisplayName = "Get Vertice Count"))
		static int32 GetVerticeCount_VoxelData(const FMarchingCubesData& VoxelData);

	/**
	* Retrieve Triangle Count in Voxel Data
	* @param	VoxelData					Voxel Data.
	* @return	Triangle Count.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Voxel Data", Meta = (DisplayName = "Get Triangle Count"))
		static int32 GetTriangleCount_VoxelData(const FMarchingCubesData& VoxelData);

	/**
	* Retrieve Vertice at index in Voxel Data
	* @param	VoxelData							Voxel Data.
	* @param	VerticeIndex						Vertice Index.
	* @param	OutValue							Vertice position.
	* @return	Returns true if succesfully retrieved Vertice.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Voxel Data", Meta = (DisplayName = "Get Vertice"))
		static bool GetVertice_VoxelData(const FMarchingCubesData& VoxelData, int32 VerticeIndex, FVector& OutValue);

	/**
	* Retrieve Triangle at index in Voxel Data
	* @param	VoxelData							Voxel Data.
	* @param	TriangleIndex						Triangle Index.
	* @param	A									Vertice A Index.
	* @param	B									Vertice B Index.
	* @param	C									Vertice C Index.
	* @return	Returns true if succesfully retrieved Triangle.
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Marching Cubes|Voxel Data", Meta = (DisplayName = "Get Triangle"))
		static bool GetTriangle_VoxelData(const FMarchingCubesData& VoxelData, int32 TriangleIndex, int32& A, int32& B, int32& C);

	/**
	* Generate Marching Cubes geometry in Voxel Space.
	* @param Class				Task Class to be executed.
	* @param ChunkSlot			{X, Y, Z} coordinates of the geometry in the chunk grid. If not using a chunk system, could use {1, 1, 1}.
	* @param Settings			Generic settings for Marching Cubes algorithm.
	* @param Algorithm			Marching Cubes algorithm used to generate the geometry.
	* @param bForceManifold		Force Manifold mesh generation. Enabling this removes dualism and is used only by DMC algorithm.
	* @param bUseSharedPoints	If set to true, enables use of shared points to reduce vertices amount.
	* @param Task				Running Task.
	* @param VoxelData			Generated Marching Cubes Geometry.
	* @param ExecutionType		Execution type.
	* @param ThreadPool			Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Generate Marching Cubes Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Marching Cubes")
		static void DoGenerateMarchingCubesTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UGenerateMarchingCubesTask> Class, const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, EMarchingCubesAlgorithm Algorithm, bool bForceManifold, bool bUseSharedPoints, UMultiTaskBase*& Task, FMarchingCubesData& VoxelData, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);


	/**
	* Convert Marching Cubes Geometry to Renderable geometry.
	* @param Class				Task Class to be executed.
	* @param ChunkSlot			{X, Y, Z} coordinates of the geometry in the chunk grid. If not using a chunk system, could use {1, 1, 1}.
	* @param Settings			Generic settings for Marching Cubes algorithm.
	* @param NormalType			Normal algorithm to be used when generating Mesh Data
	* @param bUseFlatShading	Whether Normals use flat shading.
	* @param VoxelData			Marching Cubes Geometry to convert to Renderable Geometry.
	* @param Task				Running Task.
	* @param SimplifierSettings	LOD Settings.
	* @param MeshData			Generated Renderable Geometry for each LOD.
	* @param ExecutionType		Execution type.
	* @param ThreadPool			Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Convert VoxelData To MeshData Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", AutoCreateRefTerm = "SimplifierSettings"), Category = "Multi Task 2|Marching Cubes")
		static void DoConvertVoxelDataToMeshDataTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UGenerateMarchingCubesTask> Class, const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, EMarchingCubesNormal NormalType, bool bUseFlatShading, UPARAM(ref)FMarchingCubesData& VoxelData, const TArray<FMarchingCubesSimplifierSettings>& SimplifierSettings, UMultiTaskBase*& Task, TArray<FMarchingCubesMeshData>& MeshData, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);
};