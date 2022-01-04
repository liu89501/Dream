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

FPlayerServerDataInterface::FPlayerServerDataInterface()
{
	CALLBACK_BINDING_RAW(TService_PlayerInfo::MarkId, this, &FPlayerServerDataInterface::OnReceivePlayerInformation);
	CALLBACK_BINDING_RAW(TService_EquipWeapon::MarkId, this, &FPlayerServerDataInterface::OnReceiveEquipWeapon);
	CALLBACK_BINDING_RAW(TService_EquipModule::MarkId, this, &FPlayerServerDataInterface::OnReceiveEquipModule);
	CALLBACK_BINDING_RAW(TService_LearningTalents::MarkId, this, &FPlayerServerDataInterface::OnReceiveLearningTalents);
	CALLBACK_BINDING_RAW(TService_GetStoreItems::MarkId, this, &FPlayerServerDataInterface::OnReceiveGetStoreItems);
	CALLBACK_BINDING_RAW(TService_PayItem::MarkId, this, &FPlayerServerDataInterface::OnReceivePayItem);
	CALLBACK_BINDING_RAW(TService_GetTalents::MarkId, this, &FPlayerServerDataInterface::OnReceiveGetTalents);
	CALLBACK_BINDING_RAW(TService_GetTasks::MarkId, this, &FPlayerServerDataInterface::OnReceiveGetTasks);
	CALLBACK_BINDING_RAW(TService_AcceptTask::MarkId, this, &FPlayerServerDataInterface::OnReceiveAcceptTask);
	CALLBACK_BINDING_RAW(TService_DeliverTask::MarkId, this, &FPlayerServerDataInterface::OnReceiveDeliverTask);
	CALLBACK_BINDING_RAW(TService_UpdatedTrackingTask::MarkId, this, &FPlayerServerDataInterface::OnReceiveUpdatedTrackingTaskState);
	CALLBACK_BINDING_RAW(TService_ModifyTrackingState::MarkId, this, &FPlayerServerDataInterface::OnReceiveModifyTrackingState);

	
	CALLBACK_BINDING_RAW(TReceive_PropertiesChange::MarkId, this, &FPlayerServerDataInterface::OnReceivePropertiesChange);
	CALLBACK_BINDING_RAW(TReceive_Rewards::MarkId, this, &FPlayerServerDataInterface::OnReceiveReceiveRewards);
}

void FPlayerServerDataInterface::Initialize()
{
	FPlayerDataInterfaceBase::Initialize();

	BroadcastOnInitialize(true);
}

void FPlayerServerDataInterface::AddPlayerRewards(const FItemListHandle& Rewards)
{
	SocketSender->Send(PDIBuildParam<TService_AddRewards>(Rewards));
}

void FPlayerServerDataInterface::DeliverTask(int64 TaskId)
{
	SocketSender->Send(PDIBuildParam<TService_DeliverTask>(TaskId));
}

void FPlayerServerDataInterface::AcceptTask(const FAcceptTaskParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_AcceptTask>(Param));
}

void FPlayerServerDataInterface::UpdateTaskState(const FQuestActionHandle& Handle)
{
	SocketSender->Send(PDIBuildParam<TService_UpdateTaskState>(Handle));
}

void FPlayerServerDataInterface::ModifyTrackingState(const FModifyTrackingParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_ModifyTrackingState>(Param));
}

const FPlayerProperties& FPlayerServerDataInterface::GetCachedProperties() const
{
	return CachedProperties;
}

int32 FPlayerServerDataInterface::GetCacheItemCount(int32 ItemGuid)
{
	int32 Value;

	switch (GetItemType(ItemGuid))
	{
	case EItemType::Experience:
		{
			Value = CachedProperties.CurrentExperience;
			break;
		}
	default:
		{
			int32* Count = CacheItemCount.Find(ItemGuid);
			Value = Count ? *Count : 0;
		}
	}
	
	return Value;
}

void FPlayerServerDataInterface::IncreaseItemCount(int32 ItemGuid, int32 Delta)
{
	int32* ValuePtr;

	switch (GetItemType(ItemGuid))
	{
	case EItemType::Experience:
		{
			ValuePtr = &CachedProperties.CurrentExperience;
			break;
		}
	default:
		{
			ValuePtr = &CacheItemCount.FindOrAdd(ItemGuid);
		}
	}
	
	*ValuePtr = FMath::Max(0, *ValuePtr + Delta);
}

void FPlayerServerDataInterface::EquipModule(const FEquipModuleParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_EquipModule>(Param));
}

void FPlayerServerDataInterface::LearningTalents(int64 LearnedTalents)
{
	SocketSender->Send(PDIBuildParam<TService_LearningTalents>(LearnedTalents));
}

void FPlayerServerDataInterface::EquipWeapon(const FEquipWeaponParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_EquipWeapon>(Param));
}

void FPlayerServerDataInterface::GetStoreItems(const FSearchStoreItemsParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_GetStoreItems>(Param));
}

void FPlayerServerDataInterface::PayItem(int64 ItemId)
{
	SocketSender->Send(PDIBuildParam<TService_PayItem>(ItemId));
}

void FPlayerServerDataInterface::GetPlayerInfo(EGetEquipmentCondition Condition)
{
	SocketSender->Send(PDIBuildParam<TService_PlayerInfo>(Condition));
}

void FPlayerServerDataInterface::GetTalents(ETalentCategory TalentCategory)
{
	SocketSender->Send(PDIBuildParam<TService_GetTalents>(TalentCategory));
}

void FPlayerServerDataInterface::GetTasks(const FSearchTaskParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_GetTasks>(Param));
}

void FPlayerServerDataInterface::OnReceivePlayerInformation(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	FPlayerInfo PlayerInfo;
	Reader << bSuccess;

	if (bSuccess)
	{
		Reader << PlayerInfo;

		CachedProperties = PlayerInfo.Properties;

		for (const FPlayerMaterial& PlayerMaterial : PlayerInfo.Materials)
		{
			CacheItemCount.Add(PlayerMaterial.ItemGuid, PlayerMaterial.Num);
		}
	}

	BroadcastOnGetPlayerInfo(PlayerInfo, bSuccess);
}

void FPlayerServerDataInterface::OnReceiveEquipWeapon(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnEquipWeapon(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveEquipModule(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnEquipModule(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveLearningTalents(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnLearnTalents(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveGetStoreItems(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	FStoreInformation Information;
	
	Reader << bSuccess;
	if (bSuccess)
	{
		Reader << Information;
	}
	BroadcastOnGetStoreItems(Information, bSuccess);
}

void FPlayerServerDataInterface::OnReceivePayItem(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnBuyItem(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveReceiveRewards(FPacketArchiveReader& Reader)
{
	FItemListHandle Handle;
	Reader << Handle;
	
	BroadcastOnReceiveRewards(Handle);
}

void FPlayerServerDataInterface::OnReceiveGetTalents(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;

	int64 LearnedTalentsId = 0;
	if (bSuccess)
	{
		Reader << LearnedTalentsId;
	}
	
	BroadcastOnGetTalents(LearnedTalentsId, bSuccess);
}

void FPlayerServerDataInterface::OnReceiveGetTasks(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	FSearchTaskResult Result;
	
	Reader << bSuccess;
	if (bSuccess)
	{
		Reader << Result;
	}
	
	BroadcastOnGetTasks(Result, bSuccess);
}

void FPlayerServerDataInterface::OnReceiveDeliverTask(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnDeliverTask(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveAcceptTask(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnAcceptTask(bSuccess);
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
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnModifyTaskTracking(bSuccess);
}

void FPlayerServerDataInterface::OnReceivePropertiesChange(FPacketArchiveReader& Reader)
{
	Reader << CachedProperties;
	GetPlayerDataDelegate().OnPropertiesChange.Broadcast(CachedProperties);
}

#undef LOCTEXT_NAMESPACE
