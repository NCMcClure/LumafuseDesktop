// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "MultiTask2TraceLibrary.generated.h"

class AActor;

UCLASS()
class MULTITASK2_API UMultiTask2TraceLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    /**
    * Does a collision trace along the given line and returns the first blocking hit encountered.
    * This trace finds the objects that RESPONDS to the given TraceChannel
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	        World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param TraceChannel
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit		Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "LineTraceByChannelThreadSafe", Keywords = "raycast"))
        static bool LineTraceSingleThreadSafe(UObject* World, FVector Start, FVector End,
            ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit);

    /**
    * Does a collision trace along the given line and returns all hits encountered up to and including the first blocking hit.
    * This trace finds the objects that RESPOND to the given TraceChannel
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param TraceChannel	The channel to trace
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit		Properties of the trace hit.
    * @return				True if there was a blocking hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiLineTraceByChannelThreadSafe", Keywords = "raycast"))
        static bool LineTraceMultiThreadSafe(UObject* World, FVector Start, FVector End,
            ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits);

    /**
         * Sweeps a sphere along the given line and returns the first blocking hit encountered.
         * This trace finds the objects that RESPONDS to the given TraceChannel
         * There is no debug drawing and this is THREAD SAFE.
         *
         * @param Start			Start of line segment.
         * @param End			End of line segment.
         * @param Radius		Radius of the sphere to sweep
         * @param TraceChannel
         * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
         * @param OutHit		Properties of the trace hit.
         * @return				True if there was a hit, false otherwise.
         */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "SphereTraceByChannelThreadSafe", Keywords = "sweep"))
        static bool SphereTraceSingleThreadSafe(UObject* World, FVector Start, FVector End, float Radius,
            ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit);

    /**
     * Sweeps a sphere along the given line and returns all hits encountered up to and including the first blocking hit.
     * This trace finds the objects that RESPOND to the given TraceChannel
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the sphere to sweep
     * @param TraceChannel
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
     * @return				True if there was a blocking hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiSphereTraceByChannelThreadSafe", Keywords = "sweep"))
        static bool SphereTraceMultiThreadSafe(UObject* World, FVector Start, FVector End, float Radius,
            ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits);

    /**
    * Sweeps a box along the given line and returns the first blocking hit encountered.
    * This trace finds the objects that RESPONDS to the given TraceChannel
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param HalfSize	    Distance from the center of box along each axis
    * @param Orientation	Orientation of the box
    * @param TraceChannel
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit			Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "BoxTraceByChannelThreadSafe", Keywords = "sweep"))
        static bool BoxTraceSingleThreadSafe(UObject* World, FVector Start, FVector End, FVector HalfSize,
            FRotator Orientation, ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit);

    /**
    * Sweeps a box along the given line and returns all hits encountered.
    * This trace finds the objects that RESPONDS to the given TraceChannel
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param HalfSize	    Distance from the center of box along each axis
    * @param Orientation	Orientation of the box
    * @param TraceChannel
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHits		A list of hits, sorted along the trace from start to finish. The blocking hit will be the last hit, if there was one.
    * @return				True if there was a blocking hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiBoxTraceByChannelThreadSafe", Keywords = "sweep"))
        static bool BoxTraceMultiThreadSafe(UObject* World, FVector Start, FVector End, FVector HalfSize,
            FRotator Orientation, ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits);


    /**
     * Sweeps a capsule along the given line and returns the first blocking hit encountered.
     * This trace finds the objects that RESPOND to the given TraceChannel
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the capsule to sweep
     * @param HalfHeight	Distance from center of capsule to tip of hemisphere endcap.
     * @param TraceChannel
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHit		Properties of the trace hit.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "CapsuleTraceByChannelThreadSafe", Keywords = "sweep"))
        static bool CapsuleTraceSingleThreadSafe(UObject* World, FVector Start, FVector End, float Radius,
            float HalfHeight, ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit);

    /**
     * Sweeps a capsule along the given line and returns all hits encountered up to and including the first blocking hit.
     * This trace finds the objects that RESPOND to the given TraceChannel
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the capsule to sweep
     * @param HalfHeight	Distance from center of capsule to tip of hemisphere endcap.
     * @param TraceChannel
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
     * @return				True if there was a blocking hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiCapsuleTraceByChannelThreadSafe", Keywords = "sweep"))
        static bool CapsuleTraceMultiThreadSafe(UObject* World, FVector Start, FVector End, float Radius,
            float HalfHeight, ETraceTypeQuery TraceChannel, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits);

    /**
     * Does a collision trace along the given line and returns the first hit encountered.
     * This only finds objects that are of a type specified by ObjectTypes.
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param ObjectTypes	Array of Object Types to trace
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHit		Properties of the trace hit.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "LineTraceForObjectsThreadSafe", Keywords = "raycast"))
        static bool LineTraceSingleForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            FHitResult& OutHit);

    /**
     * Does a collision trace along the given line and returns all hits encountered.
     * This only finds objects that are of a type specified by ObjectTypes.
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param ObjectTypes	Array of Object Types to trace
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHit		Properties of the trace hit.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiLineTraceForObjectsThreadSafe", Keywords = "raycast"))
        static bool LineTraceMultiForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            TArray<FHitResult>& OutHits);

    /**
     * Sweeps a sphere along the given line and returns the first hit encountered.
     * This only finds objects that are of a type specified by ObjectTypes.
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the sphere to sweep
     * @param ObjectTypes	Array of Object Types to trace
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHit		Properties of the trace hit.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "SphereTraceForObjectsThreadSafe", Keywords = "sweep"))
        static bool SphereTraceSingleForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            FHitResult& OutHit);

    /**
     * Sweeps a sphere along the given line and returns all hits encountered.
     * This only finds objects that are of a type specified by ObjectTypes.
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the sphere to sweep
     * @param ObjectTypes	Array of Object Types to trace
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiSphereTraceForObjectsThreadSafe", Keywords = "sweep"))
        static bool SphereTraceMultiForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            TArray<FHitResult>& OutHits);


    /**
    * Sweeps a box along the given line and returns the first hit encountered.
    * This only finds objects that are of a type specified by ObjectTypes.
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param Orientation
    * @param HalfSize		Radius of the sphere to sweep
    * @param ObjectTypes	Array of Object Types to trace
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit			Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "BoxTraceForObjectsThreadSafe", Keywords = "sweep"))
        static bool BoxTraceSingleForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            FVector HalfSize, FRotator Orientation,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            FHitResult& OutHit);


    /**
    * Sweeps a box along the given line and returns all hits encountered.
    * This only finds objects that are of a type specified by ObjectTypes.
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param Orientation
    * @param HalfSize		Radius of the sphere to sweep
    * @param ObjectTypes	Array of Object Types to trace
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiBoxTraceForObjectsThreadSafe", Keywords = "sweep"))
        static bool BoxTraceMultiForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            FVector HalfSize, FRotator Orientation,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            TArray<FHitResult>& OutHits);

    /**
     * Sweeps a capsule along the given line and returns the first hit encountered.
     * This only finds objects that are of a type specified by ObjectTypes.
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the capsule to sweep
     * @param HalfHeight	Distance from center of capsule to tip of hemisphere endcap.
     * @param ObjectTypes	Array of Object Types to trace
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHit		Properties of the trace hit.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "CapsuleTraceForObjectsThreadSafe", Keywords = "sweep"))
        static bool CapsuleTraceSingleForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius, float HalfHeight,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            FHitResult& OutHit);

    /**
     * Sweeps a capsule along the given line and returns all hits encountered.
     * This only finds objects that are of a type specified by ObjectTypes.
     * There is no debug drawing and this is THREAD SAFE.
     *
     * @param World	World
     * @param Start			Start of line segment.
     * @param End			End of line segment.
     * @param Radius		Radius of the capsule to sweep
     * @param HalfHeight	Distance from center of capsule to tip of hemisphere endcap.
     * @param ObjectTypes	Array of Object Types to trace
     * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
     * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
     * @return				True if there was a hit, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiCapsuleTraceForObjectsThreadSafe", Keywords = "sweep"))
        static bool CapsuleTraceMultiForObjectsThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius, float HalfHeight,
            const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            TArray<FHitResult>& OutHits);

    // BY PROFILE

    /**
    * Trace a ray against the world using a specific profile and return the first blocking hit
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit			Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "LineTraceByProfileThreadSafe", Keywords = "raycast"))
        static bool LineTraceSingleByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            FName ProfileName, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit);

    /**
    *  Trace a ray against the world using a specific profile and return overlapping hits and then first blocking hit
    *  Results are sorted, so a blocking hit (if found) will be the last element of the array
    *  Only the single closest blocking result will be generated, no tests will be done after that
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit		Properties of the trace hit.
    * @return				True if there was a blocking hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiLineTraceByProfileThreadSafe", Keywords = "raycast"))
        static bool LineTraceMultiByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            FName ProfileName, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits);

    /**
    *  Sweep a sphere against the world and return the first blocking hit using a specific profile
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param Radius			Radius of the sphere to sweep
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit			Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "SphereTraceByProfileThreadSafe", Keywords = "sweep"))
        static bool SphereTraceSingleByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius, FName ProfileName, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit);

    /**
    *  Sweep a sphere against the world and return all initial overlaps using a specific profile, then overlapping hits and then first blocking hit
    *  Results are sorted, so a blocking hit (if found) will be the last element of the array
    *  Only the single closest blocking result will be generated, no tests will be done after that
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param Radius		Radius of the sphere to sweep
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
    * @return				True if there was a blocking hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiSphereTraceByProfileThreadSafe", Keywords = "sweep"))
        static bool SphereTraceMultiByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius, FName ProfileName, bool bTraceComplex,
            const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits);

    /**
    *  Sweep a box against the world and return the first blocking hit using a specific profile
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param HalfSize	    Distance from the center of box along each axis
    * @param Orientation	Orientation of the box
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit			Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "BoxTraceByProfileThreadSafe", Keywords = "sweep"))
        static bool BoxTraceSingleByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            FVector HalfSize, FRotator Orientation, FName ProfileName,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            FHitResult& OutHit);

    /**
    *  Sweep a box against the world and return all initial overlaps using a specific profile, then overlapping hits and then first blocking hit
    *  Results are sorted, so a blocking hit (if found) will be the last element of the array
    *  Only the single closest blocking result will be generated, no tests will be done after that
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param HalfSize	    Distance from the center of box along each axis
    * @param Orientation	Orientation of the box
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHits		A list of hits, sorted along the trace from start to finish. The blocking hit will be the last hit, if there was one.
    * @return				True if there was a blocking hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiBoxTraceByProfileThreadSafe", Keywords = "sweep"))
        static bool BoxTraceMultiByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            FVector HalfSize, FRotator Orientation, FName ProfileName,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            TArray<FHitResult>& OutHits);


    /**
    *  Sweep a capsule against the world and return the first blocking hit using a specific profile
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param Radius			Radius of the capsule to sweep
    * @param HalfHeight		Distance from center of capsule to tip of hemisphere endcap.
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHit			Properties of the trace hit.
    * @return				True if there was a hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "CapsuleTraceByProfileThreadSafe", Keywords = "sweep"))
        static bool CapsuleTraceSingleByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius, float HalfHeight, FName ProfileName,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            FHitResult& OutHit);

    /**
    *  Sweep a capsule against the world and return all initial overlaps using a specific profile, then overlapping hits and then first blocking hit
    *  Results are sorted, so a blocking hit (if found) will be the last element of the array
    *  Only the single closest blocking result will be generated, no tests will be done after that
    * There is no debug drawing and this is THREAD SAFE.
    *
    * @param World	World
    * @param Start			Start of line segment.
    * @param End			End of line segment.
    * @param Radius			Radius of the capsule to sweep
    * @param HalfHeight		Distance from center of capsule to tip of hemisphere endcap.
    * @param ProfileName	The 'profile' used to determine which components to hit
    * @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
    * @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
    * @return				True if there was a blocking hit, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "Multi Task 2|Collision", meta = (AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "MultiCapsuleTraceByProfileThreadSafe", Keywords = "sweep"))
        static bool CapsuleTraceMultiByProfileThreadSafe(UObject* World, FVector Start, FVector End,
            float Radius, float HalfHeight, FName ProfileName,
            bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
            TArray<FHitResult>& OutHits);
};