// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DModuleBase.h"
#include "ShootWeapon.h"

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
enum class ETalentCategory : uint8
{
	SniperExpert,
    BlastingGrandmaster,
    Warrior
};

/*
 * 获取玩家信息时 相关的装备是获取已装备在身上的还是其他的
 */
UENUM(BlueprintType)
enum class EGetTaskCondition : uint8
{
	NotAccept,
	UnSubmit,
	Submitted
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Condition)
	int32 TargetValue;

	UPROPERTY()
	int32 CurrentValue;

	UDQuestCondition()
		: TargetValue(0),
		  CurrentValue(0)
	{
	}

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Condition)
	TSubclassOf<AActor> TargetType;

	virtual bool UpdateCondition(const FQuestActionHandle& Handle) override;

	virtual void SerializeItemData(FArchive& Ar) override;
	
};

USTRUCT(BlueprintType)
struct FQuestConditionHandle
{
	GENERATED_BODY()

	FQuestConditionHandle() : QuestCondition(nullptr)
	{
	}

	explicit FQuestConditionHandle(UDQuestCondition* InCondition)
        : QuestCondition(InCondition)
	{
		if (InCondition)
		{
			ConditionClass = InCondition->GetClass()->GetPathName();
		}
	}

	friend FArchive& operator<<(FArchive& Ar, FQuestConditionHandle& A)
	{
		Ar << A.ConditionClass;
	
		if (Ar.IsLoading())
		{
			if (A.QuestCondition == nullptr)
			{
				if (UClass* Class = LoadClass<UDQuestCondition>(nullptr, *A.ConditionClass))
				{
					A.QuestCondition = NewObject<UDQuestCondition>(GetTransientPackage(), Class);
				}
			}
		}

		if (A.QuestCondition)
		{
			A.QuestCondition->SerializeItemData(Ar);
		}
		return Ar;
	}

	UDQuestCondition* operator->() const
	{
		return QuestCondition;
	}

	UDQuestCondition* Get() const;

	UPROPERTY(BlueprintReadOnly)
	UDQuestCondition* QuestCondition;

	UPROPERTY()
	FString ConditionClass;
};


UCLASS(Blueprintable, Abstract, EditInlineNew)
class DREAM_API UItemData : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	int64 ItemId;

	/** 序列化时标识一下具体序列化的是什么数据 */
	uint8 SerializeMark;
	
	int32 PlayerId;

	UItemData() : ItemId(0), SerializeMark(0), PlayerId(0)
	{
	}

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

	UItemDataWeapon()
	{
		SerializeMark = 1;
	}

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

	UItemDataModule()
	{
		SerializeMark = 2;
	}

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

	UItemDataContainer()
	{
		SerializeMark = 3;
	}

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

	UItemDataExperience()
	{
		SerializeMark = 4;
	}

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

	UItemDataMoney()
	{
		SerializeMark = 5;
	}

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

USTRUCT(BlueprintType)
struct FItemDataHandle
{
	GENERATED_BODY()

public:
	
	FItemDataHandle() : ItemData(nullptr)
	{
	}

	explicit FItemDataHandle(UItemData* InItemData) :
		ItemData(InItemData)
	{
		if (InItemData)
		{
			ItemDataClass = InItemData->GetClass()->GetPathName();
		}
	}

	friend FArchive& operator<<(FArchive& Ar, FItemDataHandle& Value)
	{
		Ar << Value.ItemDataClass;

		if (Ar.IsLoading())
		{
			if (Value.ItemData == nullptr)
			{
				if (UClass* DataClass = LoadClass<UItemData>(nullptr, *Value.ItemDataClass))
				{
					Value.ItemData = NewObject<UItemData>(GetTransientPackage(), DataClass);
				}
			}
		}

		if (Value.ItemData)
		{
			Value.ItemData->SerializeItemData(Ar);
		}
		
		return Ar;
	}

	UItemData* Get() const;

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData;

	UPROPERTY()
	FString ItemDataClass;
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
	int32 TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (MetaClass = "DQuestDescription"))
	FSoftObjectPath TaskDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	UDQuestCondition* CompleteCondition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	UItemData* CompletedReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TaskGroupId;

	UPROPERTY(BlueprintReadOnly)
	ETaskMark TaskMark;

	UPROPERTY(BlueprintReadWrite)
	bool bTracking;

	int64 GetSortNum() const
	{
		return TaskId;
	}

	friend FArchive& operator<<(FArchive& Ar, FTaskInformation& A)
	{
		Ar << A.TaskId;
		Ar << A.TaskGroupId;
		Ar << A.TaskDescription;

		FQuestConditionHandle Condition(A.CompleteCondition);
		Ar << Condition;
		A.CompleteCondition = Condition.Get();

		
		FItemDataHandle Reward(A.CompletedReward);
		Ar << Reward;
		A.CompletedReward = Reward.Get();
		
		Ar << A.TaskMark;
		Ar << A.bTracking;
		return Ar;
	}
};

USTRUCT()
struct FNetTaskInformation
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TaskId;

	UPROPERTY()
	ETaskMark TaskMark;

	UPROPERTY()
	bool bTracking;
	
	UPROPERTY()
	int32 ProgressCurrentVal;
	
	UPROPERTY()
	int32 ProgressTargetVal;

	friend FArchive& operator<<(FArchive& Ar, FNetTaskInformation& A)
	{
		Ar << A.TaskId;
		Ar << A.TaskMark;
		Ar << A.bTracking;
		Ar << A.ProgressCurrentVal;
		Ar << A.ProgressTargetVal;
		return Ar;
	}

};

USTRUCT()
struct FSearchTaskParam
{
	GENERATED_BODY()

	UPROPERTY()
	EGetTaskCondition Condition;

	UPROPERTY()
	int32 GroupId;


	FSearchTaskParam()
		: Condition(EGetTaskCondition::NotAccept),
		  GroupId(0)
	{
	}

	FSearchTaskParam(EGetTaskCondition Condition, int32 GroupId)
		: Condition(Condition),
		  GroupId(GroupId)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FSearchTaskParam& A)
	{
		Ar << A.Condition;
		Ar << A.GroupId;
		return Ar;
	}
};

UCLASS()
class UTaskDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FTaskInformation> Tasks;

	bool GetInformationByTaskId(int32 TaskId, FTaskInformation& Information);

	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;
	
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

	friend FArchive& operator<<(FArchive& Ar, FTalentInfo& A)
	{
		Ar << A.TalentId;
		Ar << A.TalentGroupId;
		Ar << A.TalentIndex;
		Ar << A.bLearned;
		Ar << A.TalentClass;
		return Ar;
	}
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

	friend FArchive& operator<<(FArchive& Ar, FPlayerWeapon& Weapon)
	{
		Ar << Weapon.WeaponId;
		Ar << Weapon.WeaponClass;
		Ar << Weapon.bEquipped;
		Ar << Weapon.Index;
		Ar << Weapon.Attributes;
		return Ar;
	}
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

	friend FArchive& operator<<(FArchive& Ar, FPlayerModule& Module)
	{
		Ar << Module.ModuleId;
		Ar << Module.ModuleClass;
		Ar << Module.bEquipped;
		Ar << Module.Category;
		Ar << Module.Attributes;
		return Ar;
	}
};

/** 玩家拥有的各种数据 */
USTRUCT(BlueprintType)
struct FPlayerProperties
{
	GENERATED_BODY()

	FPlayerProperties()
		: Money(0),
		  Level(1),
		  CurrentExperience(0),
		  MaxExperience(0)
	{
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int64 Money;

	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Level;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentExperience;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxExperience;

	friend FArchive& operator<<(FArchive& Ar, FPlayerProperties& A)
	{
		Ar << A.Money;
		Ar << A.Level;
		Ar << A.CurrentExperience;
		Ar << A.MaxExperience;
		return Ar;
	}
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

	friend FArchive& operator<<(FArchive& Ar, FPlayerInfo& A)
	{
		Ar << A.Properties;
		Ar << A.Weapons;
		Ar << A.Modules;
		Ar << A.Talents;
		Ar << A.CharacterMesh;
		return Ar;
	}
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

	friend FArchive& operator<<(FArchive& Ar, FRunServerParameter& A)
	{
		Ar << A.MapAssetPath;
		Ar << A.MapName;
		Ar << A.ModeName;
		return Ar;
	}
};

USTRUCT()
struct FFindServerResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString ServerAddress;

	UPROPERTY()
	int32 ServerState;

	friend FArchive& operator<<(FArchive& Ar, FFindServerResult& A)
	{
		Ar << A.ServerAddress;
		Ar << A.ServerAddress;
		return Ar;
	}
};

USTRUCT()
struct FLaunchNotifyParam
{
	GENERATED_BODY()

	UPROPERTY()
	FString ServerId;

	UPROPERTY()
	int32 ListenPort;


	FLaunchNotifyParam()
		: ListenPort(0)
	{
	}

	FLaunchNotifyParam(const FString& ServerId, int32 ListenPort)
		: ServerId(ServerId),
		  ListenPort(ListenPort)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FLaunchNotifyParam& A)
	{
		Ar << A.ServerId;
		Ar << A.ListenPort;
		return Ar;
	}
};

USTRUCT()
struct FLoginParameter
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlatformName;

	UPROPERTY()
	FString ThirdPartyUserTicket;

	friend FArchive& operator<<(FArchive& Ar, FLoginParameter& A)
	{
		Ar << A.PlatformName;
		Ar << A.ThirdPartyUserTicket;
		return Ar;
	}
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

	friend FArchive& operator<<(FArchive& Ar, FDedicatedServerInformation& A)
	{
		Ar << A.Port;
		Ar << A.MaxPlayers;
		Ar << A.MapName;
		Ar << A.GameModeName;
		return Ar;
	}
};

USTRUCT()
struct FUpdateServerPlayerParam
{
	GENERATED_BODY()

	FUpdateServerPlayerParam() : bIncrementPlayerNum(false)
	{
	}
	
	FUpdateServerPlayerParam(bool InIncrementPlayerNum) : bIncrementPlayerNum(InIncrementPlayerNum)
	{
	}
	
	UPROPERTY()
	bool bIncrementPlayerNum;

	friend FArchive& operator<<(FArchive& Ar, FUpdateServerPlayerParam& A)
	{
		Ar << A.bIncrementPlayerNum;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FStoreInformation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Item)
	TArray<FItemDataHandle> Items;

	friend FArchive& operator<<(FArchive& Ar, FStoreInformation& Value)
	{
		Ar << Value.Items;
		return Ar;
	}
};

USTRUCT()
struct FEquipWeaponParam
{
	GENERATED_BODY()

	UPROPERTY()
	int64 WeaponId;
	UPROPERTY()
	int32 EquipmentIndex;

	FEquipWeaponParam()
		: WeaponId(0),
          EquipmentIndex(0)
	{
	}

	FEquipWeaponParam(int64 WeaponId, int32 EquipmentIndex)
		: WeaponId(WeaponId),
		  EquipmentIndex(EquipmentIndex)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FEquipWeaponParam& A)
	{
		Ar << A.WeaponId;
		Ar << A.EquipmentIndex;
		return Ar;
	}
};

USTRUCT()
struct FEquipModuleParam
{
	GENERATED_BODY()

	UPROPERTY()
	int64 ModuleId;
	UPROPERTY()
	EModuleCategory ModuleCategory;

	FEquipModuleParam()
		: ModuleId(0),
          ModuleCategory(EModuleCategory::C1)
	{
	}

	FEquipModuleParam(int64 ModuleId, EModuleCategory ModuleCategory)
		: ModuleId(ModuleId),
		  ModuleCategory(ModuleCategory)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FEquipModuleParam& A)
	{
		Ar << A.ModuleId;
		Ar << A.ModuleCategory;
		return Ar;
	}
};

USTRUCT()
struct FBuyItemParam
{
	GENERATED_BODY()


	FBuyItemParam()
		: StoreId(0),
          ItemId(0)
	{
	}

	FBuyItemParam(int32 StoreId, int64 ItemId)
		: StoreId(StoreId),
		  ItemId(ItemId)
	{
	}

	UPROPERTY()
	int32 StoreId;
	UPROPERTY()
	int64 ItemId;

	friend FArchive& operator<<(FArchive& Ar, FBuyItemParam& A)
	{
		Ar << A.StoreId;
		Ar << A.ItemId;
		return Ar;
	}
};

struct FAcceptTaskParam
{

	int32 TaskId;
	int32 TaskGroupId;
	int32 ConditionTargetValue;

	FAcceptTaskParam()
		: TaskId(0),
		  TaskGroupId(0),
		  ConditionTargetValue(0)
	{
		
	}

	FAcceptTaskParam(int32 TaskId, int32 TaskGroupId, int32 InConditionTargetValue)
		: TaskId(TaskId),
		  TaskGroupId(TaskGroupId),
		  ConditionTargetValue(InConditionTargetValue)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FAcceptTaskParam& A)
	{
		Ar << A.TaskId;
		Ar << A.TaskGroupId;
		Ar << A.ConditionTargetValue;
		return Ar;
	}
};

USTRUCT()
struct FModifyTrackingParam
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TaskId;
	UPROPERTY()
	bool bTracking;

	friend FArchive& operator<<(FArchive& Ar, FModifyTrackingParam& A)
	{
		Ar << A.TaskId;
		Ar << A.bTracking;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FMulticastDelegateHandle
{
	GENERATED_BODY()

	FDelegateHandle Handle;
};

#if WITH_EDITORONLY_DATA

USTRUCT()
struct FMyTestSerialize
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLoginParameter> ArrSer;

	UPROPERTY()
	float IntArr;

	UPROPERTY()
	TSet<int32> SetTest;

	friend FArchive& operator<<(FArchive& Ar, FMyTestSerialize& A)
	{
		Ar << A.SetTest;
		Ar << A.ArrSer;
		Ar << A.IntArr;
		return Ar;
	}
};

#endif