// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "DreamType.h"
#include "PlayerDataInterfaceType.generated.h"

enum class ETaskCondGUID : int32
{
	None,
	CONTAINER,
	KILL_TARGET,
	EVENT
};

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
UENUM(BlueprintType)
enum class EGetEquipmentCondition : uint8
{
	All,
	Equipped,
	UnEquipped
};

struct FQuestActionBase
{
public:

	FQuestActionBase() : ActionCondGUID(ETaskCondGUID::None)
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

struct FQuestActionContainer : FQuestActionBase
{
public:

	FQuestActionContainer()
	{
		ActionCondGUID = ETaskCondGUID::CONTAINER;
	}

	void AddAction(FQuestActionBase* Action)
	{
		Actions.Add(MakeShareable(Action));
	}
	
	void AddActionPtr(TSharedPtr<FQuestActionBase> Action)
	{
		Actions.Add(Action);
	}

	void GetActions(TArray<TSharedPtr<FQuestActionBase>>& OutActions) const
	{
		OutActions = Actions;
	}

	virtual void SerializeData(FArchive& Ar) override;

private:

	TArray<TSharedPtr<FQuestActionBase>> Actions;
};

struct FQuestAction_KilledTarget : FQuestActionBase
{
	explicit FQuestAction_KilledTarget(EPawnType InTargetPawn) : TargetPawn(InTargetPawn)
	{
		ActionCondGUID = ETaskCondGUID::KILL_TARGET;
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
	explicit FQuestAction_Event(const FName& InEventName) : EventName(InEventName)
	{
		ActionCondGUID = ETaskCondGUID::EVENT;
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

	FQuestActionHandle(int32 InPlayerId, FQuestActionBase* InData)
		: PlayerId(InPlayerId),
		  ActionData(InData)
	{
	}

	FQuestActionBase* GetData() const
	{
		return ActionData;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuestActionHandle& A)
	{
		Ar << A.PlayerId;
		A.ActionData->SerializeData(Ar);
		return Ar;
	}

private:

	int32 PlayerId;

	FQuestActionBase* ActionData;
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
class DREAM_API UDQuestCondition_Container : public UDQuestCondition
{
	GENERATED_BODY()

public:

	UDQuestCondition_Container()
	{
		ConditionGUID = ETaskCondGUID::CONTAINER;
	}

	TArray<UDQuestCondition*>& GetConditions()
	{
		return Conditions;
	}

	virtual float GetQuestProgressPercent() const override;

	virtual void SerializeData(FArchive& Ar) override;
	
#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

protected:

	UPROPERTY(EditAnywhere, Instanced, Category=Condition)
	TArray<UDQuestCondition*> Conditions;
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

	FQuestConditionHandle() : QuestCondition(nullptr)
	{
	}

	explicit FQuestConditionHandle(UDQuestCondition* InCondition) : QuestCondition(InCondition)
	{
	}

	UDQuestCondition* operator->() const
	{
		return QuestCondition;
	}

	UDQuestCondition* operator*() const
	{
		return QuestCondition;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuestConditionHandle& A)
	{
		if (Ar.IsLoading())
		{
			A.QuestCondition = UDQuestCondition::Decode(Ar);
		}
		else
		{
			A.QuestCondition->SerializeData(Ar);
		}
		return Ar;
	}

	UPROPERTY(BlueprintReadOnly)
	UDQuestCondition* QuestCondition;
};


UCLASS(Blueprintable, Abstract, EditInlineNew)
class DREAM_API UItemData : public UObject
{
	GENERATED_BODY()

public:

	/** 序列化时标识一下具体序列化的是什么数据 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	int32 ItemGuid;
	
	// 反序列化使用 UItemData::decode
	virtual void SerializeItemData(FArchive& Ar);

	static UItemData* Decode(FArchive& Ar);

	virtual int32 GetItemNum() const { return 1; }

public:

	UFUNCTION(BlueprintCallable, Category=ItemData)
	UClass* GetItemClass() const;

	UFUNCTION(BlueprintCallable, Category=ItemData)
	const FPropsInfo& GetPropsInfo() const;

	UFUNCTION(BlueprintCallable, Category=ItemData)
	TEnumAsByte<EItemType::Type> GetItemType() const;
};

UCLASS()
class DREAM_API UItemDataEquipment : public UItemData
{
	GENERATED_BODY()

public:

	virtual void SerializeItemData(FArchive& Ar) override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	FEquipmentAttributes Attributes;
};


UCLASS()
class DREAM_API UItemDataEquipment_Random : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=ItemData)
	class UDEquipmentAttributesPool* AttrPool;

public:

	UItemDataEquipment* CastToEquipment();
};


UCLASS()
class DREAM_API UItemDataNumericalValue : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=ItemData)
	int32 Value;

	virtual int32 GetItemNum() const override
	{
		return Value;
	}

	virtual void SerializeItemData(FArchive& Ar) override;
};


USTRUCT(BlueprintType)
struct FAcquisitionCost
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemGuid;

	UPROPERTY(BlueprintReadOnly)
	int32 CostAmount;

	friend FArchive& operator<<(FArchive& Ar, FAcquisitionCost& R)
	{
		Ar << R.ItemGuid;
		Ar << R.CostAmount;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct DREAM_API FItemListHandle
{
	GENERATED_BODY()

public:

	FItemListHandle()
		: PlayerId(0)
	{
		
	}

	FItemListHandle(int32 PlayerId, const TArray<UItemData*>& Items)
		: PlayerId(PlayerId),
		  Items(Items)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FItemListHandle& R)
	{
		bool bIsSaving = Ar.IsSaving();

		if (bIsSaving)
		{
			Ar << R.PlayerId;
		}

		int32 Num = R.Items.Num();
		Ar << Num;

		R.Items.SetNumZeroed(Num);
		
		for (int32 N = 0; N < Num; N++)
		{
			if (bIsSaving)
			{
				R.Items[N]->SerializeItemData(Ar);
			}
			else
			{
				R.Items[N] = UItemData::Decode(Ar);
			}
		}
		return Ar;
	}

public:

	int32 PlayerId;

	UPROPERTY(BlueprintReadOnly, Category=ItemData)
	TArray<UItemData*> Items;
};

USTRUCT(BlueprintType)
struct FItemDataHandle
{
	GENERATED_BODY()

public:

	FItemDataHandle() : PlayerId(0), ItemData(nullptr)
	{
	}

	FItemDataHandle(int32 InPlayerId, UItemData* InItemData)
		: PlayerId(InPlayerId), ItemData(InItemData)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FItemDataHandle& R)
	{
		if (Ar.IsLoading())
		{
			R.ItemData = UItemData::Decode(Ar);
		}
		else
		{
			// playerId 只在写到服务器时才需要
			Ar << R.PlayerId;
			
			R.ItemData->SerializeItemData(Ar);
		}

		return Ar;
	}

	UItemData* Get() const;

	int32 PlayerId;

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData;
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

	friend FArchive& operator<<(FArchive& Ar, FTaskInformation& A)
	{
		Ar << A.TaskId;
		Ar << A.TaskGroupId;
		Ar << A.TaskDescription;
		Ar << A.TaskMark;
		Ar << A.bTracking;
		Ar << A.Condition;
		Ar << A.CompletedReward;
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
		Ar << A.Tasks;
		Ar << A.TotalPage;
		Ar << A.TotalNum;
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
		  WeaponId(0)
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

	friend FArchive& operator<<(FArchive& Ar, FPlayerWeapon& Weapon)
	{
		Ar << Weapon.WeaponId;
		Ar << Weapon.ItemGuid;
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
		: ItemGuid(0),
		  bEquipped(false),
		  ModuleId(0),
		  Category(EModuleCategory::C1)
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

	friend FArchive& operator<<(FArchive& Ar, FPlayerModule& Module)
	{
		Ar << Module.ModuleId;
		Ar << Module.ItemGuid;
		Ar << Module.bEquipped;
		Ar << Module.Category;
		Ar << Module.Attributes;
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
	FPlayerProperties Properties;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerMaterial> Materials;

	UPROPERTY(BlueprintReadOnly)
	FSoftObjectPath CharacterMesh;

	friend FArchive& operator<<(FArchive& Ar, FPlayerInfo& A)
	{
		Ar << A.Properties;
		Ar << A.Weapons;
		Ar << A.Modules;
		Ar << A.LearnedTalents;
		Ar << A.Materials;
		Ar << A.CharacterMesh;
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FSearchServerParam
{
	GENERATED_USTRUCT_BODY()

	FSearchServerParam() = default;

	FSearchServerParam(const FString& InMapName, const FString& InModeName, const FString& InMapAssetPath) :
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

	friend FArchive& operator<<(FArchive& Ar, FSearchServerParam& A)
	{
		Ar << A.MapAssetPath;
		Ar << A.MapName;
		Ar << A.ModeName;
		return Ar;
	}
};

USTRUCT()
struct FSearchServerResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString ServerAddress;

	UPROPERTY()
	FString ServerID;

	friend FArchive& operator<<(FArchive& Ar, FSearchServerResult& A)
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

USTRUCT(BlueprintType)
struct FStoreItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FAcquisitionCost> Costs;

	/** Store Item ID */
	UPROPERTY(BlueprintReadOnly)
	int64 SIID;

	friend FArchive& operator<<(FArchive& Ar, FStoreItem& R)
	{
		Ar << R.SIID;
		Ar << R.Costs;
		R.ItemData = UItemData::Decode(Ar);
		return Ar;
	}
};

USTRUCT(BlueprintType)
struct FStoreInformation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TotalPage;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalItems;
	
	UPROPERTY(BlueprintReadOnly)
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

USTRUCT(BlueprintType)
struct FMulticastDelegateHandle
{
	GENERATED_BODY()

	FDelegateHandle Handle;
};
