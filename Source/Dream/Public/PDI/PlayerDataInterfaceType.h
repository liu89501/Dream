// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DModuleBase.h"
#include "DPropsType.h"
#include "Engine/DataTable.h"
#include "PlayerDataInterfaceType.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPDS, Log, All);

UENUM(BlueprintType)
namespace EPDTalentCategory
{
	enum Type
	{
		SniperExpert,
        BlastingGrandmaster,
        Warrior
    };
}

/*
 * 获取玩家信息时 相关的装备是获取已装备在身上的还是其他的
 */
UENUM(BlueprintType)
enum class EGetTaskCondition : uint8
{
	All,
	InProgress,
	Completed,
	UnComplete,
};

/*
 * 获取玩家信息时 相关的装备是获取已装备在身上的还是其他的
 */
UENUM(BlueprintType)
enum class EGetEquipmentCondition : uint8
{
	All,
	Equipped,
	UnEquipped
};


UCLASS(Blueprintable, Abstract, EditInlineNew)
class UItemData : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	int64 ItemId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	int64 ItemPrice;

	virtual void PostInitProperties() override;

	virtual void InitializeProperties();

	virtual UClass* GetItemClass() const;

	virtual int32 GetItemAmount() const;

	virtual EItemType::Type NativeGetItemType() const;

	UFUNCTION(BlueprintPure, Category=ItemData)
	const FPropsInfo& GetPropsInformation() const;

	UFUNCTION(BlueprintPure, Category=ItemData)
	TEnumAsByte<EItemType::Type> GetItemType() const;
};

UCLASS(Abstract)
class UItemDataEquipment : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (EditCondition = "bFixedAttributes"), Category=ItemData)
	FEquipmentAttributes Attributes;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	bool bFixedAttributes;

	virtual void AttemptAssignAttributes() {}
};

UCLASS()
class UItemDataWeapon : public UItemDataEquipment
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (MetaClass = "ShootWeapon"), Category=ItemData)
	FSoftClassPath WeaponClass;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	int32 RateOfFire;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	int32 Magazine;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	FText FireMode;

	virtual void InitializeProperties() override;

	virtual UClass* GetItemClass() const override;

	virtual void AttemptAssignAttributes() override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Weapon;
	}
	
};

UCLASS()
class UItemDataModule : public UItemDataEquipment
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (MetaClass = "DModuleBase"), Category=ItemData)
	FSoftClassPath ModuleClass;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	EModuleCategory Category;

	virtual void InitializeProperties() override;

	virtual UClass* GetItemClass() const override;

	virtual void AttemptAssignAttributes() override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Module;
	}
};

UCLASS()
class UItemDataExperience : public UItemData
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category=ItemData)
	int32 ExperienceAmount;

	virtual UClass* GetItemClass() const override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Other;
	}

	virtual int32 GetItemAmount() const override
	{
		return ExperienceAmount;
	}
};


UCLASS()
class UItemDataMoney : public UItemData
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category=ItemData)
	int32 MoneyAmount;

	virtual UClass* GetItemClass() const override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Other;
	}

	virtual int32 GetItemAmount() const override
	{
		return MoneyAmount;
	}
};



USTRUCT(BlueprintType)
struct FTaskInformation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int64 TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	class UDQuestCondition* CompleteCondition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<UItemData*> CompletedReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsInProgress;
};


USTRUCT(BlueprintType)
struct FTalentInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TalentId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TalentGroupId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TalentIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bLearned;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = ( MetaClass = "DreamGameplayAbility" ))
	FSoftClassPath TalentClass;
};


USTRUCT(BlueprintType)
struct FPlayerWeaponAdd
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = ( MetaClass = "ShootWeapon" ))
	FSoftClassPath WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FEquipmentAttributes Attributes;
};

USTRUCT()
struct FUserExperiencePair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString UniqueID;

	UPROPERTY(EditAnywhere)
	int32 IncreaseExperienceAmount;
};

USTRUCT(BlueprintType)
struct FPlayerWeapon
{
	GENERATED_BODY()

	FPlayerWeapon()
		: bEquipped(false),
		  Index(0),
		  WeaponId(0)
	{
	}

	FPlayerWeapon(UItemDataWeapon* DataWeapon)
		: WeaponClass(DataWeapon->WeaponClass),
		  bEquipped(false),
		  Index(0),
		  WeaponId(0),
		  Attributes(DataWeapon->Attributes)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo, meta = ( MetaClass = "ShootWeapon" ))
	FSoftClassPath WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	bool bEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	int64 WeaponId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	FEquipmentAttributes Attributes;
};

USTRUCT(BlueprintType)
struct FPlayerModule
{
	GENERATED_BODY()

	FPlayerModule()
		: bEquipped(false),
		  ModuleId(0),
		  Category(EModuleCategory::C1)
	{
	}

	FPlayerModule(UItemDataModule* ItemDataModule)
		: ModuleClass(ItemDataModule->ModuleClass),
		bEquipped(false),
		ModuleId(0),
		Category(ItemDataModule->Category),
		Attributes(ItemDataModule->Attributes)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = ( MetaClass = "DModuleBase" ))
	FSoftClassPath ModuleClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int64 ModuleId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EModuleCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FEquipmentAttributes Attributes;
};

USTRUCT(BlueprintType)
struct FPlayerModuleList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerModule> Modules;
};

/** 玩家拥有的各种数据 */
USTRUCT(BlueprintType)
struct FPlayerProperties
{
	GENERATED_BODY()

	FPlayerProperties()
		: Money(0),
		  AvailableTalentPoints(0),
		  Level(1),
		  CurrentExperience(0),
		  MaxExperience(0)
	{
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int64 Money;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 AvailableTalentPoints;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Level;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentExperience;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxExperience;
	
};

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FPlayerWeapon> Weapons;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FPlayerModule> Modules;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FTalentInfo> Talents;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FPlayerProperties Properties;
};

USTRUCT(BlueprintType)
struct FRunServerParameter
{
	GENERATED_USTRUCT_BODY()

	FRunServerParameter() = default;
	FRunServerParameter(const FString& InMapName, const FString& InModeName) :
		MapName(InMapName),
		ModeName(InModeName)
	{
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString MapName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString ModeName;
};


USTRUCT(BlueprintType)
struct FStoreInformation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Item)
	TArray<UItemData*> Items;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Item)
	int64 PlayerMoneyAmount;

};
