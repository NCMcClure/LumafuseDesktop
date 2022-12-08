// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "MultiThreadTask.h"
#include "Engine/StaticMesh.h"
#include "DynamicMesh3.h"
#include "Util/ProgressCancel.h"
#include "ProceduralMeshComponent.h"
#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif
#include "GenerateMarchingCubesTask.generated.h"

#ifndef VOXELMARGIN
#define VOXELMARGIN 4
#endif

UENUM(BlueprintType)
enum class EMarchingCubesAlgorithm : uint8
{
	Simple,
	Dual,
	Tetrahedrons,
};

UENUM(BlueprintType)
enum class EMarchingCubesSimplifierMethod : uint8
{
	QEM,
	VolumePreservation,
};

UENUM(BlueprintType)
enum class EMarchingCubesSimplifierType : uint8
{
	Vertex,
	Triangle,
};

UENUM(BlueprintType)
enum class EMarchingCubesNormal : uint8
{
	GradientAdvanced,
	GradientBasic,
	Triangle,
};

/**
* Holds the generic settings for Marching Cubes algorithm.
*/
USTRUCT(BlueprintType)
struct MULTITASK2_API FMarchingCubesSettings
{
	GENERATED_BODY()
	/**
	* Grid dimension sizes in each direction.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
		FIntVector Units;

	/**
	* Voxel Size.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
		float Resolution;

	/**
	* The minimum ISOLevel. All voxel coordinates with values >= ISOLevel will contribute to the mesh.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
		float ISOLevel;

	/**
	* Whether to invert the generated mesh triangles.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
		bool bInverted;

	/**
	* World size in chunks on each axis.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
		FIntVector WorldSize;

	FMarchingCubesSettings()
		: Units(FIntVector(32, 32, 32))
		, Resolution(500.f)
		, ISOLevel(0.5f)
		, bInverted(false)
		, WorldSize(FIntVector(1, 1, 1))
	{}
};


struct MULTITASK2_API FMarchingCubesDualPointKey
{
	FIntVector Coordinates;
	uint16 PointCode;

	FMarchingCubesDualPointKey()
		: Coordinates(FIntVector(0, 0, 0))
		, PointCode(0)
	{}

	FMarchingCubesDualPointKey(const FIntVector& InCoordinates, const uint16& InPointCode)
		: Coordinates(InCoordinates)
		, PointCode(InPointCode)
	{}

	bool operator==(const FMarchingCubesDualPointKey& Other) const
	{
		return Coordinates == Other.Coordinates && PointCode == Other.PointCode;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FMarchingCubesDualPointKey& DualPointKey)
	{
		return FCrc::MemCrc32(&DualPointKey, sizeof(FMarchingCubesDualPointKey));
	}
};


/**
* Holds the density data for specific {X, Y, Z} coordinates in the voxel grid.
*/
USTRUCT(BlueprintType)
struct MULTITASK2_API FMarchingCubesDensityPoint
{
	GENERATED_BODY()
	/**
	* Point density
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Data Point")
		float Value;

	/**
	* Point color
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Data Point")
		FLinearColor Color;

	FMarchingCubesDensityPoint()
		: Value(0.0f)
		, Color(FLinearColor::Black)
	{}

	FMarchingCubesDensityPoint(const float& InValue, const FLinearColor& InColor)
		: Value(InValue)
		, Color(InColor)
	{}

	bool operator==(const FMarchingCubesDensityPoint& Other) const
	{
		return Value == Other.Value && Color == Other.Color;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FMarchingCubesDensityPoint& DensityPoint)
	{
		return FCrc::MemCrc32(&DensityPoint, sizeof(FMarchingCubesDensityPoint));
	}
};

USTRUCT(BlueprintType)
struct MULTITASK2_API FMarchingCubesData
{
	GENERATED_BODY()
		FDynamicMesh3 Data;

	void Reset()
	{
		Data.Clear();
	}
};


USTRUCT(BlueprintType)
struct MULTITASK2_API FMarchingCubesSimplifierSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simplifier Settings")
		float Quality;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simplifier Settings")
		EMarchingCubesSimplifierMethod Method;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simplifier Settings")
		EMarchingCubesSimplifierType TargetType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simplifier Settings")
		bool bForcePreserveBoundaryShape;

	FMarchingCubesSimplifierSettings()
		: Quality(1.0f)
		, Method(EMarchingCubesSimplifierMethod::QEM)
		, TargetType(EMarchingCubesSimplifierType::Vertex)
		, bForcePreserveBoundaryShape(true)
	{}

	friend FORCEINLINE uint32 GetTypeHash(const FMarchingCubesSimplifierSettings& QEMSettings)
	{
		return FCrc::MemCrc32(&QEMSettings, sizeof(FMarchingCubesSimplifierSettings));
	}
};

USTRUCT(BlueprintType)
struct MULTITASK2_API FMarchingCubesMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Data")
		TArray<FVector> Positions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Data")
		TArray<int32> Indices;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Data")
		TArray<FVector> Normals;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Data")
		TArray<FVector2D> UVs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Data")
		TArray<FLinearColor> Colors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Data")
		TArray<FProcMeshTangent> Tangents;

	void Reset()
	{
		Positions.Empty();
		Normals.Empty();
		Tangents.Empty();
		UVs.Empty();
		Colors.Empty();
		Indices.Empty();
	}
};

UCLASS(HideDropdown, Blueprintable, NotBlueprintType, hidecategories = (Object, Events), meta = (DontUseGenericSpawnObject = "true"))
class MULTITASK2_API UGenerateMarchingCubesTask : public UMultiThreadTask
{
	friend class FGenerateMarchingCubesTaskAction;
	friend class FVoxelDataToMeshDataTaskAction;
	GENERATED_BODY()

public:

	virtual bool Start() override;

	/**
	* Called on Background Thread when the Task is executed.
	*/
	virtual void TaskBody_Implementation() override;

	virtual void Cancel() override;

	void GenerateVoxelData();

	void ConvertVoxelDataToMeshData();

	/**
	* Dynamically build density data using blueprints. This implementation is used by Marching Cubes algorithm to generate density data.
	* This function executes on background thread.
	* @param VoxelCoordinates	{X, Y, Z} coordinates in the voxel grid.
	* @param ChunkSlot			{X, Y, Z} coordinates of the geometry in the chunk grid. If not using a chunk system, could use {0, 0, 0}.
	* @param Settings			Generic settings for Marching Cubes algorithm.
	* @param DensityPoint		Computed density point.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Density Data Builder")
		void ConstructDensityPoint(const FIntVector& VoxelCoordinates, FMarchingCubesDensityPoint& DensityPoint);
	virtual void ConstructDensityPoint_Implementation(const FIntVector& VoxelCoordinates, FMarchingCubesDensityPoint& DensityPoint);

protected:
	virtual int32 AddVertex(const FVector& VoxelCoordinates);
	virtual void AddTriangle(const int32& Index1, const int32& Index2, const int32& Index3);
	virtual FMarchingCubesDensityPoint* GetDensityPoint(const FIntVector& VoxelCoordinates);
	int32 VoxelCoordinatesToLinearIndex(const FIntVector& VoxelCoordinates) const;

	void GenerateMeshDataForVoxelCoordinates_DMC(const FIntVector& VoxelCoordinates);
	void GenerateMeshDataForVoxelCoordinates_MC(const FIntVector& VoxelCoordinates);
	void GenerateMeshDataForVoxelCoordinates_MT(const FIntVector& VoxelCoordinates);

private:
	uint8 GetCellCode(const FIntVector& VoxelCoordinates);
	uint16 GetDualPointCode(const FIntVector& VoxelCoordinates, const int32& Edge);
	void CalculateDualPoint(const FIntVector& VoxelCoordinates, const uint16& PointCode, FVector& OutCoordinates);
	int32 GetSharedDualPointIndex(const FIntVector& VoxelCoordinates, const int32& Edge);
	FVector VertexInterp(const FVector& p1, const FVector& p2, const float& valp1, const float& valp2) const;

	void GetNormal(const FVector& VoxelCoordinates, FVector& ReturnValue);
	void MakePerpVectors(const FVector& Normal, FVector& OutPerp1, FVector& OutPerp2) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Meta = (ClampMin = "0", UIMin = "0"), Category = "General")
		int32 ResetRunawayIterationCount = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Marching Cubes")
		FMarchingCubesSettings Settings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Marching Cubes")
		FIntVector ChunkSlot;

	FMarchingCubesData VoxelData;
	TArray<FMarchingCubesMeshData> MeshData;
protected:
	bool bGenerateMeshData = false;
	TArray<TSharedPtr<FMarchingCubesDensityPoint>> DensityData;
private:
	TMap<FMarchingCubesDualPointKey, int32> PointMap;
	TMap<FVector, int32> MCPointMap;
	TArray<FMarchingCubesSimplifierSettings> LODSettings;

	/**
	* Marching Cubes algorithm used to generate the geometry.
	*/
	EMarchingCubesAlgorithm Algorithm;

	/**
	* Force Manifold mesh generation. Enabling this removes dualism and is used only by DMC algorithm.
	* !!!Experimental!!!
	*/
	bool bForceManifold;

	/**
	* If set to true, enables use of shared points to reduce vertices amount.
	*/
	bool bUseSharedPoints;

	/**
	* Normal algorithm to be used when generating Mesh Data
	*/
	EMarchingCubesNormal NormalType;

	/**
	* Whether Normals use flat shading.
	*/
	bool bUseFlatShading;

	FProgressCancel Progress;

};

class MULTITASK2_API FGenerateMarchingCubesTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	FMarchingCubesData& VoxelData;
	bool bStarted;
public:
	FGenerateMarchingCubesTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class UGenerateMarchingCubesTask> Class, const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, const EMarchingCubesAlgorithm& Algorithm, const bool& bForceManifold, const bool& bUseSharedPoints, FMarchingCubesData& InVoxelData, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, Class)
		, Branches(InBranches)
		, VoxelData(InVoxelData)
		, bStarted(false)
	{
		OutTask = Task;

		UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
		if (LocalTask)
		{
			LocalTask->ChunkSlot = ChunkSlot;
			LocalTask->Settings = Settings;

			LocalTask->Algorithm = Algorithm;
			LocalTask->bForceManifold = bForceManifold;
			LocalTask->bUseSharedPoints = bUseSharedPoints;

			LocalTask->bGenerateMeshData = false;

			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();

			LocalTask->ExecutionType = InExecutionType;
			LocalTask->ThreadPool = ThreadPool;
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
			if (!IsCanceled())
			{
				if (!IsRunning())
				{
					UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
					if (LocalTask)
					{
						VoxelData = MoveTemp(LocalTask->VoxelData);
						LocalTask->DensityData.Empty();
						LocalTask->PointMap.Empty();
						LocalTask->MCPointMap.Empty();
						LocalTask->VoxelData = FMarchingCubesData();
					}
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
				if (LocalTask)
				{
					LocalTask->DensityData.Empty();
					LocalTask->PointMap.Empty();
					LocalTask->MCPointMap.Empty();
					LocalTask->VoxelData = FMarchingCubesData();
				}
				Branches = EMultiTask2Branches::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
			if (LocalTask)
			{
				LocalTask->DensityData.Empty();
				LocalTask->PointMap.Empty();
				LocalTask->MCPointMap.Empty();
				LocalTask->VoxelData = FMarchingCubesData();
			}
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2Branches::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};

class MULTITASK2_API FVoxelDataToMeshDataTaskAction : public FSingleTaskActionBase
{
	EMultiTask2Branches& Branches;
	TArray<FMarchingCubesMeshData>& MeshData;
	bool bStarted;
public:
	FVoxelDataToMeshDataTaskAction(UObject* InObject, EMultiTask2Branches& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class UGenerateMarchingCubesTask> Class, const FIntVector& ChunkSlot, const FMarchingCubesSettings& Settings, const EMarchingCubesNormal& NormalType, const bool& bUseFlatShading, const FMarchingCubesData& InVoxelData, const TArray<FMarchingCubesSimplifierSettings>& SimplifierSettings, TArray<FMarchingCubesMeshData>& InMeshData, const ETaskExecutionType& InExecutionType, UMultiTaskThreadPool* ThreadPool, UMultiTaskBase*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, Class)
		, Branches(InBranches)
		, MeshData(InMeshData)
		, bStarted(false)
	{
		OutTask = Task;

		UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
		if (LocalTask)
		{
			LocalTask->ChunkSlot = ChunkSlot;
			LocalTask->Settings = Settings;

			LocalTask->NormalType = NormalType;
			LocalTask->bUseFlatShading = bUseFlatShading;

			LocalTask->LODSettings = SimplifierSettings;
			LocalTask->bGenerateMeshData = true;

			Branches = EMultiTask2Branches::OnStart;
			LocalTask->BodyFunction();

			LocalTask->VoxelData = InVoxelData;

			LocalTask->ExecutionType = InExecutionType;
			LocalTask->ThreadPool = ThreadPool;
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
			if (!IsCanceled())
			{
				if (!IsRunning())
				{
					UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
					if (LocalTask)
					{
						MeshData = LocalTask->MeshData;
						LocalTask->MeshData.Empty();
						LocalTask->DensityData.Empty();
						LocalTask->PointMap.Empty();
						LocalTask->MCPointMap.Empty();
						LocalTask->VoxelData = FMarchingCubesData();
					}
					Branches = EMultiTask2Branches::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
				if (LocalTask)
				{
					LocalTask->MeshData.Empty();
					LocalTask->DensityData.Empty();
					LocalTask->PointMap.Empty();
					LocalTask->MCPointMap.Empty();
					LocalTask->VoxelData = FMarchingCubesData();
				}
				Branches = EMultiTask2Branches::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else {
			UGenerateMarchingCubesTask* LocalTask = Cast<UGenerateMarchingCubesTask>(Task);
			if (LocalTask)
			{
				LocalTask->MeshData.Empty();
				LocalTask->DensityData.Empty();
				LocalTask->PointMap.Empty();
				LocalTask->MCPointMap.Empty();
				LocalTask->VoxelData = FMarchingCubesData();
			}
			//If we reached this point it means the task was unable to start.
			Branches = EMultiTask2Branches::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};