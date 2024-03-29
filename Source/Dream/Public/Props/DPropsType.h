#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DPropsType.generated.h"

UENUM(BlueprintType)
enum class EModuleCategory : uint8
{
	C1,
    C2,
    C3,
    C4,
    Max
};

// ItemGuid  int32 (0xFFFF) = [FF -> ItemType] [FF -> 物品ID]
UENUM(BlueprintType)
namespace EItemType
{
	enum Type
	{
		INVALID,
        Weapon UMETA(DisplayName="武器"),
        Module UMETA(DisplayName="模块"),
        Material UMETA(DisplayName="材料"),
        Consumable UMETA(DisplayName="消耗品"),
		Experience UMETA(DisplayName="经验"),
		Ability UMETA(DisplayName="能力"),
		All,
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

	FPropsInfo()
		: PropsQuality(EPropsQuality::Normal),
		PropsIcon(nullptr)
	{};

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Props)
	EPropsQuality PropsQuality;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = ( AllowedClasses="Texture,MaterialInterface" ), Category = Props)
	UObject* PropsIcon;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Props)
	FText PropsName;
};

USTRUCT()
struct FItemDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSoftClassPath ItemClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPropsInfo ItemBaseInfo;

	UClass* GetItemClass() const;
};

USTRUCT(BlueprintType)
struct FItemGuidHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ItemGuid;
};

USTRUCT(BlueprintType)
struct FAttributeRandomItem_Float
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Value;

	UPROPERTY(EditAnywhere)
	float Probability;
};

USTRUCT(BlueprintType)
struct FAttributeRandomItem_Ability
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Meta = (FilterItemType="Ability"))
	FItemGuidHandle Guid;

	UPROPERTY(EditAnywhere)
	float Probability;
};

USTRUCT(BlueprintType)
struct FEquipmentAttributes
{
	GENERATED_BODY()

	FEquipmentAttributes()
        : AttackPower(0),
		  MaxHealth(0),
          CriticalDamage(0),
          CriticalRate(0),
          HealthSteal(0),
          Defense(0),
          DamageReduction(0),
          Penetration(0)
	{
	}

	FEquipmentAttributes& operator+=(const FEquipmentAttributes& RHS);
	FEquipmentAttributes& operator+=(FEquipmentAttributes&& RHS);
	
	FEquipmentAttributes& operator+=(const struct FBaseAttributes& RHS);

	FEquipmentAttributes operator-(const FEquipmentAttributes& RHS) const;
	
	FEquipmentAttributes& operator-=(const FEquipmentAttributes& RHS);

	FEquipmentAttributes(FEquipmentAttributes&& Other) noexcept;
	FEquipmentAttributes(const FEquipmentAttributes& Other);
	
	FEquipmentAttributes& operator=(const FEquipmentAttributes& Other);

	FEquipmentAttributes& operator=(FEquipmentAttributes&& Other) noexcept;

	FEquipmentAttributes& MergeAndSkipPerks(const FEquipmentAttributes& Other);

	bool IsValidNumericalValue(float ErrorTolerance = SMALL_NUMBER) const;
	
	/**
	* 攻击力
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float AttackPower;

	/**
	* 暴击伤害
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float MaxHealth;

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
	* 伤害减免 [暂未使用]
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float DamageReduction;

	/**
	* 穿透 [暂未使用]
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attribute)
	float Penetration;

	/**
	* 装备Perk属性
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attribute)
	TArray<int32> Perks;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	friend FArchive& operator<<(FArchive& Ar, FEquipmentAttributes& Attr);
};

template<>
struct TStructOpsTypeTraits<FEquipmentAttributes> : TStructOpsTypeTraitsBase2<FEquipmentAttributes>
{
	enum
	{
        WithNetSerializer = true
    };
};

/**
 * FEquipmentAttributes 属性句柄
 */
USTRUCT(BlueprintType)
struct FAttributeHandle
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere)
	TFieldPath<FFloatProperty> AttributeProperty;

public:

	bool operator==(const FAttributeHandle& Other) const;
	bool operator!=(const FAttributeHandle& Other) const;

	friend uint32 GetTypeHash(const FAttributeHandle& Other);
};

namespace FEmptyStruct
{
	extern const FPropsInfo EmptyPropsInfo;
	extern const FEquipmentAttributes EmptyAttributes;
	extern const FSoftClassPath EmptySoftClassPath;
	extern const FItemDef EmptyItemDef;
}

namespace ItemUtils
{
	FORCEINLINE EItemType::Type GetItemType(int32 ItemGuid)
	{
		return static_cast<EItemType::Type>(ItemGuid >> 20 & 0xFF);
	}

	FORCEINLINE EPropsQuality GetItemQuality(int32 ItemGuid)
	{
		return static_cast<EPropsQuality>(ItemGuid >> 16 & 0xF);
	}
}