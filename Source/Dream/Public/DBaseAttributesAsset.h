// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DBaseAttributesAsset.generated.h"

USTRUCT(BlueprintType)
struct FBaseAttributes
{
	GENERATED_BODY()


	FBaseAttributes()
        : MaxHealth(0),
		  AttackPower(0),
          CriticalDamage(0),
          CriticalRate(0),
          HealthSteal(0),
          Defense(0)
	{
	}

	/**
	 * 最大生命值
	 */
	UPROPERTY(EditAnywhere, Category = Attribute)
	float MaxHealth;

	/**
	* 攻击力
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	int32 AttackPower;

	/**
	* 暴击伤害
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	float CriticalDamage;

	/**
	* 暴击率
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	float CriticalRate;

	/**
	* 吸血
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	float HealthSteal;

	/**
	* 防御
	*/
	UPROPERTY(EditAnywhere, Category = Attribute)
	float Defense;

	FBaseAttributes& operator*(const float& Scale)
	{
		MaxHealth = MaxHealth * (Scale + 1);
		AttackPower = AttackPower * (Scale + 1);
		CriticalDamage = CriticalDamage * (Scale + 1);
		CriticalRate = CriticalRate * (Scale + 1);
		HealthSteal = HealthSteal * (Scale + 1);
		Defense = Defense * (Scale + 1);
		return *this;
	}
};

/**
 * 
 */
UCLASS()
class DREAM_API UDBaseAttributesAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 *  角色的基础属性
	 */
	UPROPERTY(EditAnywhere)
	FBaseAttributes BaseAttributes;

	UPROPERTY(EditAnywhere, Meta = (MinClamp = 0, MaxClamp = 0.5))
	float IncrementPerLevel;
	
};
