// Copyright 2021 RLoris

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "SortPluginBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

/* Sort */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FIntSortDelegate, const int32&, A, const int32&, B, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FFloatSortDelegate, const float&, A, const float&, B, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FStringSortDelegate, const FString&, A, const FString&, B, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FNameSortDelegate, const FName&, A, const FName&, B, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FVectorSortDelegate, const FVector&, A, const FVector&, B, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FActorSortDelegate, const AActor*, A, const AActor*, B, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FObjectSortDelegate, const UObject*, A, const UObject*, B, bool&, Result);

/* Filter */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FIntFilterDelegate, const int32&, Value, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FFloatFilterDelegate, const float&, Value, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FStringFilterDelegate, const FString&, Value, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FNameFilterDelegate, const FName&, Value, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FVectorFilterDelegate, const FVector&, Value, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FActorFilterDelegate, const AActor*, Value, bool&, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FObjectFilterDelegate, const UObject*, Value, bool&, Result);

template <typename T> class TReverseSortPredicate
{
	public:
		bool operator()(const T A, const T B) const
		{
			return A > B;
		}
};

UCLASS()
class USortPluginBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	public:
		/* Average */
		template <typename T> static float Average(const TArray<T>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "AverageInteger", CompactNodeTitle = "Average", Keywords = "Average plugin Array Integer", ToolTip = "Get the average of an array"), Category = "Average")
		static float AverageInteger(const TArray<int32>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "AverageFloat", CompactNodeTitle = "Average", Keywords = "Average plugin Array Float", ToolTip = "Get the average of an array"), Category = "Average")
		static float AverageFloat(const TArray<float>& Array);
		
		/* Minimum */
		template <typename T> static int32 Minimum(const TArray<T>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "MinimumIntegerIndex", CompactNodeTitle = "MinIdx", Keywords = "Minimum plugin Array Integer Index", ToolTip = "Get the minimum value index of an array"), Category = "Minimum")
		static int32 MinimumIntegerIndex(const TArray<int32>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "MinimumFloatIndex", CompactNodeTitle = "MinIdx", Keywords = "Minimum plugin Array Float Index", ToolTip = "Get the minimum value index of an array"), Category = "Minimum")
		static int32 MinimumFloatIndex(const TArray<float>& Array);
		
		/* Maximum */
		template <typename T> static int32 Maximum(const TArray<T>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "MaximumIntegerIndex", CompactNodeTitle = "MaxIdx", Keywords = "Maximum plugin Array Integer Index", ToolTip = "Get the maximum value index of an array"), Category = "Maximum")
		static int32 MaximumIntegerIndex(const TArray<int32>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "MaximumFloatIndex", CompactNodeTitle = "MaxIdx", Keywords = "Maximum plugin Array Float Index", ToolTip = "Get the maximum value index of an array"), Category = "Maximum")
		static int32 MaximumFloatIndex(const TArray<float>& Array);
		
		/* Normalize */
		template <typename T> static TArray<T> MinMaxNormalization(const TArray<T>& Array, T Min, T Max);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "MinMaxFloatNormalization", CompactNodeTitle = "MinMaxNormalization", Keywords = "MinMaxNormalization plugin Array Float", ToolTip = "Normalize the number of an array between two values"), Category = "Normalization")
		static TArray<float> MinMaxFloatNormalization(const TArray<float>& Array, float Min = 0, float Max = 1);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "MinMaxIntegerNormalization", CompactNodeTitle = "MinMaxNormalization", Keywords = "MinMaxNormalization plugin Array Integer", ToolTip = "Normalize the number of an array between two values"), Category = "Normalization")
		static TArray<int32> MinMaxIntegerNormalization(const TArray<int32>& Array, int32 Min = 0, int32 Max = 100);

		/* Reverse */
		template <typename T> static TArray<T> Reverse(const TArray<T>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseInteger", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array Integer", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<int32> ReverseInteger(const TArray<int32>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseFloat", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array Float", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<float> ReverseFloat(const TArray<float>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseString", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array String", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<FString> ReverseString(const TArray<FString>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseName", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array Name", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<FName> ReverseName(const TArray<FName>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseVector", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array Vector", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<FVector> ReverseVector(const TArray<FVector>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseActor", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array Actor", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<AActor*> ReverseActor(const TArray<AActor*>& Array);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReverseObject", CompactNodeTitle = "Reverse", Keywords = "Reverse plugin Array Object", ToolTip = "Reverse an array by copy"), Category = "Reverse")
		static TArray<UObject*> ReverseObject(const TArray<UObject*>& Array);

		/* Convert */
		UFUNCTION(BlueprintPure, meta = (DisplayName = "SplitString", CompactNodeTitle = "Split", Keywords = "Convert plugin Array String Split Explode Separator", ToolTip = "Convert a string to array"), Category = "Convert")
		static TArray<FString> SplitString(FString String, const FString& Separator, ESearchCase::Type SearchCase, bool RemoveEmptyString = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToIntegerSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array Integer Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<int32> ToIntegerSet(const TArray<int32>& Array) { return TSet<int32>(Array);  };
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToFloatSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array Float Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<float> ToFloatSet(const TArray<float>& Array) { return TSet<float>(Array); };
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToNameSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array Name Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<FName> ToNameSet(const TArray<FName>& Array) { return TSet<FName>(Array); };
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToStringSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array String Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<FString> ToStringSet(const TArray<FString>& Array) { return TSet<FString>(Array); };
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToVectorSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array Vector Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<FVector> ToVectorSet(const TArray<FVector>& Array) { return TSet<FVector>(Array); };
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToActorSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array Actor Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<AActor*> ToActorSet(const TArray<AActor*>& Array) { return TSet<AActor*>(Array); };
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToObjectSet", CompactNodeTitle = "ToSet", Keywords = "Convert plugin Array Object Set", ToolTip = "Convert an array to set"), Category = "Convert")
		static inline TSet<UObject*> ToObjectSet(const TArray<UObject*>& Array) { return TSet<UObject*>(Array); };

		/* Clamp */
		template <typename T> static TArray<T> Clamp(TArray<T>& Array, T Min, T Max);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClampInteger", CompactNodeTitle = "Clamp", Keywords = "Clamp plugin Array Integer", ToolTip = "Clamp an array"), Category = "Clamp")
		static TArray<int32> ClampInteger(TArray<int32> Array, int32 Min, int32 Max);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClampFloat", CompactNodeTitle = "Clamp", Keywords = "Clamp plugin Array Float", ToolTip = "Clamp an array"), Category = "Clamp")
		static TArray<float> ClampFloat(TArray<float> Array, float Min, float Max);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClampVector", CompactNodeTitle = "ClampSize", Keywords = "Clamp plugin Array Vector Size", ToolTip = "Clamp an array"), Category = "Clamp")
		static TArray<FVector> ClampVectorSize(TArray<FVector> Array, float MinSize, float MaxSize, bool bOnly2D = false);
		
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClampIntegerByRef", CompactNodeTitle = "ClampRef", Keywords = "Clamp plugin Array Integer Reference", ToolTip = "Clamp an array"), Category = "Clamp")
		static void ClampIntegerRef(UPARAM(ref) TArray<int32>& Array, int32 Min, int32 Max);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClampFloatByRef", CompactNodeTitle = "ClampRef", Keywords = "Clamp plugin Array Float Reference", ToolTip = "Clamp an array"), Category = "Clamp")
		static void ClampFloatRef(UPARAM(ref) TArray<float>& Array, float Min, float Max);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClampVectorRef", CompactNodeTitle = "ClampSizeRef", Keywords = "Clamp plugin Array Vector Size Reference", ToolTip = "Clamp an array"), Category = "Clamp")
		static void ClampVectorSizeRef(UPARAM(ref) TArray<FVector>& Array, float MinSize, float MaxSize, bool bOnly2D = false);

		/* Range */
		template <typename T> static TArray<T> Extract(const TArray<T>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractInteger", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Integer Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<int32> ExtractInteger(const TArray<int32>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractFloat", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Float Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<float> ExtractFloat(const TArray<float>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractString", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array String Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<FString> ExtractString(const TArray<FString>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractName", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Name Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<FName> ExtractName(const TArray<FName>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractVector", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Vector Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<FVector> ExtractVector(const TArray<FVector>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractActor", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Actor Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<AActor*> ExtractActor(const TArray<AActor*>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractObject", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Object Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<UObject*> ExtractObject(const TArray<UObject*>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractColor", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Color Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<FColor> ExtractColor(const TArray<FColor>& Array, int32 StartIndex, int32 EndIndex);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExtractByte", CompactNodeTitle = "Extract", Keywords = "Extract plugin Array Byte Subarray", ToolTip = "Extracts a subarray from array"), Category = "Range")
		static TArray<uint8> ExtractByte(const TArray<uint8>& Array, int32 StartIndex, int32 EndIndex);
	
		/* Random */
		template <typename T> static TArray<T> Random(int32 Size, T& MinValue, T& MaxValue);
		UFUNCTION(BlueprintPure, meta = (DisplayName = "RandomInteger", CompactNodeTitle = "Random", Keywords = "Random plugin Array Integer", ToolTip = "Generates a random array"), Category = "Random")
		static TArray<int32> RandomInteger(int32 Size, int32 MinValue, int32 MaxValue);
		UFUNCTION(BlueprintPure, meta = (DisplayName = "RandomFloat", CompactNodeTitle = "Random", Keywords = "Random plugin Array Float", ToolTip = "Generates a random array"), Category = "Random")
		static TArray<float> RandomFloat(int32 Size, float MinValue, float MaxValue);
		UFUNCTION(BlueprintPure, meta = (DisplayName = "RandomVector", CompactNodeTitle = "Random", Keywords = "Random plugin Array Vector", ToolTip = "Generates a random array"), Category = "Random")
		static TArray<FVector> RandomVector(int32 Size, FVector MinValue, FVector MaxValue);

		/* Sorts */
		template <typename T> static TArray<T> Sort(TArray<T>& Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortInteger", CompactNodeTitle = "Sort", Keywords = "Sort plugin Array Integer", ToolTip = "Sort an array in ascending or descending order"), Category = "Sort")
		static TArray<int32> SortInteger(TArray<int32> Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortFloat", CompactNodeTitle="Sort", Keywords = "Sort plugin Array Float", ToolTip = "Sort an array in ascending or descending order"), Category = "Sort")
		static TArray<float> SortFloat(TArray<float> Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortString", CompactNodeTitle = "Sort", Keywords = "Sort plugin Array String", ToolTip = "Sort an array in ascending or descending order"), Category = "Sort")
		static TArray<FString> SortString(TArray<FString> Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortName", CompactNodeTitle = "Sort", Keywords = "Sort plugin Array Name", ToolTip = "Sort an array in ascending or descending order"), Category = "Sort")
		static TArray<FName> SortName(TArray<FName> Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortActor", CompactNodeTitle = "Sort", Keywords = "Sort plugin Array Actor", ToolTip = "Sort an array in ascending or descending order"), Category = "Sort")
		static TArray<AActor*> SortActor(TArray<AActor*> Array, AActor* const & Actor, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortVector", CompactNodeTitle = "Sort", Keywords = "Sort plugin Array Vector", ToolTip = "Sort an array in ascending or descending order"), Category = "Sort")
		static TArray<FVector> SortVector(TArray<FVector> Array, FVector Origin, bool bIsAscending = true);

		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortIntegerByRef", CompactNodeTitle = "SortRef", Keywords = "Sort plugin Array Integer Reference", ToolTip = "Sort an array in ascending or descending order by reference"), Category = "Sort")
		static void SortIntegerRef(UPARAM(ref) TArray<int32>& Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortFloatByRef", CompactNodeTitle = "SortRef", Keywords = "Sort plugin Array Float Reference", ToolTip = "Sort an array in ascending or descending order by reference"), Category = "Sort")
		static void SortFloatRef(UPARAM(ref) TArray<float>& Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortStringByRef", CompactNodeTitle = "SortRef", Keywords = "Sort plugin Array String Reference", ToolTip = "Sort an array in ascending or descending order by reference"), Category = "Sort")
		static void SortStringRef(UPARAM(ref) TArray<FString>& Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortNameByRef", CompactNodeTitle = "SortRef", Keywords = "Sort plugin Array Name Reference", ToolTip = "Sort an array in ascending or descending order by reference"), Category = "Sort")
		static void SortNameRef(UPARAM(ref) TArray<FName>& Array, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortActorByRef", CompactNodeTitle = "SortRef", Keywords = "Sort plugin Array Actor Reference", ToolTip = "Sort an array in ascending or descending order by reference"), Category = "Sort")
		static void SortActorRef(UPARAM(ref) TArray<AActor*>& Array, AActor* const & Actor, bool bIsAscending = true);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortVectorByRef", CompactNodeTitle = "SortRef", Keywords = "Sort plugin Array Vector Reference", ToolTip = "Sort an array in ascending or descending order by reference"), Category = "Sort")
		static void SortVectorRef(UPARAM(ref) TArray<FVector>& Array, FVector Origin, bool bIsAscending = true);

		/* Sorts Predicate */
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortInteger", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array Integer Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<int32> PredicateSortInteger(TArray<int32> Array, const FIntSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortFloat", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array Float Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<float> PredicateSortFloat(TArray<float> Array, const FFloatSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortString", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array String Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<FString> PredicateSortString(TArray<FString> Array, const FStringSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortName", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array Name Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<FName> PredicateSortName(TArray<FName> Array, const FNameSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortVector", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array Vector Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<FVector> PredicateSortVector(TArray<FVector> Array, const FVectorSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortActor", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array Actor Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<AActor*> PredicateSortActor(TArray<AActor*> Array, const FActorSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortObject", CompactNodeTitle = "PredicateSort", Keywords = "Sort plugin Array Object Predicate", ToolTip = "Sort an array using a predicate"), Category = "Sort")
		static TArray<UObject*> PredicateSortObject(TArray<UObject*> Array, const FObjectSortDelegate& PredicateFunction, bool InvertResult = false);
	
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortIntegerByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array Integer Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortIntegerRef(UPARAM(ref) TArray<int32>& Array, const FIntSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortFloatByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array Float Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortFloatRef(UPARAM(ref) TArray<float>& Array, const FFloatSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortStringByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array String Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortStringRef(UPARAM(ref) TArray<FString>& Array, const FStringSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortNameByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array Name Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortNameRef(UPARAM(ref) TArray<FName>& Array, const FNameSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortVectorByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array Vector Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortVectorRef(UPARAM(ref) TArray<FVector>& Array, const FVectorSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortActorByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array Actor Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortActorRef(UPARAM(ref) TArray<AActor*>& Array, const FActorSortDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateSortObjectByRef", CompactNodeTitle = "PredicateSortRef", Keywords = "Sort plugin Array Object Predicate Reference", ToolTip = "Sort an array using a predicate by reference"), Category = "Sort")
		static void PredicateSortObjectRef(UPARAM(ref) TArray<UObject*>& Array, const FObjectSortDelegate& PredicateFunction, bool InvertResult = false);

		/* Distance */
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClosestLocation", CompactNodeTitle = "Closest", Keywords = "Vector plugin Array Closest", ToolTip = "Get closest location to an origin"), Category = "Distance")
		static void ClosestLocation(const TArray<FVector>& Array, FVector Origin, FVector& Closest, float& Distance, int32& Index);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "FarthestLocation", CompactNodeTitle = "Farthest", Keywords = "Vector plugin Array Farthest", ToolTip = "Get farthest location to an origin"), Category = "Distance")
		static void FarthestLocation(const TArray<FVector>& Array, FVector Origin, FVector& Farthest, float& Distance, int32& Index);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClosestActor", CompactNodeTitle = "Closest", Keywords = "Actor plugin Array Closest", ToolTip = "Get closest actor to an origin actor"), Category = "Distance")
		static void ClosestActor(const TArray<AActor*>& Array, AActor* const & Origin, AActor*& Closest, float& Distance, int32& Index);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "FarthestActor", CompactNodeTitle = "Farthest", Keywords = "Actor plugin Array Farthest", ToolTip = "Get farthest actor to an origin actor"), Category = "Distance")
		static void FarthestActor(const TArray<AActor*>& Array, AActor* const & Origin, AActor*& Farthest, float& Distance, int32& Index);
		
		/* Filters (regex) */
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "FilterMatches", CompactNodeTitle = "Matches", Keywords = "Filter plugin Array Matches Regex", ToolTip = "Finds matching regex expressions in array"), Category = "Filter")
		static TArray<FString> FilterMatches(const TArray<FString>& Array, const FString& Pattern, bool& bFound, TArray<int32>& Indexes);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "FilterMatch", CompactNodeTitle = "Match", Keywords = "Filter plugin Array Match Regex", ToolTip = "Finds the first matching regex expression in array"), Category = "Filter")
		static FString FilterMatch(const TArray<FString>& Array, const FString& Pattern, bool& bFound, int32& Index);
		
		/* Filters Predicate */
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterInteger", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array Integer Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<int32> PredicateFilterInteger(const TArray<int32>& Array, const FIntFilterDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterFloat", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array Float Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<float> PredicateFilterFloat(const TArray<float>& Array, const FFloatFilterDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterString", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array String Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<FString> PredicateFilterString(const TArray<FString>& Array, const FStringFilterDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterName", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array Name Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<FName> PredicateFilterName(const TArray<FName>& Array, const FNameFilterDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterVector", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array Vector Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<FVector> PredicateFilterVector(const TArray<FVector>& Array, const FVectorFilterDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterActor", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array Actor Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<AActor*> PredicateFilterActor(const TArray<AActor*>& Array, const FActorFilterDelegate& PredicateFunction, bool InvertResult = false);
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "PredicateFilterObject", CompactNodeTitle = "PredicateFilter", Keywords = "Filter plugin Array Object Predicate", ToolTip = "Filter an array using a predicate"), Category = "Filter")
		static TArray<UObject*> PredicateFilterObject(const TArray<UObject*>& Array, const FObjectFilterDelegate& PredicateFunction, bool InvertResult = false);
};
