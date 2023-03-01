// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DPropsType.h"
#include "PlayerDataInterfaceType.h"
#include "DMUpgradeGearCosts.generated.h"


USTRUCT(BlueprintType)
struct FCostItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemGuidHandle ItemGuidHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseNum;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Var1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Var2;

	/**
	 * 装备等级大于等于此值时才会消耗此材料
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinLevel;
};


/**
 * 
 */
UCLASS(BlueprintType)
class DREAM_API UDMUpgradeGearCosts : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCostItem> Normal;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCostItem> Advanced;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCostItem> Rare;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCostItem> Epic;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCostItem> Legendary;

public:

	UFUNCTION(BlueprintPure, Category=UpgradeGearCosts)
	void GetUpgradeCost(int32 GearLevel, int32 ItemGuid, TArray<FAcquisitionCost>& Costs);
	
	UFUNCTION(BlueprintPure, Category=UpgradeGearCosts)
	void GetUpgradeCostHandle(int32 GearLevel, int32 ItemGuid, FCostsHandle& Handle);
};
