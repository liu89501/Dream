#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceBase.h"

DEFINED_SERVICE_INFO(9, void, TService_UpdatedTrackingTask);

DEFINED_SERVICE_INFO(10, EGetEquipmentCondition, TService_PlayerInfo);
DEFINED_SERVICE_INFO(11, FItemListHandle, TService_AddRewards);
DEFINED_SERVICE_INFO(12, int64, TService_DeliverTask);
DEFINED_SERVICE_INFO(13, FAcceptTaskParam, TService_AcceptTask);
DEFINED_SERVICE_INFO(14, FQuestActionHandle, TService_UpdateTaskState);
DEFINED_SERVICE_INFO(15, FModifyTrackingParam, TService_ModifyTrackingState);
DEFINED_SERVICE_INFO(16, FEquipModuleParam, TService_EquipModule);
DEFINED_SERVICE_INFO(17, int64, TService_LearningTalents);
DEFINED_SERVICE_INFO(18, FEquipWeaponParam, TService_EquipWeapon);
DEFINED_SERVICE_INFO(19, FSearchStoreItemsParam, TService_GetStoreItems);
DEFINED_SERVICE_INFO(20, int64, TService_PayItem);
DEFINED_SERVICE_INFO(21, ETalentCategory, TService_GetTalents);
DEFINED_SERVICE_INFO(22, FSearchTaskParam, TService_GetTasks);

DEFINED_SERVICE_INFO(23, void, TReceive_PropertiesChange);
DEFINED_SERVICE_INFO(24, void, TReceive_Rewards);


class FPlayerServerDataInterface : public FPlayerDataInterfaceBase
{

public:

	FPlayerServerDataInterface();

	virtual void Initialize() override;

	virtual void AddPlayerRewards(const FItemListHandle& Rewards) override;
	
	virtual void EquipWeapon(const FEquipWeaponParam& Param) override;

	virtual void EquipModule(const FEquipModuleParam& Param) override;

	virtual void LearningTalents(int64 LearnedTalents) override;

	virtual void GetStoreItems(const FSearchStoreItemsParam& Param) override;

	virtual void PayItem(int64 ItemId) override;

	virtual void GetPlayerInfo(EGetEquipmentCondition Condition) override;

	virtual void GetTalents(ETalentCategory TalentCategory) override;

	virtual void GetTasks(const FSearchTaskParam& Param) override;

	virtual void DeliverTask(int64 TaskId) override;
	virtual void AcceptTask(const FAcceptTaskParam& Param) override;
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) override;
	virtual void ModifyTrackingState(const FModifyTrackingParam& Param) override;
	
	virtual const FPlayerProperties& GetCachedProperties() const override;

	virtual int32 GetCacheItemCount(int32 ItemGuid) override;
	virtual void IncreaseItemCount(int32 ItemGuid, int32 Delta) override;

public:

	void OnReceivePlayerInformation(FPacketArchiveReader& Reader);
	void OnReceiveEquipWeapon(FPacketArchiveReader& Reader);
	void OnReceiveEquipModule(FPacketArchiveReader& Reader);
	void OnReceiveLearningTalents(FPacketArchiveReader& Reader);
	void OnReceiveGetStoreItems(FPacketArchiveReader& Reader);
	void OnReceivePayItem(FPacketArchiveReader& Reader);
	void OnReceiveReceiveRewards(FPacketArchiveReader& Reader);
	void OnReceiveGetTalents(FPacketArchiveReader& Reader);
	void OnReceiveGetTasks(FPacketArchiveReader& Reader);
	void OnReceiveDeliverTask(FPacketArchiveReader& Reader);
	void OnReceiveAcceptTask(FPacketArchiveReader& Reader);
	void OnReceiveUpdatedTrackingTaskState(FPacketArchiveReader& Reader);
	void OnReceiveModifyTrackingState(FPacketArchiveReader& Reader);
	void OnReceivePropertiesChange(FPacketArchiveReader& Reader);

private:

	FPlayerProperties CachedProperties;

	TMap<int32, int32> CacheItemCount;
};
