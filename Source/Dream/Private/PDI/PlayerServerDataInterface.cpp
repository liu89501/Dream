// ReSharper disable All
#include "PlayerServerDataInterface.h"
#include "DreamGameInstance.h"
#include "OnlineIdentityInterface.h"
#include "JsonUtilities.h"
#include "JsonWriter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "OnlineSubsystem.h"
#include "PlayerDataInterfaceStatic.h"
#include "SocketSubsystem.h"

#define LOCTEXT_NAMESPACE "FPlayerServerDataInterface"

#define BROADCAST_RESULT(DelegateFunc) { \
	bool bSuccess; \
	Reader << bSuccess; \
	DelegateFunc(bSuccess); }

#define BROADCAST_RESULT_PARAM(DelegateFunc, Param) { \
	bool bSuccess; \
	Reader << bSuccess; \
	Param P1; \
	if (bSuccess) { Reader << P1; } \
	DelegateFunc(P1, bSuccess); }

#define BROADCAST_RESULT_PARAM_SPEC(DelegateFunc, ParamName) { \
	bool bSuccess; \
	Reader << bSuccess; \
	if (bSuccess) { Reader << ParamName; } \
	DelegateFunc(ParamName, bSuccess); }

FPlayerServerDataInterface::FPlayerServerDataInterface()
{
	CALLBACK_BINDING_RAW(TService_PlayerInfo, this, &FPlayerServerDataInterface::OnReceivePlayerInformation);
	CALLBACK_BINDING_RAW(TService_PlayerInfo_Server, this, &FPlayerServerDataInterface::OnReceivePlayerInformation);
	
	CALLBACK_BINDING_RAW(TService_EquipWeapon, this, &FPlayerServerDataInterface::OnReceiveEquipWeapon);
	CALLBACK_BINDING_RAW(TService_EquipModule, this, &FPlayerServerDataInterface::OnReceiveEquipModule);
	CALLBACK_BINDING_RAW(TService_LearningTalents, this, &FPlayerServerDataInterface::OnReceiveLearningTalents);
	CALLBACK_BINDING_RAW(TService_GetStoreItems, this, &FPlayerServerDataInterface::OnReceiveGetStoreItems);
	CALLBACK_BINDING_RAW(TService_PayItem, this, &FPlayerServerDataInterface::OnReceivePayItem);
	CALLBACK_BINDING_RAW(TService_GetTalents, this, &FPlayerServerDataInterface::OnReceiveGetTalents);
	CALLBACK_BINDING_RAW(TService_GetTasks, this, &FPlayerServerDataInterface::OnReceiveGetTasks);
	CALLBACK_BINDING_RAW(TService_AcceptTask, this, &FPlayerServerDataInterface::OnReceiveAcceptTask);
	CALLBACK_BINDING_RAW(TService_DeliverTask, this, &FPlayerServerDataInterface::OnReceiveDeliverTask);
	CALLBACK_BINDING_RAW(TService_UpdatedTrackingTask, this, &FPlayerServerDataInterface::OnReceiveUpdatedTrackingTaskState);
	CALLBACK_BINDING_RAW(TService_ModifyTrackingState, this, &FPlayerServerDataInterface::OnReceiveModifyTrackingState);
	CALLBACK_BINDING_RAW(TService_Decompose, this, &FPlayerServerDataInterface::OnReceiveDecomposeItem);

	
	CALLBACK_BINDING_RAW(TReceive_PropertiesChange, this, &FPlayerServerDataInterface::OnReceivePropertiesChange);
	CALLBACK_BINDING_RAW(TReceive_Rewards, this, &FPlayerServerDataInterface::OnReceiveReceiveRewards);
	CALLBACK_BINDING_RAW(TReceive_MaterialsChange, this, &FPlayerServerDataInterface::OnReceiveMaterialsChange);
}

void FPlayerServerDataInterface::Initialize()
{
	FPlayerDataInterfaceBase::Initialize();

	BroadcastOnInitialize(true);
	
	MaterialsHandle.MaterialsGroup = MakeShared<TMap<int32, int32>>();
}

const FPlayerProperties& FPlayerServerDataInterface::GetCachedProperties() const
{
	return CachedProperties;
}

const FMaterialsHandle& FPlayerServerDataInterface::GetMaterialsHandle() const
{
	return MaterialsHandle;
}

void FPlayerServerDataInterface::OnReceivePlayerInformation(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT_PARAM(BroadcastOnGetPlayerInfo, FPlayerInfo);
}

void FPlayerServerDataInterface::OnReceiveEquipWeapon(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnEquipWeapon);
}

void FPlayerServerDataInterface::OnReceiveEquipModule(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnEquipModule);
}

void FPlayerServerDataInterface::OnReceiveLearningTalents(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnLearnTalents);
}

void FPlayerServerDataInterface::OnReceiveGetStoreItems(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT_PARAM(BroadcastOnGetStoreItems, FStoreInformation);
}

void FPlayerServerDataInterface::OnReceivePayItem(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnBuyItem);
}

void FPlayerServerDataInterface::OnReceiveReceiveRewards(FPacketArchiveReader& Reader)
{
	FItemListHandle Handle;
	Reader << Handle;
	
	BroadcastOnReceiveRewards(Handle);
}

void FPlayerServerDataInterface::OnReceiveGetTalents(FPacketArchiveReader& Reader)
{
	int64 Talents = 0;
	BROADCAST_RESULT_PARAM_SPEC(BroadcastOnGetTalents, Talents);
}

void FPlayerServerDataInterface::OnReceiveGetTasks(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT_PARAM(BroadcastOnGetTasks, FSearchTaskResult);
}

void FPlayerServerDataInterface::OnReceiveDeliverTask(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnDeliverTask);
}

void FPlayerServerDataInterface::OnReceiveAcceptTask(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnAcceptTask);
}

void FPlayerServerDataInterface::OnReceiveUpdatedTrackingTaskState(FPacketArchiveReader& Reader)
{
	TArray<FTaskInProgressInfo> UpdateInfo;
	Reader << UpdateInfo;

	//UE_LOG(LogDream, Verbose, TEXT("OnReceiveUpdatedTrackingTaskState: %d"), Tasks.Num());
	BroadcastOnUpdateTaskCond(UpdateInfo);
}

void FPlayerServerDataInterface::OnReceiveModifyTrackingState(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnModifyTaskTracking);
}

void FPlayerServerDataInterface::OnReceivePropertiesChange(FPacketArchiveReader& Reader)
{
	Reader << CachedProperties;
	GetPlayerDataDelegate().OnPropertiesChange.Broadcast(CachedProperties);
}

void FPlayerServerDataInterface::OnReceiveDecomposeItem(FPacketArchiveReader& Reader)
{
	BROADCAST_RESULT(BroadcastOnDecomposeItem);
}

void FPlayerServerDataInterface::OnReceiveMaterialsChange(FPacketArchiveReader& Reader)
{
	TArray<FPlayerMaterial> ChangedMaterials;
	Reader << ChangedMaterials;

	for (const FPlayerMaterial& Material : ChangedMaterials)
	{
		int& MaterialNum = MaterialsHandle->FindOrAdd(Material.ItemGuid);
		MaterialNum = Material.Num;
	}
}

#undef LOCTEXT_NAMESPACE
