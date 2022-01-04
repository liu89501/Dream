#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceBase.h"
#include "PlayerGameData.h"

/*
class FPlayerLocalDataInterface : public FPlayerDataInterfaceBase, public FGCObject
{
	
public:
	FPlayerLocalDataInterface();
	
	virtual void Initialize() override;

	virtual void AddPlayerRewards(const FItemDataHandle& Rewards) override;

	virtual void EquipWeapon(const FEquipWeaponParam& Param) override;
	virtual void EquipModule(const FEquipModuleParam& Param) override;
	virtual void GetStoreItems(int32 StoreId) override;
	virtual void PayItem(const FBuyItemParam& Param) override;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition) override;

	virtual void GetTalents(ETalentCategory TalentCategory) override;
	virtual void LearningTalents(const TArray<int32>& TalentIdArray) override;
	virtual void GetTasks(const FSearchTaskParam& Param) override;

	virtual void DeliverTask(int32 TaskId) override;
	virtual void AcceptTask(const FAcceptTaskParam& Param) override;
	virtual void ModifyTrackingState(const FModifyTrackingParam& Param) override;

	virtual void UpdateTaskState(const FQuestActionHandle& Handle) override;

	virtual const FPlayerProperties& GetCachedProperties() const override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:

	void DoAddItem(UItemData* ItemData) const;

	void DoIncreaseExperience(int32 ExpAmount) const;

	void AsyncLoadGameData(const FString& Slot, const int32 Index, class USaveGame* SaveGame);
	void AsyncTaskData(const FString& Slot, const int32 Index, class USaveGame* SaveGame);
	void AsyncStoreData(const FString& Slot, const int32 Index, class USaveGame* SaveGame);

	class UPlayerGameData* GameData;

	class UTaskData* TaskData;
	
	class UStoreData* StoreData;

private:

	void AttemptExecuteInitializeDelegate(bool bCompleted);
	void FilterTasks(EGetTaskCondition Condition, TArray<FTaskInformation>& Tasks) const;

	FThreadSafeCounter LoadCounter;
	FThreadSafeBool bInitCompleted;
};
*/

