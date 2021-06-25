#pragma once

#include "CoreMinimal.h"
#include "DPropsType.generated.h"

UENUM(BlueprintType)
enum class EPropsQuality : uint8
{
	Normal,
    Advanced,
    Rare,
    Epic,
    Legendary
};

USTRUCT(BlueprintType)
struct FEquipmentAttributes
{
	GENERATED_BODY()

	static FEquipmentAttributes EmptyAttributes;

	/**
	* 攻击力
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	int32 AttackPower;

	/**
	 * 暴击伤害
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float CriticalDamage;

	/**
	* 暴击率
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float CriticalRate;

	/**
	 * 吸血
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float HealthSteal;

	/**
	 * 防御
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float Defense;

	/**
	 * 伤害减免 (暂时未使用)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float DamageReduction;

	/**
	 * 穿透
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float Penetration;

	/**
	 * 装备Perk属性
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<TSubclassOf<class UDreamGameplayAbility>> Perks;
};

USTRUCT(BlueprintType)
struct FQualityInfo
{
	GENERATED_BODY()

	/** 道具品级名称 */
    UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FText QualityName;

	/** 道具品级的主题色 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FLinearColor QualityThemeColor;
};


USTRUCT(BlueprintType)
struct FPropsInfo
{
	GENERATED_BODY()

	static FPropsInfo EmptyPropsInfo;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Props)
	EPropsQuality PropsQuality;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Props)
	UMaterialInstance* PropsIcon;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Props)
	FText PropsName;

	bool IsValid() const
	{
		return PropsIcon != nullptr && !PropsName.IsEmpty();
	}
};
