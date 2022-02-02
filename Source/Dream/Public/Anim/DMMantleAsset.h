// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMAnimType.h"
#include "Curves/CurveVector.h"
#include "Engine/DataAsset.h"
#include "DMMantleAsset.generated.h"

USTRUCT()
struct FMantleInformation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UAnimMontage* AnimMontage;

	UPROPERTY(EditAnywhere)
	UCurveVector* PositionCurve;
	
	UPROPERTY(EditAnywhere)
	FVector StartingOffset;

	UPROPERTY(EditAnywhere)
	float LowHeight;

	UPROPERTY(EditAnywhere)
	float LowPlayRate;

	UPROPERTY(EditAnywhere)
	float LowStartPosition;

	UPROPERTY(EditAnywhere)
	float HighHeight;

	UPROPERTY(EditAnywhere)
	float HeightPlayRate;

	UPROPERTY(EditAnywhere)
	float HeightStartPosition;
};

/**
 * 
 */
UCLASS()
class DREAM_API UDMMantleAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	const FMantleInformation& GetMantleInfo(EMantleType MantleType, EOverlayState OverlayState) const;

private:

	UPROPERTY(EditAnywhere)
	FMantleInformation Mantle2mDefault;
	UPROPERTY(EditAnywhere)
	FMantleInformation Mantle1mDefault;
	UPROPERTY(EditAnywhere)
	FMantleInformation Mantle1mRH;
	UPROPERTY(EditAnywhere)
	FMantleInformation Mantle1mLH;
	UPROPERTY(EditAnywhere)
	FMantleInformation Mantle1m2H;
	UPROPERTY(EditAnywhere)
	FMantleInformation Mantle1mBox;
};
