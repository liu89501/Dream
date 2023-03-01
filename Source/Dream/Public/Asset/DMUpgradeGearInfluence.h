// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerDataInterfaceType.h"
#include "DMUpgradeGearInfluence.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class DREAM_API UDMUpgradeGearInfluence : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * 升级装备时会影响的属性
	 */
	UPROPERTY(EditAnywhere)
	TArray<FAttributeHandle> InfluenceProperties;

public:

	void AttemptCalculateAddition(FEquipmentAttributes& Attributes, float UpgradeAdditionMagnitude) const;

	bool IsInfluence(const FAttributeHandle& Attribute) const;
};
