#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterface.h"

#define MSG_ERROR TEXT("Error")
#define MSG_SUCCESS TEXT("")

class FPlayerLocalDataInterface : public FPlayerDataInterface, public FGCObject
{
	
public:
	FPlayerLocalDataInterface();
	
	virtual void Initialize(FInitializeDelegate Delegate) override;

	virtual void AddPlayerRewards(const TArray<UItemData*>& Rewards, FCommonCompleteNotify Delegate) override;

	virtual void EquipWeapon(int64 WeaponId, int32 EquippedIndex, FCommonCompleteNotify Delegate) override;
	virtual void EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate) override;
	virtual void LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate) override;
	virtual void GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate) override;
	virtual void RunServer(const FRunServerParameter& Parameter, FGetServerComplete Delegate) override;
	virtual void PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate) override;
	virtual void GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate) override;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate) override;
	virtual void GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate) override;

	virtual void GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate) override;
	virtual void LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate) override;
	virtual void GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate) override;

	virtual void IncreaseExperience(const FUserExperiencePair& UserExperience, FExperienceChangeDelegate Delegate) override;
	
	virtual void DeliverTask(const int64& TaskId, FCommonCompleteNotify Delegate) override;
	
	virtual void RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName, FRegisterServerComplete Delegate) override;
	virtual void UnRegisterServer() override;
	virtual void UpdateActivePlayers(bool bIncrement) override;
	virtual void Login(FCommonCompleteNotify Delegate) override;
	virtual void Logout() override;
	virtual FString GetServerToken() const override;

	virtual const FPlayerProperties& GetCachedProperties() const override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	
protected:

	void DoAddItem(UItemData* ItemData) const;

	void DoIncreaseExperience(int32 ExpAmount) const;

	void AsyncLoadGameData(const FString& Slot, const int32 Index, class USaveGame* SaveGame, FInitializeDelegate Delegate);
	void AsyncTaskData(const FString& Slot, const int32 Index, class USaveGame* SaveGame, FInitializeDelegate Delegate);

	TMap<EModuleCategory, FPlayerModuleList> GetModuleFromCategory() const;
	
	class UPlayerGameData* GameData;

	class UTaskData* TaskData;
};
