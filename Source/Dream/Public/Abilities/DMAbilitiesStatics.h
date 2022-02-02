// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DMAbilitiesStatics.generated.h"


/**
 * 
 */
UCLASS()
class DREAM_API UDMAbilitiesStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * 根据防御力获取实际的伤害抗性百分比
	 */
	UFUNCTION(BlueprintPure, Category = DMAbilitiesStatics)
	static float GetDamageResist(float Defense);

	/**
	 * 根据回复属性值获取生命恢复的等待时间
	 */
	UFUNCTION(BlueprintPure, Category = DMAbilitiesStatics)
	static float GetHealthRecoveryTime(float Recovery);

	/**
	 * 根据等级差获取伤害抗性百分比
	 */
	static float GetDamageResistFromLevelDiff(int32 LevelDiff);
	
};

