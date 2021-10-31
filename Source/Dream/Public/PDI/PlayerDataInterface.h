#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"

#define PDI_LOCAL TEXT("Local")
#define PDI_SERVER TEXT("Server")

#define MSG_ERROR TEXT("Error")
#define MSG_SUCCESS TEXT("")

DECLARE_DELEGATE_OneParam(FInitializeDelegate, bool /* ErrorMessage */);
DECLARE_DELEGATE_OneParam(FCommonCompleteNotify, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetWeaponComplete, const TArray<FPlayerWeapon>&, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetPlayerInfoComplete, const FPlayerInfo&, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetPlayerPropertiesDelegate, const FPlayerProperties&, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetServerComplete, const FFindServerResult& /*Result*/, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FRunServerComplete, const FString& /*ServerId*/, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_OneParam(FRegisterServerComplete, const FString& /*ServerId*/);
DECLARE_DELEGATE_TwoParams(FGetStoreItemsComplete, const FStoreInformation&, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FGetTalentsComplete, const TArray<FTalentInfo>&, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FExperienceChangeDelegate, int32 NewLevel, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FGetTasksDelegate, const TArray<FTaskInformation>&, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FTaskRewardDelegate, UItemData*, const FString& /*ErrorMessage*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FPlayerMoneyChanged, int64);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FPlayerExperienceChanged, int32 /* Max */, int32 /*Current*/, int32/*Level*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FTaskStateChanged, const TArray<FTaskInformation>&);

struct FPlayerDataDelegate
{
	FPlayerMoneyChanged OnMoneyChanged;
	FPlayerExperienceChanged OnExperienceChanged;
	FTaskStateChanged OnTaskStateChanged;
};

class FPlayerDataInterface
{
	
public:
	virtual ~FPlayerDataInterface() = default;

	virtual void Initialize(FInitializeDelegate Delegate = FInitializeDelegate()) = 0;

	// Server
	virtual void AddPlayerRewards(UItemData* Rewards, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void EquipWeapon(int64 WeaponId, int32 EquipmentIndex, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	virtual void GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate = FGetStoreItemsComplete()) = 0;

	// Local
	virtual void PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	virtual void GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate) = 0;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate) = 0;
	
	virtual void GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate) = 0;
	
	virtual void GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate) = 0;

	virtual void GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate) = 0;

	// Local
	virtual void DeliverTask(const int64& TaskId, FTaskRewardDelegate Delegate = FTaskRewardDelegate()) = 0;
	virtual void AcceptTask(const int64& TaskId, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;
	virtual void ModifyTrackingState(const int64& TaskId, bool bTracking, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Server
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) = 0;

	// Server
	virtual void RegisterServer(const FDedicatedServerInformation& Information, FRegisterServerComplete Delegate = FRegisterServerComplete()) = 0;
	virtual void UnRegisterServer() = 0;

	// Server
	virtual void UpdateActivePlayers(bool bIncrement) = 0;

	// local
	virtual void GetAvailableDedicatedServer(const FString& ServerId, FGetServerComplete Delegate) = 0;
	virtual void RunNewDedicatedServer(const FRunServerParameter& Parameter, FRunServerComplete Delegate = FRunServerComplete()) = 0;

	// Local
	virtual void Login(FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;
	virtual void Logout() = 0;

	virtual FString GetServerToken() const = 0;

	virtual const FPlayerProperties& GetCachedProperties() const = 0;

	// Local
	/** 当条用过Server函数修改过Properties数据时应该调用此函数刷新数据 */
	virtual void RefreshPlayerProperties() = 0;
	
	virtual FPlayerDataDelegate& GetPlayerDataDelegate() = 0;
	
};
