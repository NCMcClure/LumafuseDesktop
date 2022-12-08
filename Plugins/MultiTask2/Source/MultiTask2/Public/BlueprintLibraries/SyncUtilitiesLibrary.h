// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SyncUtilities.h"
#include "SyncUtilitiesLibrary.generated.h"


UCLASS()
class MULTITASK2_API USyncUtilitiesLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToBool (ThreadSafeBoolean)", CompactNodeTitle = "->",
        BlueprintAutocast), Category = "Multi Task 2|ThreadSafeBoolean")
    static bool Conv_ThreadSafeBooleanToBool(const FThreadSafeBoolean& ThreadSafeBoolean);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (ThreadSafeBoolean)", CompactNodeTitle = "->",
        BlueprintAutocast), Category = "Multi Task 2|ThreadSafeBoolean")
    static FString Conv_ThreadSafeBooleanToString(const FThreadSafeBoolean& ThreadSafeBoolean);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (ThreadSafeBoolean)", CompactNodeTitle = "=="), Category =
        "Multi Task 2|ThreadSafeBoolean")
    static bool ThreadSafeBooleanEqualsThreadSafeBoolean(const FThreadSafeBoolean& A, const FThreadSafeBoolean& B);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (bool)", CompactNodeTitle = "=="), Category =
        "Multi Task 2|ThreadSafeBoolean")
    static bool ThreadSafeBooleanEqualsBoolean(const FThreadSafeBoolean& A, const bool& B);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set (bool)", CompactNodeTitle = "Set"), Category =
        "Multi Task 2|ThreadSafeBoolean")
    static void ThreadSafeBooleanSetBoolean(UPARAM(ref)FThreadSafeBoolean& A, bool B);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set (ThreadSafeBoolean)", CompactNodeTitle = "Set"), Category =
        "Multi Task 2|ThreadSafeBoolean")
    static void ThreadSafeBooleanSetThreadSafeBoolean(UPARAM(ref)FThreadSafeBoolean& A, const FThreadSafeBoolean& B);

    //---------------------------------------------------------

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToInt32 (ThreadSafeInteger)", CompactNodeTitle = "->",
        BlueprintAutocast), Category = "Multi Task 2|ThreadSafeInteger")
    static int32 Conv_ThreadSafeIntegerToInt32(const FThreadSafeInteger& ThreadSafeInteger);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (ThreadSafeInteger)", CompactNodeTitle = "->",
        BlueprintAutocast), Category = "Multi Task 2|ThreadSafeInteger")
    static FString Conv_ThreadSafeIntegerToString(const FThreadSafeInteger& ThreadSafeInteger);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (ThreadSafeInteger)", CompactNodeTitle = "=="), Category =
        "Multi Task 2|ThreadSafeInteger")
    static bool ThreadSafeIntegerEqualsThreadSafeInteger(const FThreadSafeInteger& A, const FThreadSafeInteger& B);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (Integer)", CompactNodeTitle = "=="), Category =
        "Multi Task 2|ThreadSafeInteger")
    static bool ThreadSafeIntegerEqualsInt32(const FThreadSafeInteger& A, const int32& B);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set (ThreadSafeInteger)", CompactNodeTitle = "Set"), Category =
        "Multi Task 2|ThreadSafeInteger")
    static void ThreadSafeIntegerSetThreadSafeInteger(UPARAM(ref)FThreadSafeInteger& A, const FThreadSafeInteger& B);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set (Integer)", CompactNodeTitle = "Set"), Category =
        "Multi Task 2|ThreadSafeInteger")
    static void ThreadSafeIntegerSetInteger(UPARAM(ref)FThreadSafeInteger& A, int32 B);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Increment (ThreadSafeInteger)", CompactNodeTitle = "++",
        BlueprintAutocast), Category = "Multi Task 2|ThreadSafeInteger")
    static void ThreadSafeIntegerIncrement(UPARAM(ref)FThreadSafeInteger& ThreadSafeInteger);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Decrement (ThreadSafeInteger)", CompactNodeTitle = "--",
        BlueprintAutocast), Category = "Multi Task 2|ThreadSafeInteger")
    static void ThreadSafeIntegerDecrement(UPARAM(ref)FThreadSafeInteger& ThreadSafeInteger);

};
