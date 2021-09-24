#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"

#define PDI_LOCAL TEXT("Local")
#define PDI_SERVER TEXT("Server")

DECLARE_DELEGATE_OneParam(FInitializeDelegate, bool /* ErrorMessage */);
DECLARE_DELEGATE_OneParam(FCommonCompleteNotify, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetWeaponComplete, const TArray<FPlayerWeapon>&, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetPlayerInfoComplete, const FPlayerInfo&, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetPlayerPropertiesDelegate, const FPlayerProperties&, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FGetServerComplete, const FString& /*ServerAddress*/, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_OneParam(FRegisterServerComplete, const FString& /*ServerId*/);
DECLARE_DELEGATE_TwoParams(FGetStoreItemsComplete, const FStoreInformation&, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FGetTalentsComplete, const TArray<FTalentInfo>&, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FExperienceChangeDelegate, int32 NewLevel, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FGetTasksDelegate, const TArray<FTaskInformation>& Tasks, const FString& /*ErrorMessage*/);


class FPlayerDataInterface
{
	
public:
	virtual ~FPlayerDataInterface() = default;

	virtual void Initialize(FInitializeDelegate Delegate = FInitializeDelegate()) = 0;

	// Server
	virtual void AddPlayerRewards(const TArray<UItemData*>& Rewards, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void EquipWeapon(int64 WeaponId, int32 EquipmentIndex, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Local
	virtual void LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	virtual void GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate = FGetStoreItemsComplete()) = 0;

	// Server
	virtual void RunServer(const FRunServerParameter& Parameter, FGetServerComplete Delegate = FGetServerComplete()) = 0;

	// Local
	virtual void PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	virtual void GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate) = 0;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate) = 0;
	virtual void GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate) = 0;
	virtual void GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate) = 0;

	virtual void GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate) = 0;

	// Server 
	virtual void IncreaseExperience(const FUserExperiencePair& UserExperience, FExperienceChangeDelegate Delegate = FExperienceChangeDelegate()) = 0;

	// Local
	virtual void DeliverTask(const int64& TaskId, FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;

	// Server
	virtual void RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName, FRegisterServerComplete Delegate) = 0;
	virtual void UnRegisterServer() = 0;

	// Server
	virtual void UpdateActivePlayers(bool bIncrement) = 0;

	// Local
	virtual void Login(FCommonCompleteNotify Delegate = FCommonCompleteNotify()) = 0;
	virtual void Logout() = 0;

	virtual FString GetServerToken() const = 0;

	static bool IsLocalInterface();

	virtual const FPlayerProperties& GetCachedProperties() const = 0;
};
