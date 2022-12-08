// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "GenerateMarchingCubesTask.h"
#include "MultiTaskThreadPool.h"
#include "MarchingCubesTables.h"
#include "MultiTask2UtilitiesLibrary.h"
#include "MultiTask2VoxelLibrary.h"
#include "MultiTask2MeshSimplifier.h"
#include "RenderUtils.h"
static const FIntVector DMCOffSets[8] =
{
	FIntVector(0, 0, 0), //0
	FIntVector(1, 0, 0), //1
	FIntVector(0, 1, 0), //2
	FIntVector(1, 1, 0), //3
	FIntVector(0, 0, 1), //4
	FIntVector(1, 0, 1), //5
	FIntVector(0, 1, 1), //6
	FIntVector(1, 1, 1)  //7
};

static const FIntVector MCOffSets[8] = 
{
	FIntVector(0, 0, 0),
	FIntVector(1, 0, 0),
	FIntVector(1, 1, 0),
	FIntVector(0, 1, 0),
	FIntVector(0, 0, 1),
	FIntVector(1, 0, 1),
	FIntVector(1, 1, 1),
	FIntVector(0, 1, 1)
};

bool UGenerateMarchingCubesTask::Start()
{
	if (!(Settings.Units.X > VOXELMARGIN))
	{
		return false;
	}

	if (!(Settings.Units.Y > VOXELMARGIN))
	{
		return false;
	}

	if (!(Settings.Units.Z > VOXELMARGIN))
	{
		return false;
	}

	if (FMath::IsNearlyZero(Settings.Resolution))
	{
		return false;
	}

    bCanceled = false;

    UGenerateMarchingCubesTask* Worker = this;

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

void UGenerateMarchingCubesTask::Cancel()
{
	Super::Cancel();
	Progress.CancelF = []() { return true; };
}

void UGenerateMarchingCubesTask::TaskBody_Implementation()
{
	if (bGenerateMeshData)
	{
		ConvertVoxelDataToMeshData();
	}
	else {
		GenerateVoxelData();
	}
}

void UGenerateMarchingCubesTask::GenerateVoxelData()
{
	DensityData.SetNumZeroed(Settings.Units.X * Settings.Units.Y * Settings.Units.Z);
	PointMap.Empty();
	MCPointMap.Empty();

	const int32 VSizeX = Settings.Units.X - VOXELMARGIN;
	const int32 VSizeY = Settings.Units.Y - VOXELMARGIN;
	const int32 VSizeZ = Settings.Units.Z - VOXELMARGIN;
	const int32 Size = (VSizeX * VSizeY * VSizeZ);

	const int32 Step = VOXELMARGIN / 2;

	const bool bResetRunaway = ResetRunawayIterationCount > 0;

	for (int32 CurrentIndex = 0; CurrentIndex < Size; ++CurrentIndex)
	{
		if (IsCanceled())
		{
			return;
		}
		const int32 X = (CurrentIndex % VSizeX) + Step;
		const int32 Y = ((CurrentIndex / VSizeX) % VSizeY) + Step;
		const int32 Z = (CurrentIndex / (VSizeX * VSizeY)) + Step;

		switch (Algorithm)
		{
		case EMarchingCubesAlgorithm::Dual:
			GenerateMeshDataForVoxelCoordinates_DMC(FIntVector(X, Y, Z));
			break;
		case EMarchingCubesAlgorithm::Simple:
			GenerateMeshDataForVoxelCoordinates_MC(FIntVector(X, Y, Z));
			break;
		case EMarchingCubesAlgorithm::Tetrahedrons:
			GenerateMeshDataForVoxelCoordinates_MT(FIntVector(X, Y, Z));
			break;
		default:
			break;
		}

		if (bResetRunaway)
		{
			if (!(CurrentIndex % ResetRunawayIterationCount))
			{
				UMultiTask2UtilitiesLibrary::ResetRunaway();
			}
		}

	}
	
	/*
	int32 before = VoxelData.Data.VertexCount();
	TMultiTask2MeshSimplifier< FQuadricErrord > Simplifier(&VoxelData.Data);
	Simplifier.bForcePreserveBoundaryShape = true;
	Simplifier.Progress = &Progress;
	Simplifier.SimplifyToMinimalPlanar();
	VoxelData.Data.CompactInPlace();
	UE_LOG(LogTemp, Warning, TEXT("Before: %d After: %d"), before, VoxelData.Data.VertexCount());
	*/
}

void UGenerateMarchingCubesTask::ConvertVoxelDataToMeshData()
{
	DensityData.SetNumZeroed(Settings.Units.X * Settings.Units.Y * Settings.Units.Z);

	if (LODSettings.Num() <= 0)
	{
		LODSettings.Add(FMarchingCubesSimplifierSettings());
	}

	MeshData.Empty();
	MeshData.SetNum(LODSettings.Num());


	const bool bResetRunaway = ResetRunawayIterationCount > 0;

	for (int32 LODIdx = 0; LODIdx < LODSettings.Num(); ++LODIdx)
	{
		const float LocalQuality = FMath::Clamp(LODSettings[LODIdx].Quality, 0.0f, 1.0f);

		if (!FMath::IsNearlyZero(1.0f - LocalQuality))
		{

			int32 TargetCount = 0;

			if (LODSettings[LODIdx].TargetType == EMarchingCubesSimplifierType::Vertex)
			{
				TargetCount = FMath::FloorToInt((float)(VoxelData.Data.VertexCount()) * LODSettings[LODIdx].Quality);
			}
			else {
				TargetCount = FMath::FloorToInt((float)(VoxelData.Data.TriangleCount()) * LODSettings[LODIdx].Quality);
			}

			if (TargetCount > 0)
			{
				if (LODSettings[LODIdx].Method == EMarchingCubesSimplifierMethod::QEM)
				{
					TMultiTask2MeshSimplifier< FQuadricErrord > Simplifier(&VoxelData.Data);
					Simplifier.bForcePreserveBoundaryShape = LODSettings[LODIdx].bForcePreserveBoundaryShape;
					Simplifier.Progress = &Progress;
					if (LODSettings[LODIdx].TargetType == EMarchingCubesSimplifierType::Vertex)
					{
						Simplifier.SimplifyToVertexCount(TargetCount);
					}
					else {
						Simplifier.SimplifyToTriangleCount(TargetCount);
					}
				}
				else {
					TMultiTask2MeshSimplifier<FVolPresQuadricErrord> Simplifier(&VoxelData.Data);
					Simplifier.bForcePreserveBoundaryShape = LODSettings[LODIdx].bForcePreserveBoundaryShape;
					Simplifier.Progress = &Progress;
					if (LODSettings[LODIdx].TargetType == EMarchingCubesSimplifierType::Vertex)
					{
						Simplifier.SimplifyToVertexCount(TargetCount);
					}
					else {
						Simplifier.SimplifyToTriangleCount(TargetCount);
					}
				}
				if (IsCanceled())
				{
					return;
				}
				VoxelData.Data.CompactInPlace();

			}
		}

		const int32 VertNum = VoxelData.Data.VertexCount();
		const int32 TriNum = VoxelData.Data.TriangleCount();

		if (IsCanceled())
		{
			return;
		}

		if (VertNum >= 3 && TriNum > 0)
		{
			MeshData[LODIdx].Positions.SetNumZeroed(VertNum);
			MeshData[LODIdx].Tangents.SetNumZeroed(VertNum);
			MeshData[LODIdx].UVs.SetNumZeroed(VertNum);
			MeshData[LODIdx].Colors.SetNumZeroed(VertNum);
			MeshData[LODIdx].Normals.SetNumZeroed(VertNum);

			MeshData[LODIdx].Indices.SetNumZeroed(TriNum * 3);

			int32 TriIdx = 0;
			for (const FIndex3i& Triangle : VoxelData.Data.TrianglesItr())
			{
				if (IsCanceled())
				{
					return;
				}

				const FVector v0 = FVector(VoxelData.Data.GetVertex(Triangle[0]));
				const FVector v1 = FVector(VoxelData.Data.GetVertex(Triangle[1]));
				const FVector v2 = FVector(VoxelData.Data.GetVertex(Triangle[2]));

				const FVector Normal = FVector::CrossProduct(v1 - v0, v2 - v0);

				for (int32 i = 0; i < 3; ++i)
				{
					if (IsCanceled())
					{
						return;
					}

					MeshData[LODIdx].Indices[(TriIdx * 3) + i] = (uint32)Triangle[i];

					if (NormalType == EMarchingCubesNormal::Triangle)
					{
						if (bUseFlatShading)
						{
							MeshData[LODIdx].Normals[Triangle[i]] = Settings.bInverted ? Normal : -Normal;
						}
						else {
							MeshData[LODIdx].Normals[Triangle[i]] += Settings.bInverted ? Normal : -Normal;
						}
						MeshData[LODIdx].Normals[Triangle[i]].Normalize();
					}
				}

				if (bResetRunaway)
				{
					if (!(TriIdx % ResetRunawayIterationCount))
					{
						UMultiTask2UtilitiesLibrary::ResetRunaway();
					}
				}

				TriIdx++;
			}

			FBox BoundingBox;
			BoundingBox.Init();

			int32 VertIdx = 0;
			for (const FVector3d& VoxelCoordinates : VoxelData.Data.VerticesItr())
			{
				if (IsCanceled())
				{
					return;
				}

				const FVector Coords = FVector(VoxelCoordinates);

				UMultiTask2VoxelLibrary::VoxelCoordinatesToLocalLocation(Coords, Settings, MeshData[LODIdx].Positions[VertIdx]);
				const FMarchingCubesDensityPoint* DensityPoint = GetDensityPoint(FIntVector(Coords));

				if (NormalType == EMarchingCubesNormal::GradientAdvanced || NormalType == EMarchingCubesNormal::GradientBasic)
				{
					FVector Normal = FVector::ZeroVector;
					GetNormal(Coords, Normal);
					if (bUseFlatShading)
					{
						MeshData[LODIdx].Normals[VertIdx] = Settings.bInverted ? -Normal : Normal;
					}
					else {
						MeshData[LODIdx].Normals[VertIdx] += Settings.bInverted ? -Normal : Normal;
						MeshData[LODIdx].Normals[VertIdx].Normalize();
					}
				}

				FVector TangentX, TangentY;
				MakePerpVectors(MeshData[LODIdx].Normals[VertIdx], TangentX, TangentY);
				const bool bFlipY = (GetBasisDeterminantSignByte(TangentX, TangentY, MeshData[LODIdx].Normals[VertIdx]) == -127) ? true : false;
				MeshData[LODIdx].Tangents[VertIdx] = FProcMeshTangent(TangentX, bFlipY);

				MeshData[LODIdx].Colors[VertIdx] = DensityPoint->Color;
				MeshData[LODIdx].UVs[VertIdx] = FVector2D(MeshData[LODIdx].Positions[VertIdx].X, MeshData[LODIdx].Positions[VertIdx].Y) / Settings.Resolution;
				BoundingBox += MeshData[LODIdx].Positions[VertIdx];

				if (bResetRunaway)
				{
					if (!(VertIdx % ResetRunawayIterationCount))
					{
						UMultiTask2UtilitiesLibrary::ResetRunaway();
					}
				}

				VertIdx++;
			}
		}
	}

}

void UGenerateMarchingCubesTask::ConstructDensityPoint_Implementation(const FIntVector& VoxelCoordinates, FMarchingCubesDensityPoint& DensityPoint)
{
}

int32 UGenerateMarchingCubesTask::AddVertex(const FVector& VoxelCoordinates)
{
	return (int32)VoxelData.Data.AppendVertex(FVector3d(VoxelCoordinates));
}

void UGenerateMarchingCubesTask::AddTriangle(const int32& Index1, const int32& Index2, const int32& Index3)
{
	int32 Indices[3];
	if (Settings.bInverted)
	{
		Indices[0] = Index3;
		Indices[1] = Index2;
		Indices[2] = Index1;
	}
	else {
		Indices[0] = Index1;
		Indices[1] = Index2;
		Indices[2] = Index3;
	}
	VoxelData.Data.AppendTriangle(Index1, Index2, Index3);
}

FMarchingCubesDensityPoint* UGenerateMarchingCubesTask::GetDensityPoint(const FIntVector& VoxelCoordinates)
{
	const int32 LinearIndex = VoxelCoordinatesToLinearIndex(VoxelCoordinates);
	if (DensityData[LinearIndex].IsValid())
	{
		return DensityData[LinearIndex].Get();
	}
	else {
		FMarchingCubesDensityPoint NewDensityPoint;
		ConstructDensityPoint(VoxelCoordinates, NewDensityPoint);
		DensityData[LinearIndex] = MakeShareable<FMarchingCubesDensityPoint>(new FMarchingCubesDensityPoint(NewDensityPoint.Value, NewDensityPoint.Color));
		return DensityData[LinearIndex].Get();
	}
}

int32 UGenerateMarchingCubesTask::VoxelCoordinatesToLinearIndex(const FIntVector& VoxelCoordinates) const
{
	return FMath::Clamp(VoxelCoordinates.X + Settings.Units.X * VoxelCoordinates.Y + Settings.Units.X * Settings.Units.Y * VoxelCoordinates.Z, -1, (Settings.Units.X * Settings.Units.Y * Settings.Units.Z));
}

void UGenerateMarchingCubesTask::GenerateMeshDataForVoxelCoordinates_DMC(const FIntVector& VoxelCoordinates)
{
	//X Edge quad

	const bool EnteringXEdge = GetDensityPoint(VoxelCoordinates)->Value < Settings.ISOLevel&& GetDensityPoint(VoxelCoordinates + DMCOffSets[1])->Value >= Settings.ISOLevel;
	const bool ExitingXEdge = GetDensityPoint(VoxelCoordinates)->Value >= Settings.ISOLevel && GetDensityPoint(VoxelCoordinates + DMCOffSets[1])->Value < Settings.ISOLevel;

	if (EnteringXEdge || ExitingXEdge)
	{
		FVector vertex0, vertex1, vertex2, vertex3;

		if (!bUseSharedPoints)
		{
			CalculateDualPoint(VoxelCoordinates, GetDualPointCode(VoxelCoordinates, 1), vertex0);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[4], GetDualPointCode(VoxelCoordinates - DMCOffSets[4], 4), vertex1);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[6], GetDualPointCode(VoxelCoordinates - DMCOffSets[6], 64), vertex2);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[2], GetDualPointCode(VoxelCoordinates - DMCOffSets[2], 16), vertex3);
		}

		const int32& index0 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates, 1) : AddVertex(vertex0);
		const int32& index1 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[4], 4) : AddVertex(vertex1);
		const int32& index2 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[6], 64) : AddVertex(vertex2);
		const int32& index3 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[2], 16) : AddVertex(vertex3);

		if (EnteringXEdge)
		{
			AddTriangle(index0, index3, index1);
			AddTriangle(index3, index2, index1);
		}
		else {
			AddTriangle(index0, index1, index3);
			AddTriangle(index1, index2, index3);
		}
	}

	//Y Edge quad

	const bool EnteringYEdge = GetDensityPoint(VoxelCoordinates)->Value < Settings.ISOLevel&& GetDensityPoint(VoxelCoordinates + DMCOffSets[2])->Value >= Settings.ISOLevel;
	const bool ExitingYEdge = GetDensityPoint(VoxelCoordinates)->Value >= Settings.ISOLevel && GetDensityPoint(VoxelCoordinates + DMCOffSets[2])->Value < Settings.ISOLevel;

	if (EnteringYEdge || ExitingYEdge)
	{
		FVector vertex0, vertex1, vertex2, vertex3;

		if (!bUseSharedPoints)
		{
			CalculateDualPoint(VoxelCoordinates, GetDualPointCode(VoxelCoordinates, 256), vertex0);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[4], GetDualPointCode(VoxelCoordinates - DMCOffSets[4], 2048), vertex1);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[5], GetDualPointCode(VoxelCoordinates - DMCOffSets[5], 1024), vertex2);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[1], GetDualPointCode(VoxelCoordinates - DMCOffSets[1], 512), vertex3);
		}

		const int32& index0 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates, 256) : AddVertex(vertex0);
		const int32& index1 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[4], 2048) : AddVertex(vertex1);
		const int32& index2 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[5], 1024) : AddVertex(vertex2);
		const int32& index3 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[1], 512) : AddVertex(vertex3);

		if (ExitingYEdge)
		{
			AddTriangle(index0, index3, index1);
			AddTriangle(index3, index2, index1);
		}
		else {
			AddTriangle(index0, index1, index3);
			AddTriangle(index1, index2, index3);
		}
	}

	//Z Edge quad

	const bool EnteringZEdge = GetDensityPoint(VoxelCoordinates)->Value < Settings.ISOLevel&& GetDensityPoint(VoxelCoordinates + DMCOffSets[4])->Value >= Settings.ISOLevel;
	const bool ExitingZEdge = GetDensityPoint(VoxelCoordinates)->Value >= Settings.ISOLevel && GetDensityPoint(VoxelCoordinates + DMCOffSets[4])->Value < Settings.ISOLevel;

	if (EnteringZEdge || ExitingZEdge)
	{
		FVector vertex0, vertex1, vertex2, vertex3;
		if (!bUseSharedPoints)
		{
			CalculateDualPoint(VoxelCoordinates, GetDualPointCode(VoxelCoordinates, 8), vertex0);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[1], GetDualPointCode(VoxelCoordinates - DMCOffSets[1], 2), vertex1);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[3], GetDualPointCode(VoxelCoordinates - DMCOffSets[3], 32), vertex2);
			CalculateDualPoint(VoxelCoordinates - DMCOffSets[2], GetDualPointCode(VoxelCoordinates - DMCOffSets[2], 128), vertex3);
		}

		const int32& index0 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates, 8) : AddVertex(vertex0);
		const int32& index1 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[1], 2) : AddVertex(vertex1);
		const int32& index2 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[3], 32) : AddVertex(vertex2);
		const int32& index3 = bUseSharedPoints ? GetSharedDualPointIndex(VoxelCoordinates - DMCOffSets[2], 128) : AddVertex(vertex3);

		if (ExitingZEdge)
		{
			AddTriangle(index0, index3, index1);
			AddTriangle(index3, index2, index1);

		}
		else {
			AddTriangle(index0, index1, index3);
			AddTriangle(index1, index2, index3);
		}
	}
}

void UGenerateMarchingCubesTask::GenerateMeshDataForVoxelCoordinates_MC(const FIntVector& VoxelCoordinates)
{
	FIntVector CellVertices[8];
	float GridCell[8];

	for (int32 i = 0; i < 8; ++i)
	{
		CellVertices[i] = VoxelCoordinates + MCOffSets[i];
		GridCell[i] = GetDensityPoint(CellVertices[i])->Value;
	}

	int32 cubeindex = 0;
	FVector vertlist[12];

	if (GridCell[0] >= Settings.ISOLevel) cubeindex |= 1;
	if (GridCell[1] >= Settings.ISOLevel) cubeindex |= 2;
	if (GridCell[2] >= Settings.ISOLevel) cubeindex |= 4;
	if (GridCell[3] >= Settings.ISOLevel) cubeindex |= 8;
	if (GridCell[4] >= Settings.ISOLevel) cubeindex |= 16;
	if (GridCell[5] >= Settings.ISOLevel) cubeindex |= 32;
	if (GridCell[6] >= Settings.ISOLevel) cubeindex |= 64;
	if (GridCell[7] >= Settings.ISOLevel) cubeindex |= 128;

	if (edgeTable[cubeindex] == 0)
		return;

	if (edgeTable[cubeindex] & 1)
		vertlist[0] = VertexInterp(FVector(CellVertices[0]), FVector(CellVertices[1]), GridCell[0], GridCell[1]);
	if (edgeTable[cubeindex] & 2)
		vertlist[1] = VertexInterp(FVector(CellVertices[1]), FVector(CellVertices[2]), GridCell[1], GridCell[2]);
	if (edgeTable[cubeindex] & 4)
		vertlist[2] = VertexInterp(FVector(CellVertices[2]), FVector(CellVertices[3]), GridCell[2], GridCell[3]);
	if (edgeTable[cubeindex] & 8)
		vertlist[3] = VertexInterp(FVector(CellVertices[3]), FVector(CellVertices[0]), GridCell[3], GridCell[0]);
	if (edgeTable[cubeindex] & 16)
		vertlist[4] = VertexInterp(FVector(CellVertices[4]), FVector(CellVertices[5]), GridCell[4], GridCell[5]);
	if (edgeTable[cubeindex] & 32)
		vertlist[5] = VertexInterp(FVector(CellVertices[5]), FVector(CellVertices[6]), GridCell[5], GridCell[6]);
	if (edgeTable[cubeindex] & 64)
		vertlist[6] = VertexInterp(FVector(CellVertices[6]), FVector(CellVertices[7]), GridCell[6], GridCell[7]);
	if (edgeTable[cubeindex] & 128)
		vertlist[7] = VertexInterp(FVector(CellVertices[7]), FVector(CellVertices[4]), GridCell[7], GridCell[4]);
	if (edgeTable[cubeindex] & 256)
		vertlist[8] = VertexInterp(FVector(CellVertices[0]), FVector(CellVertices[4]), GridCell[0], GridCell[4]);
	if (edgeTable[cubeindex] & 512)
		vertlist[9] = VertexInterp(FVector(CellVertices[1]), FVector(CellVertices[5]), GridCell[1], GridCell[5]);
	if (edgeTable[cubeindex] & 1024)
		vertlist[10] = VertexInterp(FVector(CellVertices[2]), FVector(CellVertices[6]), GridCell[2], GridCell[6]);
	if (edgeTable[cubeindex] & 2048)
		vertlist[11] = VertexInterp(FVector(CellVertices[3]), FVector(CellVertices[7]), GridCell[3], GridCell[7]);

	TMap<int8, int32> CubePointMap;

	for (uint8 i = 0; i < 12; i++)
	{
		if (edgeTable[cubeindex] & (int32)FMath::Pow(2, i))
		{
			if (!bUseSharedPoints)
			{
				const int32 NewIndex = AddVertex(vertlist[i]);
				CubePointMap.Add(i, NewIndex);
			}
			else
			{
				bool bExistingIndex = MCPointMap.Contains(vertlist[i]);
				if (!bExistingIndex)
				{
					const int32 NewIndex = AddVertex(vertlist[i]);
					CubePointMap.Add(i, NewIndex);
					MCPointMap.Add(vertlist[i], NewIndex);
				}
				else
				{
					CubePointMap.Add(i, MCPointMap[vertlist[i]]);
				}
			}
		}
	}

	for (uint8 i = 0; i < 14; i += 3)
	{
		int32 first = triTable[cubeindex][i + 2];
		if (first != -1)
		{
			int32 second = triTable[cubeindex][i + 1];
			int32 third = triTable[cubeindex][i];
			AddTriangle(CubePointMap[third], CubePointMap[second], CubePointMap[first]);
		}
		else break;
	}
}

void UGenerateMarchingCubesTask::GenerateMeshDataForVoxelCoordinates_MT(const FIntVector& VoxelCoordinates)
{
	FVector CellVertices[8];
	float GridCell[8];

	for (int32 i = 0; i < 8; ++i)
	{
		CellVertices[i] = FVector(VoxelCoordinates) + FVector(MCOffSets[i]);
		GridCell[i] = GetDensityPoint(VoxelCoordinates + MCOffSets[i])->Value;
	}

	auto Add = [this](const FVector& V1, const FVector& V2, const FVector& V3)->void
	{
		if (!bUseSharedPoints)
		{
			AddTriangle(AddVertex(V1), AddVertex(V2), AddVertex(V3));
		}
		else
		{
			if (!MCPointMap.Contains(V1))
			{
				MCPointMap.Add(V1, AddVertex(V1));
			}
			if (!MCPointMap.Contains(V2))
			{
				MCPointMap.Add(V2, AddVertex(V2));
			}
			if (!MCPointMap.Contains(V3))
			{
				MCPointMap.Add(V3, AddVertex(V3));
			}
			AddTriangle(MCPointMap[V1], MCPointMap[V2], MCPointMap[V3]);
		}
	};


	for (uint8 i = 0; i < 6; ++i)
	{
		int32 TriIdx = 0;

		for (uint8 x = 0; x < 4; ++x)
		{
			if (GridCell[TetraList[i][x]] >= Settings.ISOLevel)
			{
				TriIdx |= (1 << x);
			}
		}

		switch (TriIdx)
		{

			// we don't do anything if everyone is inside or outside
		case 0x00:
		case 0x0F:
			break;

			// only vert 0 is inside
		case 0x01:
			Add(
				VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][0]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][0]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][0]], GridCell[TetraList[i][2]])
			);
			break;

			// only vert 1 is inside
		case 0x02:
			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][1]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][1]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
			);
			break;

			// only vert 2 is inside
		case 0x04:
			Add(
				VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][2]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][2]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][2]], GridCell[TetraList[i][1]])
			);
			break;

			// only vert 3 is inside
		case 0x08:
			Add(
				VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][3]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][3]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
			);
			break;

			// verts 0, 1 are inside
		case 0x03:
			Add(
				VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][2]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
			);

			Add(
				VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][2]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][2]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
			);
			break;

			// verts 0, 2 are inside
		case 0x05:
			Add(
				VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][1]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][1]], GridCell[TetraList[i][0]])
			);

			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][1]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][2]], GridCell[TetraList[i][3]])
			);
			break;

			// verts 0, 3 are inside
		case 0x09:
			Add(
				VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][0]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][0]], GridCell[TetraList[i][2]])
			);

			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][3]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][0]], GridCell[TetraList[i][2]])
			);
			break;

			// verts 1, 2 are inside
		case 0x06:
			Add(
				VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][0]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][0]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
			);

			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][0]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][3]], GridCell[TetraList[i][2]])
			);
			break;

			// verts 2, 3 are inside
		case 0x0C:
			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][2]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
			);

			Add(
				VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][2]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][2]], GridCell[TetraList[i][1]])
			);
			break;

			// verts 1, 3 are inside
		case 0x0A:
			Add(
				VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][1]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][1]], GridCell[TetraList[i][2]])
			);

			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][1]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][2]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
			);
			break;

			// verts 0, 1, 2 are inside
		case 0x07:
			Add(
				VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][3]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][3]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][3]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][3]], GridCell[TetraList[i][1]])
			);
			break;

			// verts 0, 1, 3 are inside
		case 0x0B:
			Add(
				VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][2]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][2]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][2]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][2]], GridCell[TetraList[i][0]])
			);
			break;

			// verts 0, 2, 3 are inside
		case 0x0D:
			Add(
				VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][0]], GridCell[TetraList[i][1]], GridCell[TetraList[i][0]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][1]], GridCell[TetraList[i][3]])
				, VertexInterp(CellVertices[TetraList[i][1]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][1]], GridCell[TetraList[i][2]])
			);
			break;

			// verts 1, 2, 3 are inside
		case 0x0E:
			Add(
				VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][1]], GridCell[TetraList[i][0]], GridCell[TetraList[i][1]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][2]], GridCell[TetraList[i][0]], GridCell[TetraList[i][2]])
				, VertexInterp(CellVertices[TetraList[i][0]], CellVertices[TetraList[i][3]], GridCell[TetraList[i][0]], GridCell[TetraList[i][3]])
			);
			break;

		default:
			break;
		}
	}
}

uint8 UGenerateMarchingCubesTask::GetCellCode(const FIntVector& VoxelCoordinates)
{
	uint8 CellCode = 0;

	for (uint8 i = 0; i < 8; ++i)
	{
		if (GetDensityPoint(VoxelCoordinates + DMCOffSets[i])->Value >= Settings.ISOLevel)
		{
			CellCode |= (1 << i);
		}
	}
	return CellCode;
}


uint16 UGenerateMarchingCubesTask::GetDualPointCode(const FIntVector& VoxelCoordinates, const int32& Edge)
{
	uint8 CubeCode = GetCellCode(VoxelCoordinates);

	if (bForceManifold)
	{
		const uint8 Direction = ProblematicConfigs[CubeCode];
		if (Direction != 255)
		{
			FIntVector NeighborCoords = VoxelCoordinates;
			const uint32 Dim = Direction >> 1;
			int32 Delta = (Direction & 1) == 1 ? 1 : -1;
			NeighborCoords[Dim] += Delta;
			if (NeighborCoords[Dim] >= 1 && NeighborCoords[Dim] < Settings.Units[Dim] - 1)
			{
				uint8 neighborCubeCode = GetCellCode(NeighborCoords);
				if (ProblematicConfigs[neighborCubeCode] != 255)
				{
					CubeCode ^= 0xff;
				}
			}
		}
	}

	for (int32 i = 0; i < 4; ++i)
	{
		if (DualPointsList[CubeCode][i] & Edge)
		{
			return DualPointsList[CubeCode][i];
		}
	}
	return 0;
}

void UGenerateMarchingCubesTask::CalculateDualPoint(const FIntVector& VoxelCoordinates, const uint16& PointCode, FVector& OutCoordinates)
{

	int32 FoundIntersectionPoints = 0;
	OutCoordinates = FVector::ZeroVector;

	float GridCell[8];
	for (uint8 i = 0; i < 8; ++i)
	{
		GridCell[i] = GetDensityPoint(VoxelCoordinates + DMCOffSets[i])->Value;
	}


	if (PointCode & 1) //1
	{
		OutCoordinates.X += (Settings.ISOLevel - GridCell[0]) / (GridCell[1] - GridCell[0]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 2) //2
	{
		OutCoordinates.X += 1.0f;
		OutCoordinates.Z += (Settings.ISOLevel - GridCell[1]) / (GridCell[5] - GridCell[1]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 4)//3
	{
		OutCoordinates.X += (Settings.ISOLevel - GridCell[4]) / (GridCell[5] - GridCell[4]);
		OutCoordinates.Z += 1.0f;
		FoundIntersectionPoints++;
	}

	if (PointCode & 8)//4
	{
		OutCoordinates.Z += (Settings.ISOLevel - GridCell[0]) / (GridCell[4] - GridCell[0]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 16)//5
	{
		OutCoordinates.X += (Settings.ISOLevel - GridCell[2]) / (GridCell[3] - GridCell[2]);
		OutCoordinates.Y += 1.0f;
		FoundIntersectionPoints++;
	}

	if (PointCode & 32)//6
	{
		OutCoordinates.X += 1.0f;
		OutCoordinates.Y += 1.0f;
		OutCoordinates.Z += (Settings.ISOLevel - GridCell[3]) / (GridCell[7] - GridCell[3]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 64)//7
	{
		OutCoordinates.X += (Settings.ISOLevel - GridCell[6]) / (GridCell[7] - GridCell[6]);
		OutCoordinates.Y += 1.0f;
		OutCoordinates.Z += 1.0f;
		FoundIntersectionPoints++;
	}

	if (PointCode & 128)//8
	{
		OutCoordinates.Y += 1.0f;
		OutCoordinates.Z += (Settings.ISOLevel - GridCell[2]) / (GridCell[6] - GridCell[2]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 256)//9
	{
		OutCoordinates.Y += (Settings.ISOLevel - GridCell[0]) / (GridCell[2] - GridCell[0]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 512)//10
	{
		OutCoordinates.X += 1.0f;
		OutCoordinates.Y += (Settings.ISOLevel - GridCell[1]) / (GridCell[3] - GridCell[1]);
		FoundIntersectionPoints++;
	}

	if (PointCode & 1024)//11
	{
		OutCoordinates.X += 1.0f;
		OutCoordinates.Y += (Settings.ISOLevel - GridCell[5]) / (GridCell[7] - GridCell[5]);
		OutCoordinates.Z += 1.0f;
		FoundIntersectionPoints++;
	}

	if (PointCode & 2048)//12
	{
		OutCoordinates.Y += (Settings.ISOLevel - GridCell[4]) / (GridCell[6] - GridCell[4]);
		OutCoordinates.Z += 1.0f;
		FoundIntersectionPoints++;
	}

	OutCoordinates *= (1.0f / (float)FoundIntersectionPoints);
	OutCoordinates += FVector(VoxelCoordinates);
}

int32 UGenerateMarchingCubesTask::GetSharedDualPointIndex(const FIntVector& VoxelCoordinates, const int32& Edge)
{
	const FMarchingCubesDualPointKey Key(VoxelCoordinates, GetDualPointCode(VoxelCoordinates, Edge));
	if (PointMap.Contains(Key))
	{
		return PointMap[Key];
	}
	else {
		FVector NewVoxelCoordinates;
		CalculateDualPoint(VoxelCoordinates, Key.PointCode, NewVoxelCoordinates);
		const int32 NewVertID = AddVertex(NewVoxelCoordinates);
		PointMap.Add(Key, NewVertID);
		return NewVertID;
	}
}

FVector UGenerateMarchingCubesTask::VertexInterp(const FVector& p1, const FVector& p2, const float& valp1, const float& valp2) const
{
	if ((double)FMath::Abs(Settings.ISOLevel - valp1) < 0.00001)
	{
		return p1;
	}

	if ((double)FMath::Abs(Settings.ISOLevel - valp2) < 0.00001)
	{
		return p2;
	}

	if ((double)FMath::Abs(valp2 - valp1) < 0.00001)
	{
		return p1;
	}

	if (valp2 == valp1)
	{
		return p1;
	}

	return p1 + ((Settings.ISOLevel - valp1) / (valp2 - valp1)) * (p2 - p1);
}

void UGenerateMarchingCubesTask::GetNormal(const FVector& VoxelCoordinates, FVector& ReturnValue)
{
	if (NormalType == EMarchingCubesNormal::GradientAdvanced)
	{
		FVector BoundDensity[3];
		FIntVector UpperBounds;
		FVector Delta;
		int32 Idx = 0;
		for (int32 Dim = 0; Dim < 3; ++Dim)
		{
			UpperBounds[Dim] = (int32)(VoxelCoordinates[Dim] + 0.5f);
			Delta[Dim] = VoxelCoordinates[Dim] + 0.5f - (float)UpperBounds[Dim];
			for (int32 Offset = 0; Offset < 3; ++Offset)
			{
				if (IsCanceled())
				{
					return;
				}
				FIntVector Coord = FIntVector(VoxelCoordinates);
				Coord[Dim] += (Offset - 1);
				BoundDensity[Dim][Offset] = GetDensityPoint(Coord)->Value;
				++Idx;

			}
		}

		//x
		ReturnValue.X = (BoundDensity[0].Y - BoundDensity[0].X) * (1.0f - Delta.X) + (BoundDensity[0].Z - BoundDensity[0].Y) * Delta.X; // lerp
		// y
		ReturnValue.Y = (BoundDensity[1].Y - BoundDensity[1].X) * (1.0f - Delta.Y) + (BoundDensity[1].Z - BoundDensity[1].Y) * Delta.Y; // lerp
		// z
		ReturnValue.Z = (BoundDensity[2].Y - BoundDensity[2].X) * (1.0f - Delta.Z) + (BoundDensity[2].Z - BoundDensity[2].Y) * Delta.Z; // lerp

		ReturnValue = Settings.bInverted ? ReturnValue : -ReturnValue;
		ReturnValue.Normalize();
	}
	else {
		const float dx = GetDensityPoint(FIntVector(VoxelCoordinates.X + 1, VoxelCoordinates.Y, VoxelCoordinates.Z))->Value - GetDensityPoint(FIntVector(VoxelCoordinates.X - 1, VoxelCoordinates.Y, VoxelCoordinates.Z))->Value;
		const float dy = GetDensityPoint(FIntVector(VoxelCoordinates.X, VoxelCoordinates.Y + 1, VoxelCoordinates.Z))->Value - GetDensityPoint(FIntVector(VoxelCoordinates.X, VoxelCoordinates.Y - 1, VoxelCoordinates.Z))->Value;
		const float dz = GetDensityPoint(FIntVector(VoxelCoordinates.X, VoxelCoordinates.Y, VoxelCoordinates.Z + 1))->Value - GetDensityPoint(FIntVector(VoxelCoordinates.X, VoxelCoordinates.Y, VoxelCoordinates.Z - 1))->Value;

		ReturnValue = FVector(dx, dy, dz);
		ReturnValue = Settings.bInverted ? ReturnValue : -ReturnValue;
		ReturnValue.Normalize();
	}

}

void UGenerateMarchingCubesTask::MakePerpVectors(const FVector& Normal, FVector& OutPerp1, FVector& OutPerp2) const
{
	// Duff et al method, from https://graphics.pixar.com/library/OrthonormalB/paper.pdf
	if (Normal.Z < 0.)
	{
		const float A = 1.0f / (1.0f - Normal.Z);
		const float B = Normal.X * Normal.Y * A;
		OutPerp1.X = 1.0f - Normal.X * Normal.X * A;
		OutPerp1.Y = -B;
		OutPerp1.Z = Normal.X;
	}
	else
	{
		const float A = 1.0f / (1.0f + Normal.Z);
		const float B = -Normal.X * Normal.Y * A;
		OutPerp1.X = 1.f - Normal.X * Normal.X * A;
		OutPerp1.Y = B;
		OutPerp1.Z = -Normal.X;
	}
}