// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiThreadTask.h"
#include "PixelReaderTask.h"
#include "MultiTaskThreadPool.h"
#include "ProceduralMeshComponent.h"
#include "DelaunayTriangulation2DTask.h"
#include "MultiTask2VoxelLibrary.h"
#include "MultiThreadTaskLibrary.generated.h"

class UTexture;
class UMultiTaskMutex;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class MULTITASK2_API UMultiThreadTaskLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	* Execute a task on a separate thread.
	* @param Class			Task Class to be executed.
	* @param Task			Running Task.
	* @param ExecutionType	Execution type.
	* @param ThreadPool		Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Single Thread Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static void DoSingleThreadTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiThreadTask> Class, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Execute a task on a separate thread.
	* @param Task			Running Task.
	* @param ExecutionType	Execution type.
	* @param ThreadPool		Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Single Thread Task 2", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static void DoSingleThreadTask2(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UMultiTaskBase*& Task, EMultiTask2BranchesWithBody& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);


	/**
	* Execute a task on multiple threads.
	* Important: Execution finish order is not guaranteed.
	* @param Class			Task Class to be executed.
	* @param Count			Amount of Tasks to be executed.
	* @param Tasks			Array of running Tasks.
	* @param ExecutionType	Execution type.
	* @param ThreadPool		Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Multiple Thread Task", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static void DoMultiThreadTask(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TSubclassOf<class UMultiThreadTask> Class, int32 Count, TArray<UMultiTaskBase*>& Tasks, EMultiTask2BranchesNoCancel& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Spawn HISM Instances using parallel(optional) multi-threading.
	* @param HISM						HISM Component.
	* @param Transforms					Transforms for the instances that are going to be spawned.
	* @param Chunks						Split the work into the specified amount of chunks. Values higher than 1 enable parallelism.
	* @param bWorldSpace				Whether the provided transforms are in World Space.
	* @param bCreatePhysicsBodies		With Collision enabled, Instance Physics Bodies get automatically updated on Game Thread.
	* @param bCreateInternalDataCopies	Whether a copy of the input array should be created internally. Keeping it disabled increases performance but the input array shouldn't change while the Task is running
	* @param Task						Running Task.
	* @param ExecutionType				Execution type.
	* @param ThreadPool					Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Spawn Instances", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", Chunks = "1", bCreatePhysicsBodies = "true"), Category = "Multi Task 2|Threading")
		static void DoSpawnInstances(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, const TArray<FTransform>& Transforms, int32 Chunks, bool bWorldSpace, bool bCreatePhysicsBodies, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, TArray<int32>& NewInstances, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Update HISM Instances and Custom Data using parallel(optional) multi-threading.
	* @param HISM						HISM Component.
	* @param StartIndex					The starting index of the instances to update.
	* @param Transforms					Transforms for the instances that are going to be spawned (optional).
	* @param CustomData					Float Custom Data Array (optional).
	* @param Chunks						Split the work into the specified amount of chunks. Values higher than 1 enable parallelism.
	* @param bWorldSpace				Whether the provided transforms are in World Space.
	* @param bTeleport					Whether or not the instances should be moved normaly, or teleported (moved instantly, ignoring velocity).
	* @param bUpdatePhysicsBodies		With Collision enabled, Instance Physics Bodies get automatically updated on Game Thread.
	* @param bMarkRenderStateDirty		If true, the change should be visible immediately.
	* @param bCreateInternalDataCopies	Whether a copy of the input arrays should be created internally. Keeping it disabled increases performance but the input arrays shouldn't change while the Task is running 
	* @param Task						Running Task.
	* @param ExecutionType				Execution type.
	* @param ThreadPool					Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Update Instances", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", Chunks = "1", bUpdatePhysicsBodies = "true", AutoCreateRefTerm = "Transforms, CustomData"), Category = "Multi Task 2|Threading")
		static void DoUpdateInstances(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<float>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Update HISM Instances and Custom Data using parallel(optional) multi-threading.
	* @param HISM						HISM Component.
	* @param StartIndex					The starting index of the instances to update.
	* @param Transforms					Transforms for the instances that are going to be spawned (optional).
	* @param CustomData					FVector2D Custom Data Array (optional).
	* @param Chunks						Split the work into the specified amount of chunks. Values higher than 1 enable parallelism.
	* @param bWorldSpace				Whether the provided transforms are in World Space.
	* @param bTeleport					Whether or not the instances should be moved normaly, or teleported (moved instantly, ignoring velocity).
	* @param bUpdatePhysicsBodies		With Collision enabled, Instance Physics Bodies get automatically updated on Game Thread.
	* @param bMarkRenderStateDirty		If true, the change should be visible immediately.
	* @param bCreateInternalDataCopies	Whether a copy of the input arrays should be created internally. Keeping it disabled increases performance but the input arrays shouldn't change while the Task is running
	* @param Task						Running Task.
	* @param ExecutionType				Execution type.
	* @param ThreadPool					Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Update Instances 2", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", Chunks = "1", bUpdatePhysicsBodies = "true", AutoCreateRefTerm = "Transforms, CustomData"), Category = "Multi Task 2|Threading")
		static void DoUpdateInstances2(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<FVector2D>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Update HISM Instances and Custom Data using parallel(optional) multi-threading.
	* @param HISM						HISM Component.
	* @param StartIndex					The starting index of the instances to update.
	* @param Transforms					Transforms for the instances that are going to be spawned (optional).
	* @param CustomData					FVector Custom Data Array (optional).
	* @param Chunks						Split the work into the specified amount of chunks. Values higher than 1 enable parallelism.
	* @param bWorldSpace				Whether the provided transforms are in World Space.
	* @param bTeleport					Whether or not the instances should be moved normaly, or teleported (moved instantly, ignoring velocity).
	* @param bUpdatePhysicsBodies		With Collision enabled, Instance Physics Bodies get automatically updated on Game Thread.
	* @param bMarkRenderStateDirty		If true, the change should be visible immediately.
	* @param bCreateInternalDataCopies	Whether a copy of the input arrays should be created internally. Keeping it disabled increases performance but the input arrays shouldn't change while the Task is running
	* @param Task						Running Task.
	* @param ExecutionType				Execution type.
	* @param ThreadPool					Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Update Instances 3", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", Chunks = "1", bUpdatePhysicsBodies = "true", AutoCreateRefTerm = "Transforms, CustomData"), Category = "Multi Task 2|Threading")
		static void DoUpdateInstances3(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<FVector>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Update HISM Instances and Custom Data using parallel(optional) multi-threading.
	* @param HISM						HISM Component.
	* @param StartIndex					The starting index of the instances to update.
	* @param Transforms					Transforms for the instances that are going to be spawned (optional).
	* @param CustomData					FVector4 Custom Data Array (optional).
	* @param Chunks						Split the work into the specified amount of chunks. Values higher than 1 enable parallelism.
	* @param bWorldSpace				Whether the provided transforms are in World Space.
	* @param bTeleport					Whether or not the instances should be moved normaly, or teleported (moved instantly, ignoring velocity).
	* @param bUpdatePhysicsBodies		With Collision enabled, Instance Physics Bodies get automatically updated on Game Thread.
	* @param bMarkRenderStateDirty		If true, the change should be visible immediately.
	* @param bCreateInternalDataCopies	Whether a copy of the input arrays should be created internally. Keeping it disabled increases performance but the input arrays shouldn't change while the Task is running
	* @param Task						Running Task.
	* @param ExecutionType				Execution type.
	* @param ThreadPool					Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Update Instances 4", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", Chunks = "1", bUpdatePhysicsBodies = "true", AutoCreateRefTerm = "Transforms, CustomData"), Category = "Multi Task 2|Threading")
		static void DoUpdateInstances4(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UHierarchicalInstancedStaticMeshComponent* HISM, int32 StartIndex, const TArray<FTransform>& Transforms, const TArray<FVector4>& CustomData, int32 Chunks, bool bWorldSpace, bool bTeleport, bool bUpdatePhysicsBodies, bool bMarkRenderStateDirty, bool bCreateInternalDataCopies, UMultiTaskBase*& Task, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	* Triangulate an array of 2D points.
	* @param Points						Array of 2D vector points.
	* @param Triangles					Generated Delaunay triangles.
	* @param Task						Running Task.
	* @param ExecutionType				Execution type.
	* @param ThreadPool					Thread Pool to be used. If Null and ExecutionType is ThreadPool, then the engine's internal Thread Pool will be used.
	*/
	UFUNCTION(BlueprintCallable, Meta = (Latent, DisplayName = "Do Delaunay Triangulation 2D", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static void DoDelaunayTriangulation2D(UObject* WorldContextObject, FLatentActionInfo LatentInfo, UPARAM(ref)TArray<FVector2D>& Points, UMultiTaskBase*& Task, TArray<FMultiTask2Delaunay2DTriangle>& Triangles, EMultiTask2Branches& Out, ETaskExecutionType ExecutionType = ETaskExecutionType::ThreadPool, UMultiTaskThreadPool* ThreadPool = nullptr);

	/**
	 * Creates the thread pool with the specified number of threads
	 *
	 * @param InNumQueuedThreads Specifies the number of threads to use in the pool
	 * @param StackSize The size of stack the threads in the pool need (32K default)
	 * @param ThreadPriority priority of new pool thread
	 * @param Name optional name for the pool to be used for instrumentation
	 * @return ThreadPool Object
	 */
	UFUNCTION(BlueprintCallable, Meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static UMultiTaskThreadPool* CreateThreadPool(UObject* WorldContextObject, int32 NumQueuedThreads = 1, int32 StackSize = 32768, EMultiTaskThreadPriority ThreadPriority = EMultiTaskThreadPriority::Normal, FString Name = "UnknownThreadPool");
	/**
	 * Attempts to destroy a Thread Pool immediately.
	 *
	 * @param ThreadPool Thread pool to be destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Threading")
		static void DestroyThreadPoolImmediately(UMultiTaskThreadPool* ThreadPool);

	/**
	 * Creates a Mutex Object.
	 */
	UFUNCTION(BlueprintCallable, Meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Multi Task 2|Threading")
		static UMultiTaskMutex* CreateMutex(UObject* WorldContextObject);

	/**
	* Put a thread to sleep for the amount of seconds.
	* @param Seconds	Amount of seconds to sleep.
	* This function doesn't work on Game Thread.
	*/
	UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Threading|Utilities")
		static void Sleep(float Seconds);

	/**
	 * Return the number of CPU cores
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Threading|Utilities")
		static int32 GetNumberOfCores();

	/**
	* Return the number of CPU cores including Hyperthreads
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Threading|Utilities")
		static int32 GetNumberOfCoresIncludingHyperthreads();

	/**
	* Return whether it's called from game thread or not
	*/
	UFUNCTION(BlueprintPure, Category = "Multi Task 2|Threading|Utilities")
		static bool IsInGameThread();
};

