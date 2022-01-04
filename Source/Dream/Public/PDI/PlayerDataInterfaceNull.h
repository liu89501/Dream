#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceBase.h"

class FPlayerDataInterfaceNull : public FPlayerDataInterface
{

public:

	virtual ~FPlayerDataInterfaceNull() override;
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
	virtual void ModifyTrackingState(const FModifyTrackingParam& Param) override;
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) override;
	virtual void RegisterServer(const FDedicatedServerInformation& Information) override;
	virtual void UpdateActivePlayers(const FUpdateServerPlayerParam& Param) override;
	virtual void NotifyBackendServer(const FLaunchNotifyParam& Param) override;
	virtual void SearchDedicatedServer(const FSearchServerParam& Parameter) override;
	virtual void Login() override;
	virtual void Logout() override;
	virtual int32 GetClientPlayerID() override;
	virtual TSharedPtr<FInternetAddr> GetBackendServerAddr() override;
	virtual const FPlayerProperties& GetCachedProperties() const override;
	virtual int32 GetCacheItemCount(int32 ItemGuid) override;
	virtual void IncreaseItemCount(int32 ItemGuid, int32 Delta) override;
	virtual FPlayerDataDelegate& GetPlayerDataDelegate() override;
	virtual FOnServerConnectionLose& OnServerConnectionLoseDelegate() override;
	virtual void ReconnectToSpecifiedServer(uint32 Address, uint32 Port, FOnReconnectServer Delegate) override;

private:

	FPlayerProperties EmptyProperties;
	FPlayerDataDelegate EmptyDelegate;
	FOnServerConnectionLose DummyOnServerConnectionLose;
};
