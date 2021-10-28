#pragma once

#include "CoreMinimal.h"
#include "IHttpRequest.h"
#include "PlayerDataInterfaceBase.h"

class FPlayerServerDataInterface : public FPlayerDataInterfaceBase
{

public:
	
	virtual void AddPlayerRewards(UItemData* Rewards, FCommonCompleteNotify Delegate) override;
	
	virtual void EquipWeapon(int64 WeaponId, int32 EquipmentIndex, FCommonCompleteNotify Delegate) override;

	virtual void EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate) override;

	virtual void LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate) override;

	virtual void LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate) override;

	virtual void GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate) override;

	virtual void PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate) override;

	virtual void GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate) override;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate) override;

	virtual void GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate) override;
	virtual void GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate) override;

	virtual void GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate) override;

	virtual void DeliverTask(const int64& TaskId, FTaskRewardDelegate Delegate) override;
	virtual void AcceptTask(const int64& TaskId, FCommonCompleteNotify Delegate) override;
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) override;
	
	virtual const FPlayerProperties& GetCachedProperties() const override;
	
	virtual void RefreshPlayerProperties() override;

protected:

	void OnGetWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetWeaponComplete Delegate);
	void OnGetPlayerInfoCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetPlayerInfoComplete Delegate);
	void OnAddItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnSwitchWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate);
	void OnPayItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate);
	void OnGetStoreItemsCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetStoreItemsComplete Delegate);

private:

	FPlayerProperties CachedProperties;
};

