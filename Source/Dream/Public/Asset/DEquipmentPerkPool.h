// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "Engine/DataAsset.h"
#include "DEquipmentPerkPool.generated.h"

USTRUCT()
struct FPerkList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FAttributeRandomItem_Ability> Perks;
};

UCLASS(Blueprintable)
class DREAM_API UDPerkPool : public UDataAsset
{
	GENERATED_BODY()

public:
	
	/**
	* 武器的Perk
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FPerkList> PerkGroup;

public:

	void GeneratePerks(TArray<int32>& PerkGuids);
};

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDEquipmentAttributesPool : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> AttackPower;

	/**
	* 生命值
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> MaxHealth;
	
	/**
	* 暴击伤害
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> CriticalDamage;

	/**
	* 暴击率
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> CriticalRate;

	/**
	* 吸血
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> HealthSteal;

	/**
	* 防御
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> Defense;

	/**
	* 伤害减免 (暂时未使用)
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> DamageReduction;

	/**
	* 穿透
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	TArray<FAttributeRandomItem_Float> Penetration;

	UPROPERTY(EditAnywhere, Category = Attribute)
	UDPerkPool* PerkPool;
	
public:

	void GenerateAttributes(FEquipmentAttributes& Attributes);
};
