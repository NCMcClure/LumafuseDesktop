// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.
#include "SyncUtilitiesLibrary.h"

bool USyncUtilitiesLibrary::Conv_ThreadSafeBooleanToBool(const FThreadSafeBoolean& ThreadSafeBoolean)
{
    return ThreadSafeBoolean.ToBool();
}

FString USyncUtilitiesLibrary::Conv_ThreadSafeBooleanToString(const FThreadSafeBoolean& ThreadSafeBoolean)
{
    return ThreadSafeBoolean.ToString();
}

bool USyncUtilitiesLibrary::ThreadSafeBooleanEqualsThreadSafeBoolean(const FThreadSafeBoolean& A,
                                                                     const FThreadSafeBoolean& B)
{
    return A == B;
}

bool USyncUtilitiesLibrary::ThreadSafeBooleanEqualsBoolean(const FThreadSafeBoolean& A, const bool& B)
{
    return A == B;
}

void USyncUtilitiesLibrary::ThreadSafeBooleanSetBoolean(UPARAM(ref)FThreadSafeBoolean& A, bool B)
{
    A = B;
}

void USyncUtilitiesLibrary::ThreadSafeBooleanSetThreadSafeBoolean(
    UPARAM(ref)FThreadSafeBoolean& A, const FThreadSafeBoolean& B)
{
    A = B;
}

int32 USyncUtilitiesLibrary::Conv_ThreadSafeIntegerToInt32(const FThreadSafeInteger& ThreadSafeInteger)
{
    return ThreadSafeInteger.GetValue();
}

FString USyncUtilitiesLibrary::Conv_ThreadSafeIntegerToString(const FThreadSafeInteger& ThreadSafeInteger)
{
    return ThreadSafeInteger.ToString();
}

bool USyncUtilitiesLibrary::ThreadSafeIntegerEqualsThreadSafeInteger(const FThreadSafeInteger& A,
                                                                     const FThreadSafeInteger& B)
{
    return A == B;
}

bool USyncUtilitiesLibrary::ThreadSafeIntegerEqualsInt32(const FThreadSafeInteger& A, const int32& B)
{
    return A == B;
}

void USyncUtilitiesLibrary::ThreadSafeIntegerSetThreadSafeInteger(
    UPARAM(ref)FThreadSafeInteger& A, const FThreadSafeInteger& B)
{
    A = B;
}

void USyncUtilitiesLibrary::ThreadSafeIntegerSetInteger(UPARAM(ref)FThreadSafeInteger& A, int32 B)
{
    A = B;
}

void USyncUtilitiesLibrary::ThreadSafeIntegerIncrement(UPARAM(ref)FThreadSafeInteger& ThreadSafeInteger)
{
    ThreadSafeInteger.Increment();
}

void USyncUtilitiesLibrary::ThreadSafeIntegerDecrement(UPARAM(ref)FThreadSafeInteger& ThreadSafeInteger)
{
    ThreadSafeInteger.Decrement();
}
