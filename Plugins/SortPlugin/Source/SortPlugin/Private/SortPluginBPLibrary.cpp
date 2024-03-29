// Copyright 2021 RLoris

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SortPluginBPLibrary.h"
#include "SortPlugin.h"
#include "Internationalization/Regex.h"

USortPluginBPLibrary::USortPluginBPLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

template <typename T> TArray<T> USortPluginBPLibrary::Sort(TArray<T>& Array, bool bIsAscending)
{
	if (bIsAscending)
	{
		Array.Sort();
	}
	else
	{
		Array.Sort(TReverseSortPredicate<T>());
	}
	return Array;
}

TArray<int32> USortPluginBPLibrary::SortInteger(TArray<int32> Array, bool bIsAscending)
{	
	return Sort<int32>(Array, bIsAscending);
}

TArray<float> USortPluginBPLibrary::SortFloat(TArray<float> Array, bool bIsAscending)
{
	return Sort<float>(Array, bIsAscending);
}

TArray<FString> USortPluginBPLibrary::SortString(TArray<FString> Array, bool bIsAscending)
{
	return Sort<FString>(Array, bIsAscending);
}

TArray<FName> USortPluginBPLibrary::SortName(TArray<FName> Array, bool bIsAscending)
{
	// DO NOT USE FOR FNAME -> return Sort<FName>(Array, bIsAscending)
	if (bIsAscending)
	{
		Array.Sort([](const FName A, const FName B) {
			return A.LexicalLess(B);
		});
	}
	else
	{
		Array.Sort([](const FName A, const FName B) {
			return B.LexicalLess(A);
		});
	}
	return Array;
}

TArray<AActor*> USortPluginBPLibrary::SortActor(TArray<AActor*> Array, AActor* const & Actor, bool bIsAscending)
{
	if (Actor != nullptr)
	{
		if (bIsAscending)
		{
			Array.Sort([Actor](AActor& A, AActor& B) {
				return (&A)->GetDistanceTo(Actor) < (&B)->GetDistanceTo(Actor);
			});
		}
		else
		{
			Array.Sort([Actor](AActor& A, AActor& B) {
				return (&A)->GetDistanceTo(Actor) > (&B)->GetDistanceTo(Actor);
			});
		}
	}
	return Array;
}

TArray<FVector> USortPluginBPLibrary::SortVector(TArray<FVector> Array, FVector Origin, bool bIsAscending)
{
	if (bIsAscending)
	{
		Array.Sort([Origin](FVector A, FVector B) {
			return FVector::Distance(A, Origin) < FVector::Distance(B, Origin);
		});
	}
	else
	{
		Array.Sort([Origin](FVector A, FVector B) {
			return FVector::Distance(A, Origin) > FVector::Distance(B, Origin);
		});
	}
	return Array;
}

void USortPluginBPLibrary::ClosestLocation(const TArray<FVector>& Array, FVector Origin, FVector& ClosestVector, float& Distance, int32& Index)
{
	float ClosestDistance = 0;
	Index = -1;
	for(int i = 0; i < Array.Num(); i++) 
	{
		FVector Vector = Array[i];
		Distance = FVector::Distance(Origin, Vector);
		if (i == 0 || Distance < ClosestDistance) 
		{
			Index = i;
			ClosestDistance = Distance;
			ClosestVector = Vector;
		}
	}
	Distance = ClosestDistance;
}

void USortPluginBPLibrary::FarthestLocation(const TArray<FVector>& Array, FVector Origin, FVector& FarthestVector, float& Distance, int32& Index)
{
	float FarthestDistance = 0;
	Index = -1;
	for (int i = 0; i < Array.Num(); i++)
	{
		FVector Vector = Array[i];
		Distance = FVector::Distance(Origin, Vector);
		if (i == 0 || Distance > FarthestDistance)
		{
			Index = i;
			FarthestDistance = Distance;
			FarthestVector = Vector;
		}
	}
	Distance = FarthestDistance;
}

void USortPluginBPLibrary::ClosestActor(const TArray<AActor*>& Array, AActor* const & Origin, AActor*& Closest, float& Distance, int32& Index)
{
	float ClosestDistance = 0;
	Index = -1;
	if (Origin == nullptr)
	{
		return;
	}
	for (int i = 0; i < Array.Num(); i++)
	{
		AActor* Actor = Array[i];
		Distance = Origin->GetDistanceTo(Actor);
		if (i == 0 || Distance < ClosestDistance)
		{
			Index = i;
			ClosestDistance = Distance;
			Closest = Actor;
		}
	}
	Distance = ClosestDistance;
}

void USortPluginBPLibrary::FarthestActor(const TArray<AActor*>& Array, AActor* const & Origin, AActor*& Farthest, float& Distance, int32& Index)
{
	float FarthestDistance = 0;
	Index = -1;
	if (Origin == nullptr)
	{
		return;
	}
	for (int i = 0; i < Array.Num(); i++)
	{
		AActor* Actor = Array[i];
		Distance = Origin->GetDistanceTo(Actor);
		if (i == 0 || Distance > FarthestDistance)
		{
			Index = i;
			FarthestDistance = Distance;
			Farthest = Actor;
		}
	}
	Distance = FarthestDistance;
}

TArray<int32> USortPluginBPLibrary::PredicateSortInteger(TArray<int32> Array, const FIntSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const int32& A, const int32& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<float> USortPluginBPLibrary::PredicateSortFloat(TArray<float> Array, const FFloatSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const float& A, const float& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<FString> USortPluginBPLibrary::PredicateSortString(TArray<FString> Array, const FStringSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const FString& A, const FString& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<FName> USortPluginBPLibrary::PredicateSortName(TArray<FName> Array, const FNameSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const FName& A, const FName& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<FVector> USortPluginBPLibrary::PredicateSortVector(TArray<FVector> Array, const FVectorSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const FVector& A, const FVector& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<AActor*> USortPluginBPLibrary::PredicateSortActor(TArray<AActor*> Array, const FActorSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](AActor& A, AActor& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(&A, &B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<UObject*> USortPluginBPLibrary::PredicateSortObject(TArray<UObject*> Array, const FObjectSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](UObject& A, UObject& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(&A, &B, Result);
		return InvertResult ? !Result : Result;
	});
	return Array;
}

TArray<FString> USortPluginBPLibrary::FilterMatches(const TArray<FString>& Array, const FString& Pattern, bool& bFound, TArray<int32>& Indexes)
{
	const FRegexPattern customPattern(Pattern);
	TArray<FString> Result;
	bFound = false;
	for(int32 i = 0; i < Array.Num(); i++) 
	{
		FRegexMatcher CustomMatcher(customPattern, Array[i]);

		if (CustomMatcher.FindNext())
		{
			bFound = true;
			Result.Add(Array[i]);
			Indexes.Add(i);
		}
	}
	return Result;
}

FString USortPluginBPLibrary::FilterMatch(const TArray<FString>& Array, const FString& Pattern, bool& bFound, int32& Index)
{
	const FRegexPattern customPattern(Pattern);
	bFound = false;
	Index = 0;
	for (FString String : Array)
	{
		FRegexMatcher CustomMatcher(customPattern, String);
		if (CustomMatcher.FindNext())
		{
			bFound = true;
			return String;
		}
		Index++;
	}
	Index = -1;
	return FString();
}

TArray<int32> USortPluginBPLibrary::PredicateFilterInteger(const TArray<int32>& Array, const FIntFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const int32& A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

TArray<float> USortPluginBPLibrary::PredicateFilterFloat(const TArray<float>& Array, const FFloatFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const float& A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

TArray<FString> USortPluginBPLibrary::PredicateFilterString(const TArray<FString>& Array, const FStringFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const FString& A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

TArray<FName> USortPluginBPLibrary::PredicateFilterName(const TArray<FName>& Array, const FNameFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const FName& A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

TArray<UObject*> USortPluginBPLibrary::PredicateFilterObject(const TArray<UObject*>& Array, const FObjectFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const UObject* A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

TArray<FVector> USortPluginBPLibrary::PredicateFilterVector(const TArray<FVector>& Array, const FVectorFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const FVector& A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

TArray<AActor*> USortPluginBPLibrary::PredicateFilterActor(const TArray<AActor*>& Array, const FActorFilterDelegate& PredicateFunction, bool InvertResult)
{
	return Array.FilterByPredicate([PredicateFunction, InvertResult](const AActor* A) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, Result);
		return InvertResult ? !Result : Result;
	});
}

template<typename T>
int32 USortPluginBPLibrary::Minimum(const TArray<T>& Array)
{
	int32 Index = Array.Num() > 0 ? 0 : -1;
	for (int32 i = 1; i < Array.Num(); i++)
	{
		if (Array[i] < Array[Index]) 
		{
			Index = i;
		}
	}
	return Index;
}

template<typename T>
int32 USortPluginBPLibrary::Maximum(const TArray<T>& Array)
{
	int32 Index = Array.Num() > 0 ? 0 : -1;
	for (int32 i = 1; i < Array.Num(); i++)
	{
		if (Array[i] > Array[Index])
		{
			Index = i;
		}
	}
	return Index;
}

template<typename T>
TArray<T> USortPluginBPLibrary::MinMaxNormalization(const TArray<T>& Array, T Min, T Max)
{
	TArray<T> NormalizedArray;
	if (Array.Num() > 0) 
	{
		NormalizedArray.SetNumUninitialized(Array.Num());
		const int32 MinIdx = USortPluginBPLibrary::Minimum(Array);
		const int32 MaxIdx = USortPluginBPLibrary::Maximum(Array);
		const T MinVal = Array[MinIdx];
		const T MaxVal = Array[MaxIdx] != 0 ? Array[MaxIdx] : 1;
		for (int32 i = 0; i < Array.Num(); i++)
		{
			NormalizedArray[i] = (((Array[i] - MinVal) / (MaxVal * 1.0f)) * (Max - Min)) + Min;
		}
	}
	return NormalizedArray;
}

template <typename T> TArray<T> USortPluginBPLibrary::Reverse(const TArray<T>& Array)
{
	TArray<T> ReversedArray;
	for (auto i = Array.Num() - 1; i >= 0; i--)
	{
		ReversedArray.Add(Array[i]);
	}
	return ReversedArray;
}

template <typename T> float USortPluginBPLibrary::Average(const TArray<T>& Array)
{
	float Average = 0.0f;
	int32 Total = Array.Num();
	Total = Total > 0 ? Total : 1;

	for (const T& Value : Array)
	{
		Average += Value;
	}

	return Average / Total;
}

float USortPluginBPLibrary::AverageInteger(const TArray<int32>& Array)
{
	return USortPluginBPLibrary::Average(Array);
}

float USortPluginBPLibrary::AverageFloat(const TArray<float>& Array)
{
	return USortPluginBPLibrary::Average(Array);
}

int32 USortPluginBPLibrary::MinimumIntegerIndex(const TArray<int32>& Array)
{
	return USortPluginBPLibrary::Minimum(Array);
}

int32 USortPluginBPLibrary::MinimumFloatIndex(const TArray<float>& Array)
{
	return USortPluginBPLibrary::Minimum(Array);
}

int32 USortPluginBPLibrary::MaximumIntegerIndex(const TArray<int32>& Array)
{
	return USortPluginBPLibrary::Maximum(Array);
}

int32 USortPluginBPLibrary::MaximumFloatIndex(const TArray<float>& Array)
{
	return USortPluginBPLibrary::Maximum(Array);
}

TArray<float> USortPluginBPLibrary::MinMaxFloatNormalization(const TArray<float>& Array, float Min, float Max)
{
	return USortPluginBPLibrary::MinMaxNormalization(Array, Min, Max);
}

TArray<int32> USortPluginBPLibrary::MinMaxIntegerNormalization(const TArray<int32>& Array, int32 Min, int32 Max)
{
	return USortPluginBPLibrary::MinMaxNormalization(Array, Min, Max);
}

TArray<int32> USortPluginBPLibrary::ReverseInteger(const TArray<int32>& Array)
{
	return Reverse<int32>(Array);
}

TArray<float> USortPluginBPLibrary::ReverseFloat(const TArray<float>& Array)
{
	return Reverse<float>(Array);
}

TArray<FString> USortPluginBPLibrary::ReverseString(const TArray<FString>& Array)
{
	return Reverse<FString>(Array);
}

TArray<FName> USortPluginBPLibrary::ReverseName(const TArray<FName>& Array)
{
	return Reverse<FName>(Array);
}

TArray<FVector> USortPluginBPLibrary::ReverseVector(const TArray<FVector>& Array)
{
	return Reverse<FVector>(Array);
}

TArray<AActor*> USortPluginBPLibrary::ReverseActor(const TArray<AActor*>& Array)
{
	return Reverse<AActor*>(Array);
}

TArray<UObject*> USortPluginBPLibrary::ReverseObject(const TArray<UObject*>& Array)
{
	return Reverse<UObject*>(Array);
}

TArray<FString> USortPluginBPLibrary::SplitString(FString String, const FString& Separator, ESearchCase::Type SearchCase, bool RemoveEmptyString)
{
	FString LeftString;
	FString RightString;
	TArray<FString> Array;
	bool Split = false;
	do 
	{
		Split = String.Split(Separator, &LeftString, &RightString, SearchCase);
		if (Split) 
		{
			if (RemoveEmptyString)
			{
				if (!LeftString.IsEmpty())
				{
					Array.Add(LeftString);
				}
			}
			else
			{
				Array.Add(LeftString);
			}
		}
		else
		{
			if (RemoveEmptyString)
			{
				if (!String.IsEmpty())
				{
					Array.Add(String);
				}
			}
			else
			{
				Array.Add(String);
			}
		}
		String = RightString;
	} while (Split);

	return Array;
}

template <typename T> TArray<T> USortPluginBPLibrary::Extract(const TArray<T>& Array, int32 StartIndex, int32 EndIndex) 
{
	TArray<T> SubArray;
	if (StartIndex < EndIndex && EndIndex < Array.Num())
	{
		for (int i = StartIndex; i <= EndIndex; i++)
		{
			SubArray.Add(Array[i]);
		}
	}
	return SubArray;
}

TArray<int32> USortPluginBPLibrary::ExtractInteger(const TArray<int32>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<int32>(Array, StartIndex, EndIndex);
}

TArray<float> USortPluginBPLibrary::ExtractFloat(const TArray<float>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<float>(Array, StartIndex, EndIndex);
}

TArray<FString> USortPluginBPLibrary::ExtractString(const TArray<FString>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<FString>(Array, StartIndex, EndIndex);
}

TArray<FName> USortPluginBPLibrary::ExtractName(const TArray<FName>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<FName>(Array, StartIndex, EndIndex);
}

TArray<FVector> USortPluginBPLibrary::ExtractVector(const TArray<FVector>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<FVector>(Array, StartIndex, EndIndex);
}

TArray<AActor*> USortPluginBPLibrary::ExtractActor(const TArray<AActor*>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<AActor*>(Array, StartIndex, EndIndex);
}

TArray<UObject*> USortPluginBPLibrary::ExtractObject(const TArray<UObject*>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<UObject*>(Array, StartIndex, EndIndex);
}

TArray<FColor> USortPluginBPLibrary::ExtractColor(const TArray<FColor>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<FColor>(Array, StartIndex, EndIndex);
}

TArray<uint8> USortPluginBPLibrary::ExtractByte(const TArray<uint8>& Array, int32 StartIndex, int32 EndIndex)
{
	return Extract<uint8>(Array, StartIndex, EndIndex);
}

template <typename T> TArray<T> USortPluginBPLibrary::Random(int32 Size, T& MinValue, T& MaxValue)
{
	TArray<T> Array;
	if (Size > 0 && MinValue < MaxValue) {
		for (int i = 0; i < Size; i++)
		{
			Array.Add(FMath::RandRange(MinValue, MaxValue));
		}
	}
	return Array;
}

TArray<int32> USortPluginBPLibrary::RandomInteger(int32 Size, int32 MinValue, int32 MaxValue)
{
	return Random<int32>(Size, MinValue, MaxValue);
}

TArray<float> USortPluginBPLibrary::RandomFloat(int32 Size, float MinValue, float MaxValue)
{
	return Random<float>(Size, MinValue, MaxValue);
}

TArray<FVector> USortPluginBPLibrary::RandomVector(int32 Size, FVector MinValue, FVector MaxValue)
{
	TArray<FVector> Array;
	if (Size > 0 && MinValue.X <= MaxValue.X && MinValue.Y <= MaxValue.Y && MinValue.Z <= MaxValue.Z)
	{
		for (int i = 0; i < Size; i++)
		{
			Array.Add(FVector(FMath::FRandRange(MinValue.X, MaxValue.X), FMath::FRandRange(MinValue.Y, MaxValue.Y), FMath::FRandRange(MinValue.Z, MaxValue.Z)));
		}
	}
	return Array;
}

template <typename T> TArray<T> USortPluginBPLibrary::Clamp(TArray<T>& Array, T Min, T Max)
{
	if (Min < Max)
	{
		for (int i = 0; i < Array.Num(); i++)
		{
			if (Array[i] < Min)
			{
				Array[i] = Min;
			}
			else if (Array[i] > Max)
			{
				Array[i] = Max;
			}
		}
	}
	return Array;
}

TArray<int32> USortPluginBPLibrary::ClampInteger(TArray<int32> Array, int32 Min, int32 Max)
{
	return Clamp<int32>(Array, Min, Max);
}

TArray<float> USortPluginBPLibrary::ClampFloat(TArray<float> Array, float Min, float Max)
{
	return Clamp<float>(Array, Min, Max);
}

TArray<FVector> USortPluginBPLibrary::ClampVectorSize(TArray<FVector> Array, float MinSize, float MaxSize, bool bOnly2D)
{
	if (MinSize < MaxSize)
	{
		for (int i = 0; i < Array.Num(); i++)
		{
			float Size = bOnly2D ? Array[i].Size2D() : Array[i].Size();
			if (Size < MinSize || Size > MaxSize)
			{
				Array[i] = bOnly2D ? Array[i].GetClampedToSize2D(MinSize, MaxSize) : Array[i].GetClampedToSize(MinSize, MaxSize);
			}
		}
	}
	return Array;
}

void USortPluginBPLibrary::SortIntegerRef(TArray<int32>& Array, bool bIsAscending)
{
	Sort<int32>(Array, bIsAscending);
}

void USortPluginBPLibrary::SortFloatRef(TArray<float>& Array, bool bIsAscending)
{
	Sort<float>(Array, bIsAscending);
}

void USortPluginBPLibrary::SortStringRef(TArray<FString>& Array, bool bIsAscending)
{
	Sort<FString>(Array, bIsAscending);
}

void USortPluginBPLibrary::SortNameRef(TArray<FName>& Array, bool bIsAscending)
{
	// DO NOT USE FOR FNAME -> Sort<FName>(Array, bIsAscending);
	if (bIsAscending)
	{
		Array.Sort([](FName A, FName B) {
			return A.LexicalLess(B);
		});
	}
	else
	{
		Array.Sort([](FName A, FName B) {
			return B.LexicalLess(A);
		});
	}
}

void USortPluginBPLibrary::SortVectorRef(TArray<FVector>& Array, FVector Origin, bool bIsAscending)
{
	if (bIsAscending)
	{
		Array.Sort([Origin](FVector A, FVector B) {
			return FVector::Distance(A, Origin) < FVector::Distance(B, Origin);
		});
	}
	else
	{
		Array.Sort([Origin](FVector A, FVector B) {
			return FVector::Distance(A, Origin) > FVector::Distance(B, Origin);
		});
	}
}

void USortPluginBPLibrary::SortActorRef(TArray<AActor*>& Array, AActor* const & Actor, bool bIsAscending)
{
	if (Actor != nullptr)
	{
		if (bIsAscending)
		{
			Array.Sort([Actor](AActor& A, AActor& B) {
				return (&A)->GetDistanceTo(Actor) < (&B)->GetDistanceTo(Actor);
			});
		}
		else
		{
			Array.Sort([Actor](AActor& A, AActor& B) {
				return (&A)->GetDistanceTo(Actor) >  (&B)->GetDistanceTo(Actor);
			});
		}
	}
}

void USortPluginBPLibrary::ClampIntegerRef(TArray<int32>& Array, int32 Min, int32 Max)
{
	Clamp(Array, Min, Max);
}

void USortPluginBPLibrary::ClampFloatRef(TArray<float>& Array, float Min, float Max)
{
	Clamp(Array, Min, Max);
}

void USortPluginBPLibrary::ClampVectorSizeRef(TArray<FVector>& Array, float MinSize, float MaxSize, bool bOnly2D)
{
	if (MinSize < MaxSize)
	{
		for (int i = 0; i < Array.Num(); i++)
		{
			float Size = bOnly2D ? Array[i].Size2D() : Array[i].Size();
			if (Size < MinSize || Size > MaxSize)
			{
				Array[i] = bOnly2D ? Array[i].GetClampedToSize2D(MinSize, MaxSize) : Array[i].GetClampedToSize(MinSize, MaxSize);
			}
		}
	}
}

void USortPluginBPLibrary::PredicateSortIntegerRef(TArray<int32>& Array, const FIntSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const int32& A, const int32& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
}

void USortPluginBPLibrary::PredicateSortFloatRef(TArray<float>& Array, const FFloatSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const float& A, const float& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
}

void USortPluginBPLibrary::PredicateSortStringRef(TArray<FString>& Array, const FStringSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const FString& A, const FString& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
}

void USortPluginBPLibrary::PredicateSortNameRef(TArray<FName>& Array, const FNameSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const FName& A, const FName& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
}

void USortPluginBPLibrary::PredicateSortVectorRef(TArray<FVector>& Array, const FVectorSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](const FVector& A, const FVector& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(A, B, Result);
		return InvertResult ? !Result : Result;
	});
}

void USortPluginBPLibrary::PredicateSortActorRef(TArray<AActor*>& Array, const FActorSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](AActor& A, AActor& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(&A, &B, Result);
		return InvertResult ? !Result : Result;
	});
}

void USortPluginBPLibrary::PredicateSortObjectRef(TArray<UObject*>& Array, const FObjectSortDelegate& PredicateFunction, bool InvertResult)
{
	Array.Sort([PredicateFunction, InvertResult](UObject& A, UObject& B) {
		bool Result;
		PredicateFunction.ExecuteIfBound(&A, &B, Result);
		return InvertResult ? !Result : Result;
	});
}

