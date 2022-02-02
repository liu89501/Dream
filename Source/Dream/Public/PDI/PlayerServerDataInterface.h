#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceBase.h"

#define DEFAULT_SERVICE_IMPLEMENT(FuncName, Service) virtual void FuncName(const Service::Type& Param) override \
	{ \
		SocketSender->Send(PDIBuildParam<Service>(Param)); \
	}

DEFINED_SERVICE_INFO(9, void, TService_UpdatedTrackingTask);

DEFINED_SERVICE_INFO(10, int32, TService_PlayerInfo);
DEFINED_SERVICE_INFO(11, FItemListParam, TService_AddRewards);
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

DEFINED_SERVICE_INFO(25, FDecomposeParam, TService_Decompose);


DEFINED_SERVICE_INFO(23, void, TReceive_PropertiesChange);
DEFINED_SERVICE_INFO(24, void, TReceive_Rewards);
DEFINED_SERVICE_INFO(26, void, TReceive_MaterialsChange);

DEFINED_SERVICE_INFO(27, FQueryPlayerParam, TService_PlayerInfo_Server);


class FPlayerServerDataInterface : public FPlayerDataInterfaceBase
{

public:

	FPlayerServerDataInterface();

	virtual void Initialize() override;

	DEFAULT_SERVICE_IMPLEMENT(AddPlayerRewards, TService_AddRewards);
	DEFAULT_SERVICE_IMPLEMENT(EquipWeapon, TService_EquipWeapon);
	DEFAULT_SERVICE_IMPLEMENT(EquipModule, TService_EquipModule);
	DEFAULT_SERVICE_IMPLEMENT(LearningTalents, TService_LearningTalents);
	DEFAULT_SERVICE_IMPLEMENT(GetStoreItems, TService_GetStoreItems);
	DEFAULT_SERVICE_IMPLEMENT(PayItem, TService_PayItem);
	DEFAULT_SERVICE_IMPLEMENT(GetPlayerInfo, TService_PlayerInfo);
	DEFAULT_SERVICE_IMPLEMENT(GetTalents, TService_GetTalents);
	DEFAULT_SERVICE_IMPLEMENT(GetTasks, TService_GetTasks);
	DEFAULT_SERVICE_IMPLEMENT(DeliverTask, TService_DeliverTask);
	DEFAULT_SERVICE_IMPLEMENT(AcceptTask, TService_AcceptTask);
	DEFAULT_SERVICE_IMPLEMENT(UpdateTaskState, TService_UpdateTaskState);
	DEFAULT_SERVICE_IMPLEMENT(ModifyTrackingState, TService_ModifyTrackingState);
	DEFAULT_SERVICE_IMPLEMENT(DecomposeItem, TService_Decompose);
	DEFAULT_SERVICE_IMPLEMENT(ServerGetPlayerInfo, TService_PlayerInfo_Server);

	virtual const FPlayerProperties& GetCachedProperties() const override;

	virtual const FMaterialsHandle& GetMaterialsHandle() const override;

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
	void OnReceiveDecomposeItem(FPacketArchiveReader& Reader);
	void OnReceiveMaterialsChange(FPacketArchiveReader& Reader);

private:

	FPlayerProperties CachedProperties;
	
	FMaterialsHandle MaterialsHandle;
	
};

