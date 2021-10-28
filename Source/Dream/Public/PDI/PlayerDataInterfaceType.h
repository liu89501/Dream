// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DModuleBase.h"
#include "PlayerDataInterfaceType.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPDS, Log, All);

class AShootWeapon;

namespace EServerState
{
	enum ServerState
	{
		PENDING,
		IN_PROGRESS
	};
}

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
	UnSubmit,
	Submitted,
	NotAccept
};

/*
 * 获取玩家信息时 相关的装备是获取已装备在身上的还是其他的
 */
UENUM(BlueprintType)
enum class ETaskMark : uint8
{
	NotAccept,
	Accepted,
	Completed,
	Submitted
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

USTRUCT()
struct FQuestAction
{
	GENERATED_BODY()

	virtual ~FQuestAction() = default;
	
	virtual UScriptStruct* GetStructType() const
	{
		return StaticStruct();
	}
};

USTRUCT()
struct FQuestAction_KilledTarget : public FQuestAction
{
	GENERATED_BODY()

	FQuestAction_KilledTarget() : TargetClass(nullptr)
	{
		
	}

	explicit FQuestAction_KilledTarget(UClass* InTargetClass)
        : TargetClass(InTargetClass)
	{
	}

	virtual UScriptStruct* GetStructType() const override
	{
		return StaticStruct();
	}

	UClass* GetTargetClass() const
	{
		return TargetClass;
	}

private:

	UPROPERTY()
	UClass* TargetClass;
};


USTRUCT()
struct FQuestActionHandle
{
	GENERATED_BODY()

public:

	FQuestActionHandle() : ActionData(nullptr)
	{
	}

	FQuestActionHandle(FQuestAction* InData)
	{
		ActionData = TSharedPtr<FQuestAction>(InData);
	}
	
	FQuestAction* GetData() const
	{
		return ActionData.Get();
	}

private:

	TSharedPtr<FQuestAction> ActionData;
};

UCLASS(Blueprintable)
class DREAM_API UDQuestDescription : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText QuestContent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* QuestIcon;
};

UCLASS(Abstract, EditInlineNew)
class DREAM_API UDQuestCondition : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Meta = (DisplayName="GetProgressPercent", ScriptName="GetProgressPercent"), Category=QuestCondition)
    float BP_GetQuestProgressPercent() const;

	virtual float GetQuestProgressPercent() const;

	virtual bool UpdateCondition(const struct FQuestActionHandle& Handle);

	virtual bool IsCompleted() const;

	virtual void SerializeItemData(FArchive& Ar);
};

UCLASS()
class DREAM_API UDQuestCondition_KillTarget : public UDQuestCondition
{
	GENERATED_BODY()
	
public:

	UDQuestCondition_KillTarget()
		: CurrentKilled(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 KillNum;

	UPROPERTY()
	int32 CurrentKilled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> TargetType;

	virtual bool UpdateCondition(const FQuestActionHandle& Handle) override;

	virtual float GetQuestProgressPercent() const override;

	virtual bool IsCompleted() const override;

	virtual void SerializeItemData(FArchive& Ar) override;
	
};

USTRUCT()
struct FQuestConditionHandle
{
	GENERATED_BODY()

	FQuestConditionHandle() : QuestCondition(nullptr), ConditionClass(nullptr)
	{
	}

	explicit FQuestConditionHandle(UDQuestCondition* InCondition)
        : QuestCondition(InCondition), ConditionClass(InCondition->GetClass())
	{
	}

	bool Serialize(FArchive& Ar);

	UDQuestCondition* Get() const;

private:

	UPROPERTY()
	UDQuestCondition* QuestCondition;

	UPROPERTY()
	UClass* ConditionClass;
};

template<>
struct TStructOpsTypeTraits< FQuestConditionHandle > : TStructOpsTypeTraitsBase2< FQuestConditionHandle >
{
	enum
	{
		WithSerializer = true,
    };
};


UCLASS(Blueprintable, Abstract, EditInlineNew)
class DREAM_API UItemData : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	int64 ItemId;

	// 如果要做服务器版本的 这里应该需要加一个唯一ID --- 现在先不考虑。
	//FString UniqueNetId;

	virtual UClass* GetItemClass() const;

	virtual int32 GetItemAmount() const;

	virtual EItemType::Type NativeGetItemType() const;

	/** 校验这个ItemData是否有效， 对于ItemDataContainer则是校验元素是否为空 */
	virtual bool IsValidData() const;

	virtual void SerializeItemData(FArchive& Ar);

	virtual void InitializeExtraProperties();

	UFUNCTION(BlueprintPure, Category=ItemData)
	const FPropsInfo& GetPropsInformation() const;

	UFUNCTION(BlueprintPure, Category=ItemData)
	TEnumAsByte<EItemType::Type> GetItemType() const;
	
};

UCLASS(Abstract)
class DREAM_API UItemDataTradable : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	int64 ItemPrice;

	virtual void SerializeItemData(FArchive& Ar) override;
};

UCLASS(Abstract)
class DREAM_API UItemDataEquipment : public UItemDataTradable
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (EditCondition = "bFixedAttributes"), Category=ItemData)
	FEquipmentAttributes Attributes;
	
	UPROPERTY(EditAnywhere, Category=ItemData)
	bool bFixedAttributes;

	virtual void SerializeItemData(FArchive& Ar) override;
	
	virtual void AttemptAssignAttributes() {}
};

UCLASS()
class DREAM_API UItemDataWeapon : public UItemDataEquipment
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	TSubclassOf<AShootWeapon> WeaponClass;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	int32 RateOfFire;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	int32 Magazine;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	FText FireMode;

	virtual void SerializeItemData(FArchive& Ar) override;

	virtual UClass* GetItemClass() const override;

	virtual void AttemptAssignAttributes() override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Weapon;
	}

	virtual void InitializeExtraProperties() override;
};

UCLASS()
class DREAM_API UItemDataModule : public UItemDataEquipment
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	TSubclassOf<UDModuleBase> ModuleClass;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	EModuleCategory Category;

	virtual void SerializeItemData(FArchive& Ar) override;

	virtual UClass* GetItemClass() const override;

	virtual void AttemptAssignAttributes() override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Module;
	}

	virtual void InitializeExtraProperties() override;
};

UCLASS()
class DREAM_API UItemDataContainer : public UItemData
{
	GENERATED_BODY()

	public:

	virtual void SerializeItemData(FArchive& Ar) override;

	UItemData* GetNextItem();

	void AddItem(UItemData* ItemData);

	void SetItems(const TArray<UItemData*>& NewItems);

	virtual bool IsValidData() const override;

	const TArray<UItemData*>& GetItems() const
	{
		return Items;
	}

#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

	protected:

	UPROPERTY(EditAnywhere, Instanced, Category=ItemData)
	TArray<UItemData*> Items;

	private:
	
	int32 ItemIndex;
};

UCLASS()
class DREAM_API UItemDataExperience : public UItemData
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

	virtual void SerializeItemData(FArchive& Ar) override;
};


UCLASS()
class DREAM_API UItemDataMoney : public UItemData
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category=ItemData)
	int32 MoneyAmount;

	virtual UClass* GetItemClass() const override;

	virtual void SerializeItemData(FArchive& Ar) override;

	virtual EItemType::Type NativeGetItemType() const override
	{
		return EItemType::Other;
	}

	virtual int32 GetItemAmount() const override
	{
		return MoneyAmount;
	}
};

USTRUCT()
struct FItemDataHandle
{
	GENERATED_BODY()

public:
	
	FItemDataHandle() : ItemData(nullptr), ItemDataClass(nullptr)
	{
	}

	explicit FItemDataHandle(UItemData* InItemData) : ItemData(InItemData), ItemDataClass(InItemData->GetClass())
	{
	}

	bool Serialize(FArchive& Ar);

	UItemData* Get() const;

private:

	UPROPERTY()
	UItemData* ItemData;

	UPROPERTY()
	UClass* ItemDataClass;
};

template<>
struct TStructOpsTypeTraits< FItemDataHandle > : TStructOpsTypeTraitsBase2< FItemDataHandle >
{
	enum
	{
		WithSerializer = true,
    };
};

struct FItemDataRange
{

public:

	FItemDataRange(const FItemDataHandle& Handle)
	{
		Init(Handle.Get());
	}
	
    FItemDataRange(UItemData* ItemData)
	{
		Init(ItemData);
	}

	FORCEINLINE TArray<UItemData*>::RangedForIteratorType begin() { return AllItems.begin(); }
	FORCEINLINE TArray<UItemData*>::RangedForConstIteratorType begin() const { return AllItems.begin(); }
	FORCEINLINE TArray<UItemData*>::RangedForIteratorType end  () { return AllItems.end(); }
	FORCEINLINE TArray<UItemData*>::RangedForConstIteratorType end  () const { return AllItems.end(); }
	
private:

	void Init(UItemData* ItemData)
	{
		if (UItemDataContainer* ItemDataContainer = Cast<UItemDataContainer>(ItemData))
		{
			AllItems = ItemDataContainer->GetItems();
		}
		else
		{
			AllItems.Add(ItemData);
		}
	}

	TArray<UItemData*> AllItems;
};

USTRUCT(BlueprintType)
struct FTaskInformation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (MetaClass = "DQuestDescription"))
	FSoftObjectPath TaskDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	UDQuestCondition* CompleteCondition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	UItemData* CompletedReward;

	UPROPERTY(BlueprintReadOnly)
	ETaskMark TaskMark;

	int64 GetSortNum() const
	{
		return TaskId;
	}
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	TSubclassOf<AShootWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	bool bEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponInfo)
	int32 Index;

	UPROPERTY(BlueprintReadOnly, Category = WeaponInfo)
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UDModuleBase> ModuleClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEquipped;

	UPROPERTY(BlueprintReadOnly)
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

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerWeapon> Weapons;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerModule> Modules;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTalentInfo> Talents;

	UPROPERTY(BlueprintReadOnly)
	FPlayerProperties Properties;

	UPROPERTY(BlueprintReadOnly)
	FSoftObjectPath CharacterMesh;
};

USTRUCT(BlueprintType)
struct FRunServerParameter
{
	GENERATED_USTRUCT_BODY()

	FRunServerParameter() = default;
	FRunServerParameter(const FString& InMapName, const FString& InModeName, const FString& InMapAssetPath) :
		MapAssetPath(InMapAssetPath),
		MapName(InMapName),
		ModeName(InModeName)
	{
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString MapAssetPath;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString MapName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString ModeName;
};

USTRUCT()
struct FFindServerResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString ServerAddress;

	UPROPERTY()
	int32 ServerState;
};

USTRUCT()
struct FDedicatedServerInformation
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Port;
	UPROPERTY()
	int32 MaxPlayers;
	UPROPERTY()
	FString MapName;
	UPROPERTY()
	FString GameModeName;
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

USTRUCT(BlueprintType)
struct FMulticastDelegateHandle
{
	GENERATED_BODY()

	FDelegateHandle Handle;

};

