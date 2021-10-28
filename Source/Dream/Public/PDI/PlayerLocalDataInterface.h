#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceBase.h"
#include "PlayerGameData.h"

class FPlayerLocalDataInterface : public FPlayerDataInterfaceBase, public FGCObject
{
	
public:
	FPlayerLocalDataInterface();
	
	virtual void Initialize(FInitializeDelegate Delegate) override;

	virtual void AddPlayerRewards(UItemData* Rewards, FCommonCompleteNotify Delegate) override;

	virtual void EquipWeapon(int64 WeaponId, int32 EquippedIndex, FCommonCompleteNotify Delegate) override;
	virtual void EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate) override;
	virtual void LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate) override;
	virtual void GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate) override;
	virtual void PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate) override;
	virtual void GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate) override;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate) override;
	virtual void GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate) override;

	virtual void GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate) override;
	virtual void LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate) override;
	virtual void GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate) override;

	virtual void DeliverTask(const int64& TaskId, FTaskRewardDelegate Delegate) override;
	virtual void AcceptTask(const int64& TaskId, FCommonCompleteNotify Delegate) override;

	virtual void UpdateTaskState(const FQuestActionHandle& Handle) override;

	virtual const FPlayerProperties& GetCachedProperties() const override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	virtual void RefreshPlayerProperties() override;

protected:

	void DoAddItem(UItemData* ItemData) const;

	void DoIncreaseExperience(int32 ExpAmount) const;

	void AsyncLoadGameData(const FString& Slot, const int32 Index, class USaveGame* SaveGame, FInitializeDelegate Delegate);
	void AsyncTaskData(const FString& Slot, const int32 Index, class USaveGame* SaveGame, FInitializeDelegate Delegate);
	void AsyncStoreData(const FString& Slot, const int32 Index, class USaveGame* SaveGame, FInitializeDelegate Delegate);

	class UPlayerGameData* GameData;

	class UTaskData* TaskData;
	
	class UStoreData* StoreData;

private:

	void AttemptExecuteInitializeDelegate(FInitializeDelegate Delegate, bool bCompleted);
	void FilterTasks(EGetTaskCondition Condition, TArray<FTaskInformation>& Tasks) const;

	FThreadSafeCounter LoadCounter;
	FThreadSafeBool bInitCompleted;
};

