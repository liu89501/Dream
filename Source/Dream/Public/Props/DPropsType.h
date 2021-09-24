#pragma once

#include "CoreMinimal.h"
#include "DPropsType.generated.h"

UENUM(BlueprintType)
namespace EItemType
{
	enum Type
	{
		INVALID,
        Weapon UMETA(DisplayName="武器"),
        Module UMETA(DisplayName="模块"),
        Sundries UMETA(DisplayName="杂物"),
		Other UMETA(DisplayName="其他")
    };
}


UENUM(BlueprintType)
enum class ERewardNotifyMode : uint8
{
	Primary UMETA(ToolTip="这是指从屏幕左边醒目的弹出"),
	Secondary UMETA(ToolTip="这是指显示在屏幕右边的一个小提示")
};

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

USTRUCT(BlueprintType)
struct FPerkPool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = ( MetaClass = "DreamGameplayAbility" ))
	TArray<FSoftClassPath> Perks;

	FPerkPool& operator+=(const FPerkPool& Rhs)
	{
		Perks.Append(Rhs.Perks);
		return *this;
	}
};

USTRUCT(BlueprintType)
struct FEquipmentAttributes
{
	GENERATED_BODY()


	FEquipmentAttributes()
        : AttackPower(0),
          CriticalDamage(0),
          CriticalRate(0),
          HealthSteal(0),
          Defense(0),
          DamageReduction(0),
          Penetration(0)
	{
	}

	FEquipmentAttributes& operator+=(const FEquipmentAttributes& RHS);

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = ( MetaClass = "DreamGameplayAbility" ), Category = Attribute)
	TArray<FSoftClassPath> Perks;
};

USTRUCT(BlueprintType)
struct FEquipmentAttributesAssign
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<int32> AttackPower;

	/**
	* 暴击伤害
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<float> CriticalDamage;

	/**
	* 暴击率
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<float> CriticalRate;

	/**
	* 吸血
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<float> HealthSteal;

	/**
	* 防御
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<float> Defense;

	/**
	* 伤害减免 (暂时未使用)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<float> DamageReduction;

	/**
	* 穿透
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<float> Penetration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	TArray<FPerkPool> Perks;

	static FEquipmentAttributes AssignAttributes(const FEquipmentAttributesAssign& AttrAssign);
};

namespace FEmptyStruct
{
	extern const FPropsInfo EmptyPropsInfo;
	extern const FEquipmentAttributes EmptyAttributes;
	extern const FSoftClassPath EmptySoftClassPath;
}