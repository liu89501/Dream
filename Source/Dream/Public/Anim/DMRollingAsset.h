// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMCharacterType.h"
#include "Engine/DataAsset.h"
#include "DMRollingAsset.generated.h"


/**
 * 
 */
UCLASS()
class DREAM_API UDMRollingAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UAnimMontage* GetRollingMontage(ERollingDirection Direction) const;

private:

	UPROPERTY(EditAnywhere)
	UAnimMontage* Forward;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* ForwardLeft;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* ForwardRight;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* Left;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* Right;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* Backward;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* BackwardLeft;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* BackwardRight;
};
