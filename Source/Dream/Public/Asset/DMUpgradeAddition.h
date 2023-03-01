// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerDataInterfaceType.h"
#include "DMUpgradeAddition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class DREAM_API UDMUpgradeAddition : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	UCurveFloat* Normal;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* Advanced;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* Rare;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* Epic;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* Legendary;

public:

	float GetAdditionStrength(EPropsQuality Quality, uint8 Level) const;
};
