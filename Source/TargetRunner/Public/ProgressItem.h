#pragma once

#include "ProgressItem.generated.h"

//
// Current progress as percent is: CurrentProgress / OfTotalProgress
//
USTRUCT(BlueprintType)
struct FProgressItem
{
	GENERATED_BODY()

public:

	// The section of progress this is related to. Can be mapped to Text for user later.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Code;

	// A descriptive message for the progress.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Message;

	// The current progress expressed as an incrementing integer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CurrentProgress;

	// The total number of items in this progress.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float OfTotalProgress;

public:
	FProgressItem()
	{
		CurrentProgress = 0.0f;
		OfTotalProgress = 1.0f;
	}

	FProgressItem(const FName NewCode, const FString NewMessage, const float Current, const float OfTotal)
	{
		Code = NewCode;
		Message = NewMessage;
		CurrentProgress = Current;
		OfTotalProgress = OfTotal;
	}

	FProgressItem(const WIDECHAR* NewCode, const WIDECHAR * NewMessage, const float Current, const float OfTotal)
	{
		Code = FName(NewCode);
		Message = FString(NewMessage);
		CurrentProgress = Current;
		OfTotalProgress = OfTotal;
	}

	FProgressItem(const WIDECHAR* NewCode, const WIDECHAR* NewMessage, const int32 Current, const int32 OfTotal)
	{
		Code = FName(NewCode);
		Message = FString(NewMessage);
		CurrentProgress = static_cast<float>(Current);
		OfTotalProgress = static_cast<float>(OfTotal);
	}

	FORCEINLINE FProgressItem& Update(const WIDECHAR* NewMessage, const float CurrentIncrement = 1.0f)
	{
		Message = FString(NewMessage);
		CurrentProgress += CurrentIncrement;
		return *this;
	}

	FORCEINLINE FProgressItem& Update(const float CurrentIncrement = 1.0f) 
	{
		CurrentProgress += CurrentIncrement;
		return *this;
	}

	FORCEINLINE FProgressItem& Update(const int32 CurrentIncrement)
	{
		CurrentProgress += static_cast<float>(CurrentIncrement);
		return *this;
	}

	FORCEINLINE FProgressItem& Update(const WIDECHAR* NewMessage, const int32 CurrentIncrement)
	{
		Message = FString(NewMessage);
		CurrentProgress += static_cast<float>(CurrentIncrement);
		return *this;
	}

	FORCEINLINE FProgressItem& Complete(const WIDECHAR* NewMessage)
	{
		Message = FString(NewMessage);
		CurrentProgress = OfTotalProgress;
		return *this;
	}
};
