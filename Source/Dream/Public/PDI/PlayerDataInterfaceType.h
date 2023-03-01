// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "DreamType.h"
#include "PlayerDataInterfaceType.generated.h"

#define Query_Cond_Used (EQueryCondition::Weapon | EQueryCondition::Weapon_EquippedOnly | \
                                        EQueryCondition::Module | EQueryCondition::Module_EquippedOnly | EQueryCondition::Skin)

#define Query_Cond_All (EQueryCondition::Weapon | EQueryCondition::Module | EQueryCondition::Skin | EQueryCondition::Materials)

enum class ETaskCondGUID : int32
{
	None,
	KILL_TARGET,
	EVENT
};

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
	Submitted,
	Tracking
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
namespace EQueryCondition
{
	enum Condition
	{
		Weapon = 0x1, // 是否查询武器
        Weapon_EquippedOnly = 0x2, // 查询武器是已装备的
        Module = 0x4,	
        Module_EquippedOnly = 0x8,
        Materials = 0x10,
        Skin = 0x20
    };
}



struct FQuestActionBase
{
public:

	FQuestActionBase() : ActionCondGUID(ETaskCondGUID::None)
	{
	}
	
	explicit FQuestActionBase(ETaskCondGUID InCondGUID) : ActionCondGUID(InCondGUID)
	{
	}

	virtual ~FQuestActionBase()
	{
	}

	virtual void SerializeData(FArchive& Ar)
	{
		Ar << ActionCondGUID;
	}

	ETaskCondGUID ActionCondGUID;
};

struct FQuestAction_KilledTarget : FQuestActionBase
{
	explicit FQuestAction_KilledTarget(EPawnType InTargetPawn)
		: FQuestActionBase(ETaskCondGUID::KILL_TARGET)
		, TargetPawn(InTargetPawn)
	{
	}

	virtual void SerializeData(FArchive& Ar) override;

	EPawnType GetTargetPawnType() const
	{
		return TargetPawn;
	}

private:

	EPawnType TargetPawn;
};

struct FQuestAction_Event : FQuestActionBase
{
	explicit FQuestAction_Event(const FName& InEventName)
		: FQuestActionBase(ETaskCondGUID::EVENT)
		, EventName(InEventName)
	{
	}

	virtual void SerializeData(FArchive& Ar) override;

private:

	FName EventName;
};


struct FQuestActionHandle
{
public:

	FQuestActionHandle() : PlayerId(0), ActionData(nullptr)
	{
	}

	FQuestActionHandle(int32 InPlayerId, TSharedPtr<FQuestActionBase> InData)
		: PlayerId(InPlayerId),
		  ActionData(InData)
	{
	}

	FQuestActionBase* GetData() const
	{
		return ActionData.Get();
	}

	friend FArchive& operator<<(FArchive& Ar, FQuestActionHandle& A)
	{
		Ar << A.PlayerId;
		A.ActionData->SerializeData(Ar);
		return Ar;
	}

private:

	int32 PlayerId;

	TSharedPtr<FQuestActionBase> ActionData;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPropsQuality TaskQuality;
};

UCLASS(Abstract, EditInlineNew)
class DREAM_API UDQuestCondition : public UObject
{
	GENERATED_BODY()

public:

	ETaskCondGUID ConditionGUID;

	UPROPERTY(BlueprintReadOnly, Category=Condition)
	bool bCompleted;

	UFUNCTION(BlueprintPure, Meta = (DisplayName="GetProgressPercent", ScriptName="GetProgressPercent"), Category=QuestCondition)
	float BP_GetQuestProgressPercent() const;
	
	UFUNCTION(BlueprintPure, Meta = (DisplayName="GetConditionDesc", ScriptName="GetConditionDesc"), Category=QuestCondition)
	FText BP_GetConditionDesc() const;

	UFUNCTION(BlueprintCallable, Category=QuestCondition)
	bool IsCompleted() const;

	virtual FText NativeGetConditionDesc() const;
	
	virtual float GetQuestProgressPercent() const;

	virtual void SerializeData(FArchive& Ar);

	static UDQuestCondition* Decode(FArchive& Ar);
};

UCLASS(Abstract)
class DREAM_API UDQuestCondition_Count : public UDQuestCondition
{
	GENERATED_BODY()

public:

	virtual float GetQuestProgressPercent() const override;

	virtual void SerializeData(FArchive& Ar) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Condition)
	int32 TargetValue;

	UPROPERTY()
	int32 CurrentValue;
};

UCLASS()
class DREAM_API UDQuestCondition_KillTarget : public UDQuestCondition_Count
{
	GENERATED_BODY()

public:

	UDQuestCondition_KillTarget()
	{
		ConditionGUID = ETaskCondGUID::KILL_TARGET;
	}

	virtual FText NativeGetConditionDesc() const override;

	virtual void SerializeData(FArchive& Ar) override;

	UPROPERTY(EditAnywhere, Category=Condition)
	EPawnType TargetType;
};


UCLASS()
class DREAM_API UDQuestCondition_Event : public UDQuestCondition_Count
{
	GENERATED_BODY()

public:

	UDQuestCondition_Event()
	{
		ConditionGUID = ETaskCondGUID::EVENT;
	}

	virtual float GetQuestProgressPercent() const override;

	virtual FText NativeGetConditionDesc() const override;

	virtual void SerializeData(FArchive& Ar) override;
	
	UPROPERTY(EditAnywhere, Category=Condition)
	FName EventName;
};

USTRUCT(BlueprintType)
struct FQuestConditionHandle
{
	GENERATED_BODY()

	FQuestConditionHandle()
	{
	}

	explicit FQuestConditionHandle(const TArray<UDQuestCondition*>& InConditions)
		: Conditions(InConditions)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FQuestConditionHandle& Other);

	UPROPERTY(BlueprintReadOnly)
	TArray<UDQuestCondition*> Conditions;
};

/** ================================================ 物品相关 ================================================ */
/** ================================================ ↓↓↓↓↓↓ ================================================ */

#define IM_None 0
#define IM_Equipment 1
#define IM_Simple 2

struct FItem
{

public:

	FItem(uint8 InStructMark)
		: ItemGuid(0)
		, StructMark(InStructMark)
	{
	}
	
	FItem(int32 InItemGuid, uint8 InStructMark)
		: ItemGuid(InItemGuid)
		, StructMark(InStructMark)
	{
	}
	
	virtual ~FItem() = default;

	virtual int32 GetItemNum() { return 0; }

	virtual void Serialize(FArchive& Ar);

	int32 GetItemGuid() const
	{
		return ItemGuid;
	}

	uint8 GetStructMark()
	{
		return StructMark;
	}

private:

	int32 ItemGuid;

	uint8 StructMark;
};

/**
 * 装备物品
 */
struct FItemEquipment : FItem
{

public:

	FItemEquipment()
		: FItem(IM_Equipment)
		, GearLevel(0)
	{
	}
	
	FItemEquipment(int32 InItemGuid)
		: FItem(InItemGuid, IM_Equipment)
		, GearLevel(0)
	{
	}

	FItemEquipment(int32 InItemGuid, int32 InGearLevel, const FEquipmentAttributes& InAttributes)
		: FItem(InItemGuid, IM_Equipment)
		, Attributes(InAttributes)
		, GearLevel(InGearLevel)
	{
	}

	virtual int32 GetItemNum() override
	{
		return 1;
	}

	virtual void Serialize(FArchive& Ar) override;

public:
	
	FEquipmentAttributes Attributes;

	uint8 GearLevel;
};

/**
 * 简单物品，一般指那种没有属性的，比如：材料
 */
struct FItemSimple : FItem
{

public:

	FItemSimple()
		: FItem(IM_Simple)
		, ItemNum(0)
	{
	}

	explicit FItemSimple(int32 InItemGuid)
		: FItem(InItemGuid, IM_Equipment)
		, ItemNum(0)
	{
	}

	FItemSimple(int32 ItemGuid, int32 Num)
		: FItem(ItemGuid, IM_Simple)
		, ItemNum(Num)
	{
	}

	virtual int32 GetItemNum() override
	{
		return ItemNum;
	}

	virtual void Serialize(FArchive& Ar) override;

public:
	
	int32 ItemNum;
};

/**
 * helper
 */
namespace FItemHelper
{
	TSharedPtr<FItem> Decode(FArchive& Ar);
}


USTRUCT(BlueprintType)
struct DREAM_API FItemHandle
{
	GENERATED_BODY()

public:

	FItemHandle()
	{
	}
	
	FItemHandle(TSharedPtr<FItem> InItem) : Item(InItem)
	{
	}

	TSharedPtr<FItem> Get()
	{
		return Item;
	}

	FItem* operator->() const
	{
		return Item.Get();
	}

	bool IsValid() const
	{
		return Item.IsValid();
	}

	friend FArchive& operator<<(FArchive& Ar, FItemHandle& Other)
	{
		if (Ar.IsSaving())
		{
			if (Other.IsValid())
			{
				Other->Serialize(Ar);
			}
			else
			{
				int32 EmptyGuid = 0;
				Ar << EmptyGuid;
			}
		}
		else
		{
			Other.Item = FItemHelper::Decode(Ar);
		}
		return Ar;
	}

private:

	TSharedPtr<FItem> Item;
};


USTRUCT(BlueprintType)
struct DREAM_API FItemListHandle
{
	GENERATED_BODY()

public:

	FItemListHandle()
	{
	}

	explicit FItemListHandle(const TArray<TSharedPtr<FItem>>& InItems) : Items(InItems)
	{
	}

	bool IsNotEmpty() const
	{
		return Items.Num() > 0;
	}

	void AddItem(TSharedPtr<FItem> Item)
	{
		Items.Add(Item);
	}

	void AddItems(const TArray<TSharedPtr<FItem>>& NewItems)
	{
		Items.Append(NewItems);
	}
	
	void SetItems(const TArray<TSharedPtr<FItem>>& NewItems)
	{
		Items = NewItems;
	}

	FORCEINLINE TArray<TSharedPtr<FItem>>::RangedForIteratorType begin()				{ return Items.begin(); }
	FORCEINLINE TArray<TSharedPtr<FItem>>::RangedForConstIteratorType begin() const	{ return Items.begin(); }
	FORCEINLINE TArray<TSharedPtr<FItem>>::RangedForIteratorType end  ()				{ return Items.end(); }
	FORCEINLINE TArray<TSharedPtr<FItem>>::RangedForConstIteratorType end  () const	{ return Items.end(); }

	friend FArchive& operator<<(FArchive& Ar, FItemListHandle& R)
	{
		int32 Num = R.Items.Num();
		Ar << Num;

		R.Items.SetNumZeroed(Num);
		
		for (int32 N = 0; N < Num; N++)
		{
			if (Ar.IsSaving())
			{
				R.Items[N]->Serialize(Ar);
			}
			else
			{
				R.Items[N] = FItemHelper::Decode(Ar);
			}
		}
		return Ar;
	}

private:

	TArray<TSharedPtr<FItem>> Items;
};

struct FItemListParam
{

public:

	FItemListParam()
		: PlayerId(0)
	{
	}

	FItemListParam(int32 InPlayerId, const FItemListHandle& InListHandle)
		: PlayerId(InPlayerId),
		  ListHandle(InListHandle)
	{
	}
	
	FItemListParam(int32 InPlayerId, FItemListHandle&& InListHandle)
		: PlayerId(InPlayerId),
		  ListHandle(MoveTemp(InListHandle))
	{
	}
	
	FItemListParam(int32 InPlayerId, TSharedPtr<FItem> InItem)
		: PlayerId(InPlayerId)
	{
		ListHandle.AddItem(InItem);
	}

	FORCEINLINE bool IsNotEmpty() const
	{
		return ListHandle.IsNotEmpty();
	}

	friend FArchive& operator<<(FArchive& Ar, FItemListParam& Other)
	{
		Ar << Other.PlayerId;
		Ar << Other.ListHandle;
		return Ar;
	}

public:

	int32 PlayerId;
	
	FItemListHandle ListHandle;
};


USTRUCT(BlueprintType)
struct FTaskInProgressInfo
{
	GENERATED_BODY()
	
	
	UPROPERTY(BlueprintReadOnly)
	int64 PtId;

	UPROPERTY(BlueprintReadWrite)
	FQuestConditionHandle Condition;

	friend FArchive& operator<<(FArchive& Ar, FTaskInProgressInfo& R)
	{
		Ar << R.PtId;
		Ar << R.Condition;
		return Ar;
	}
};

struct FQueryPlayerParam
{
	int32 PlayerId;

	int32 Condition;

	friend FArchive& operator<<(FArchive& Ar, FQueryPlayerParam& R)
	{
		Ar << R.PlayerId;
		Ar << R.Condition;
		return Ar;
	}
};


/** ================================================ ↑↑↑↑↑↑ ================================================ */
/** ================================================ 物品相关 ================================================ */



/** ================================================ ********** ================================================ */
/** ================================================ 物品蓝图相关 ================================================ */

UCLASS(Blueprintable, Abstract, EditInlineNew)
class DREAM_API UItemData : public UObject
{
	GENERATED_BODY()

public:

	/** ItemGuid */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	FItemGuidHandle GuidHandle;

	virtual TSharedPtr<FItem> MakeItemStruct() { return nullptr; }

};

UCLASS()
class DREAM_API UItemDataEquipment : public UItemData
{
	GENERATED_BODY()

public:

	virtual TSharedPtr<FItem> MakeItemStruct() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	FEquipmentAttributes Attributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (ClampMin = 0, ClampMax = 100), Category=ItemData)
	int32 Level;
};


UCLASS()
class DREAM_API UItemDataEquipment_Random : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=ItemData)
	class UDEquipmentAttributesPool* AttrPool;

	UPROPERTY(EditAnywhere, Category=ItemData)
	FRangeRandomInt LevelRange;

public:

	virtual TSharedPtr<FItem> MakeItemStruct() override;
};


UCLASS()
class DREAM_API UItemDataNumericalValue : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	int32 Value;

public:

	virtual TSharedPtr<FItem> MakeItemStruct() override;
};

UCLASS()
class DREAM_API UItemDataNumericalValue_Random : public UItemData
{
	GENERATED_BODY()

public:

	/** 按范围随机取得一个数值 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	FRangeRandomInt RangeValue;

public:

	virtual TSharedPtr<FItem> MakeItemStruct() override;
};

/** ================================================ 物品蓝图相关 ================================================ */
/** ================================================ ********** ================================================ */

USTRUCT(BlueprintType)
struct FAcquisitionCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ItemGuid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CostAmount;

	friend FArchive& operator<<(FArchive& Ar, FAcquisitionCost& R)
	{
		Ar << R.ItemGuid;
		Ar << R.CostAmount;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FCostsHandle
{
	GENERATED_BODY()

	TSharedPtr<TArray<FAcquisitionCost>> Costs;
};


USTRUCT(BlueprintType)
struct FTaskInformation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 TaskId;

	UPROPERTY(BlueprintReadOnly)
	FSoftObjectPath TaskDescription;

	UPROPERTY(BlueprintReadWrite)
	FQuestConditionHandle Condition;

	UPROPERTY(BlueprintReadWrite)
	FItemListHandle CompletedReward;

	UPROPERTY(BlueprintReadOnly)
	int32 TaskGroupId;

	UPROPERTY(BlueprintReadOnly)
	ETaskMark TaskMark;

	UPROPERTY(BlueprintReadWrite)
	bool bTracking;

	friend FArchive& operator<<(FArchive& Ar, FTaskInformation& A);
};

USTRUCT()
struct FSearchTaskParam
{
	GENERATED_BODY()

	UPROPERTY()
	EGetTaskCondition Condition;

	UPROPERTY()
	int32 GroupId;

	UPROPERTY()
	int32 Page;


	FSearchTaskParam()
		: Condition(EGetTaskCondition::NotAccept),
		  GroupId(0),
		  Page(0)
	{
	}

	FSearchTaskParam(int32 InPage, EGetTaskCondition Condition, int32 GroupId)
		: Condition(Condition),
		  GroupId(GroupId),
		  Page(InPage)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FSearchTaskParam& A)
	{
		Ar << A.Condition;
		Ar << A.GroupId;
		Ar << A.Page;
		return Ar;
	}
};


struct FDecomposeParam
{
	FDecomposeParam()
		: ItemId(0)
		, ItemGuid(0)
	{
	}

	FDecomposeParam(int64 InItemId, int32 InItemGuid)
		: ItemId(InItemId),
		  ItemGuid(InItemGuid)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FDecomposeParam& Other)
	{
		Ar << Other.ItemGuid;
		Ar << Other.ItemId;
		return Ar;
	}

	int64 ItemId;

	int32 ItemGuid;
};

USTRUCT(BlueprintType)
struct FSearchTaskResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FTaskInformation> Tasks;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalPage;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalNum;

	friend FArchive& operator<<(FArchive& Ar, FSearchTaskResult& A)
	{
		Ar << A.TotalPage;
		Ar << A.TotalNum;
		Ar << A.Tasks;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FTalentInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TalentId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TalentIndex;

	UPROPERTY(BlueprintReadOnly)
	bool bLearned;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UDreamGameplayAbility> TalentClass;
};

USTRUCT()
struct FTalentList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FTalentInfo> Talents;
};

UCLASS()
class UTalentDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FTalentInfo> Talents;
};

UCLASS()
class UTalentAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FTalentInfo> Talent_Warrior;
	
	UPROPERTY(EditAnywhere)
	TArray<FTalentInfo> Talent_SniperExpert;

	UPROPERTY(EditAnywhere)
	TArray<FTalentInfo> Talent_BlastingGrandmaster;
};

USTRUCT(BlueprintType)
struct FPlayerWeapon
{
	GENERATED_BODY()

	FPlayerWeapon()
		: ItemGuid(0),
		  bEquipped(false),
		  Index(0),
		  WeaponId(0),
		  WeaponLevel(0)
	{
	}

	UPROPERTY(BlueprintReadOnly)
	int32 ItemGuid;

	UPROPERTY(BlueprintReadOnly)
	bool bEquipped;

	UPROPERTY(BlueprintReadOnly)
	int32 Index;

	UPROPERTY(BlueprintReadOnly)
	int64 WeaponId;

	UPROPERTY(BlueprintReadOnly)
	FEquipmentAttributes Attributes;

	UPROPERTY()
	uint8 WeaponLevel;

	friend FArchive& operator<<(FArchive& Ar, FPlayerWeapon& Weapon)
	{
		Ar << Weapon.WeaponId;
		Ar << Weapon.ItemGuid;
		Ar << Weapon.bEquipped;
		Ar << Weapon.Index;
		Ar << Weapon.Attributes;
		Ar << Weapon.WeaponLevel;
		
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FPlayerModule
{
	GENERATED_BODY()

	FPlayerModule()
		: ItemGuid(0),
		  bEquipped(false),
		  ModuleId(0),
		  Category(EModuleCategory::C1),
		  ModuleLevel(0)
	{
	}

	UPROPERTY(BlueprintReadOnly)
	int32 ItemGuid;

	UPROPERTY(BlueprintReadOnly)
	bool bEquipped;

	UPROPERTY(BlueprintReadOnly)
	int64 ModuleId;

	UPROPERTY(BlueprintReadOnly)
	EModuleCategory Category;

	UPROPERTY(BlueprintReadOnly)
	FEquipmentAttributes Attributes;

	UPROPERTY()
	uint8 ModuleLevel;

	friend FArchive& operator<<(FArchive& Ar, FPlayerModule& Module)
	{
		Ar << Module.ModuleId;
		Ar << Module.ItemGuid;
		Ar << Module.bEquipped;
		Ar << Module.Category;
		Ar << Module.Attributes;
		Ar << Module.ModuleLevel;
		
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FPlayerModuleList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerModule> Modules;

	friend FArchive& operator<<(FArchive& Ar, FPlayerModuleList& A)
	{
		Ar << A.Modules;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FPlayerMaterial
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemGuid;

	UPROPERTY(BlueprintReadOnly)
	int32 Num;

	friend FArchive& operator<<(FArchive& Ar, FPlayerMaterial& R)
	{
		Ar << R.ItemGuid;
		Ar << R.Num;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FMaterialsHandle
{
	GENERATED_BODY()

public:

	bool IsValid() const
	{
		return MaterialsGroup.IsValid();
	}

	TMap<int32, int32>* operator->() const
	{
		return MaterialsGroup.Get();
	}

	TSharedPtr<TMap<int32, int32>> MaterialsGroup;
};

/** 玩家拥有的各种数据 */
USTRUCT(BlueprintType)
struct FPlayerProperties
{
	GENERATED_BODY()

	FPlayerProperties()
		: Level(1),
		  CurrentExperience(0),
		  MaxExperience(0)
	{
	}


	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 Level;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentExperience;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxExperience;

	friend FArchive& operator<<(FArchive& Ar, FPlayerProperties& A)
	{
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
	int64 LearnedTalents;

	UPROPERTY(BlueprintReadOnly)
	int32 CharacterLevel;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerMaterial> Materials;

	UPROPERTY(BlueprintReadOnly)
	FSoftObjectPath CharacterMesh;

	friend FArchive& operator<<(FArchive& Ar, FPlayerInfo& A)
	{
		Ar << A.Weapons;
		Ar << A.Modules;
		Ar << A.LearnedTalents;
		Ar << A.CharacterLevel;
		Ar << A.Materials;
		Ar << A.CharacterMesh;
		return Ar;
	}
};

struct FLaunchServerParam
{
	FLaunchServerParam() = default;

	FLaunchServerParam(const FString& InMapAssetPath, const FString& InModeName) :
		MapAssetPath(InMapAssetPath),
		ModeName(InModeName)
	{
	}

	FString MapAssetPath;

	FString ModeName;

	friend FArchive& operator<<(FArchive& Ar, FLaunchServerParam& A)
	{
		Ar << A.MapAssetPath;
		Ar << A.ModeName;
		return Ar;
	}
};

USTRUCT()
struct FLaunchServerResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString ServerAddress;

	UPROPERTY()
	FString ServerID;

	friend FArchive& operator<<(FArchive& Ar, FLaunchServerResult& A)
	{
		Ar << A.ServerAddress;
		//Ar << A.ServerID;
		return Ar;
	}

	
	FString GetConnectURL(int32 PlayerID) const;
};

USTRUCT()
struct FLaunchNotifyParam
{
	GENERATED_BODY()

	UPROPERTY()
	FString ServerId;

	UPROPERTY()
	int32 ListenPort;

	/** 启动后要通知的玩家 */
	UPROPERTY()
	int32 PlayerId;

	UPROPERTY()
	int32 MaxPlayers;

	friend FArchive& operator<<(FArchive& Ar, FLaunchNotifyParam& A)
	{
		Ar << A.ServerId;
		Ar << A.ListenPort;
		Ar << A.PlayerId;
		Ar << A.MaxPlayers;
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

	FUpdateServerPlayerParam() : ActivePlayers(0)
	{
	}

	FUpdateServerPlayerParam(int32 InActivePlayers) : ActivePlayers(InActivePlayers)
	{
	}

	UPROPERTY()
	int32 ActivePlayers;

	friend FArchive& operator<<(FArchive& Ar, FUpdateServerPlayerParam& A)
	{
		Ar << A.ActivePlayers;
		return Ar;
	}
};

struct FStoreItem
{
	TSharedPtr<FItem> Item;
	
	TArray<FAcquisitionCost> Costs;

	/** Store Item ID */
	int64 SIID;

	friend FArchive& operator<<(FArchive& Ar, FStoreItem& R)
	{
		Ar << R.SIID;
		Ar << R.Costs;
		R.Item = FItemHelper::Decode(Ar);
		return Ar;
	}
};

struct FStoreInformation
{
	int32 TotalPage;

	int32 TotalItems;
	
	TArray<FStoreItem> Items;

	friend FArchive& operator<<(FArchive& Ar, FStoreInformation& R)
	{
		Ar << R.TotalPage;
		Ar << R.TotalItems;
		Ar << R.Items;
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



struct FSearchStoreItemsParam
{
	FSearchStoreItemsParam()
		: StoreId(0),
		  Page(0)
	{
	}

	FSearchStoreItemsParam(int32 InStoreId, int32 InPage)
		: StoreId(InStoreId),
		  Page(InPage)
	{
	}

	int32 StoreId;
	
	int32 Page;

	friend FArchive& operator<<(FArchive& Ar, FSearchStoreItemsParam& A)
	{
		Ar << A.StoreId;
		Ar << A.Page;
		return Ar;
	}
};

struct FAcceptTaskParam
{
	int64 TaskId;

	FAcceptTaskParam()
		: TaskId(0)
	{
	}

	FAcceptTaskParam(int64 TaskId)
		: TaskId(TaskId)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FAcceptTaskParam& A)
	{
		Ar << A.TaskId;
		return Ar;
	}
};

USTRUCT()
struct FModifyTrackingParam
{
	GENERATED_BODY()

	FModifyTrackingParam(int64 TaskId, bool bTracking)
		: TaskId(TaskId),
		  bTracking(bTracking)
	{
	}
	
	FModifyTrackingParam()
		: TaskId(0),
		  bTracking(false)
	{
	}

	UPROPERTY()
	int64 TaskId;
	UPROPERTY()
	bool bTracking;

	friend FArchive& operator<<(FArchive& Ar, FModifyTrackingParam& A)
	{
		Ar << A.TaskId;
		Ar << A.bTracking;
		return Ar;
	}
};

struct FEmptyParam
{
	friend FArchive& operator<<(FArchive& Ar, FEmptyParam& A)
	{
		return Ar;
	}

	static FEmptyParam SINGLETON;
};

USTRUCT(BlueprintType)
struct FMulticastDelegateHandle
{
	GENERATED_BODY()

	FDelegateHandle Handle;
};
