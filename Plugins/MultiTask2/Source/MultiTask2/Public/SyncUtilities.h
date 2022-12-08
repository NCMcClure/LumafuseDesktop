// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"
#include "SyncUtilities.generated.h"

USTRUCT(BlueprintType)
struct MULTITASK2_API FThreadSafeBoolean
{
    GENERATED_BODY()
    FThreadSafeBool Bool;

    bool ToBool() const
    {
        return Bool;
    }

    FString ToString() const
    {
        return Bool ? "true" : "false";
    }

    bool operator==(const FThreadSafeBoolean& Other) const
    {
        return Bool == Other.Bool;
    }

    bool operator==(const bool& Other) const
    {
        return Bool == Other;
    }

    void operator=(const FThreadSafeBoolean& Other)
    {
        Bool.AtomicSet(Other.ToBool());
    }

    void operator=(const bool& Other)
    {
        Bool.AtomicSet(Other);
    }
};


USTRUCT(BlueprintType)
struct MULTITASK2_API FThreadSafeInteger
{
    GENERATED_BODY()
    FThreadSafeCounter Integer;

    FThreadSafeInteger()
    {
        Integer.Set(0);
    }

    int32 GetValue() const
    {
        return Integer.GetValue();
    }

    FString ToString() const
    {
        return FString::FromInt(GetValue());
    }

    void Increment()
    {
        Integer.Increment();
    }

    void Decrement()
    {
        Integer.Decrement();
    }

    void operator=(const int32& Other)
    {
        Integer.Set(Other);
    }

    void operator=(const FThreadSafeInteger& Other)
    {
        Integer.Set(Other.GetValue());
    }

    bool operator==(const int32& Other) const
    {
        return GetValue() == Other;
    }

    bool operator==(const FThreadSafeInteger& Other) const
    {
        return GetValue() == Other.GetValue();
    }
};

