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
	CALLBACK_BINDING_RAW(TService_AddRewards::MarkId, this, &FPlayerServerDataInterface::OnReceiveAddPlayerRewards);
	CALLBACK_BINDING_RAW(TService_EquipWeapon::MarkId, this, &FPlayerServerDataInterface::OnReceiveEquipWeapon);
	CALLBACK_BINDING_RAW(TService_EquipModule::MarkId, this, &FPlayerServerDataInterface::OnReceiveEquipModule);
	CALLBACK_BINDING_RAW(TService_LearningTalents::MarkId, this, &FPlayerServerDataInterface::OnReceiveLearningTalents);
	CALLBACK_BINDING_RAW(TService_GetStoreItems::MarkId, this, &FPlayerServerDataInterface::OnReceiveGetStoreItems);
	CALLBACK_BINDING_RAW(TService_PayItem::MarkId, this, &FPlayerServerDataInterface::OnReceivePayItem);
	CALLBACK_BINDING_RAW(TService_GetTalents::MarkId, this, &FPlayerServerDataInterface::OnReceiveGetTalents);
	CALLBACK_BINDING_RAW(TService_GetTasks::MarkId, this, &FPlayerServerDataInterface::OnReceiveGetTasks);
	CALLBACK_BINDING_RAW(TService_DeliverTask::MarkId, this, &FPlayerServerDataInterface::OnReceiveDeliverTask);
	CALLBACK_BINDING_RAW(TService_AcceptTask::MarkId, this, &FPlayerServerDataInterface::OnReceiveAcceptTask);
	CALLBACK_BINDING_RAW(TService_UpdateTaskState::MarkId, this, &FPlayerServerDataInterface::OnReceiveUpdateTaskState);
	CALLBACK_BINDING_RAW(TService_ModifyTrackingState::MarkId, this, &FPlayerServerDataInterface::OnReceiveModifyTrackingState);
}

void FPlayerServerDataInterface::Initialize()
{
	FPlayerDataInterfaceBase::Initialize();

	BroadcastOnInitialize(true);
}

void FPlayerServerDataInterface::AddPlayerRewards(const FItemDataHandle& Rewards)
{
	SocketSender->Send(PDIBuildParam<TService_AddRewards>(Rewards));
}

void FPlayerServerDataInterface::DeliverTask(int32 TaskId)
{
	SocketSender->Send(PDIBuildParam<TService_DeliverTask>(TaskId));
}

void FPlayerServerDataInterface::AcceptTask(const FAcceptTaskParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_AcceptTask>(Param));
}

void FPlayerServerDataInterface::UpdateTaskState(const FQuestActionHandle& Handle)
{
	
	// todo 更新任务进度状态
}

void FPlayerServerDataInterface::ModifyTrackingState(const FModifyTrackingParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_ModifyTrackingState>(Param));
}

void FPlayerServerDataInterface::AddTaskConditionStateChangeDelegate(int32 TaskId, FOnTaskConditionStateChange Delegate)
{
	Delegates.Add(TaskId, Delegate);
}

void FPlayerServerDataInterface::RemoveTaskConditionStateChangeDelegate(int32 TaskId)
{
	Delegates.Remove(TaskId);
}

const FPlayerProperties& FPlayerServerDataInterface::GetCachedProperties() const
{
	return CachedProperties;
}

void FPlayerServerDataInterface::EquipModule(const FEquipModuleParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_EquipModule>(Param));
}

void FPlayerServerDataInterface::LearningTalents(const TArray<int32>& TalentIdArray)
{
	SocketSender->Send(PDIBuildParam<TService_LearningTalents>(TalentIdArray));
}

void FPlayerServerDataInterface::EquipWeapon(const FEquipWeaponParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_EquipWeapon>(Param));
}

void FPlayerServerDataInterface::GetStoreItems(int32 StoreId)
{
	SocketSender->Send(PDIBuildParam<TService_GetStoreItems>(StoreId));
}

void FPlayerServerDataInterface::PayItem(const FBuyItemParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_PayItem>(Param));
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

void FPlayerServerDataInterface::OnReceiveAddPlayerRewards(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	
	BroadcastOnAddPlayerRewards(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveGetTalents(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	TArray<FTalentInfo> Talents;
	
	Reader << bSuccess;
	if (bSuccess)
	{
		Reader << Talents;
	}
	
	BroadcastOnGetTalents(Talents, bSuccess);
}

void FPlayerServerDataInterface::OnReceiveGetTasks(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	TArray<FTaskInformation> TaskInformations;
	
	Reader << bSuccess;
	if (bSuccess)
	{
		EGetTaskCondition Condition;
		Reader << Condition;
		
		int32 TaskGroupId;
		Reader << TaskGroupId;

		UTaskDataAsset* TaskDataAsset = FPDIStatic::GetTaskDataAsset();

		if (Condition == EGetTaskCondition::NotAccept)
		{
			TSet<int32> AcceptTaskId;
			Reader << AcceptTaskId;

			for (const FTaskInformation& Information : TaskDataAsset->Tasks)
			{
				if (AcceptTaskId.Contains(Information.TaskId))
				{
					continue;
				}

				FTaskInformation Duplicted(Information);
				
				Duplicted.CompleteCondition = NewObject<UDQuestCondition>(GetTransientPackage(),
					Information.CompleteCondition->GetClass(), NAME_None, RF_NoFlags, Information.CompleteCondition);
				
				Duplicted.CompletedReward = NewObject<UItemData>(GetTransientPackage(),
					Information.CompletedReward->GetClass(), NAME_None, RF_NoFlags, Information.CompletedReward);
				
				TaskInformations.Add(Duplicted);
			}
		}
		else
		{
			TArray<FNetTaskInformation> NetTaskInformations;
			Reader << NetTaskInformations;

			for (const FNetTaskInformation& AcceptTask : NetTaskInformations)
			{
				FTaskInformation TaskInformation;
				
				if (TaskDataAsset->GetInformationByTaskId(AcceptTask.TaskId, TaskInformation))
				{
					TaskInformation.bTracking = AcceptTask.bTracking;
					TaskInformation.TaskMark = AcceptTask.TaskMark;
					TaskInformation.CompleteCondition->CurrentValue = AcceptTask.ProgressCurrentVal;

					TaskInformation.CompleteCondition = NewObject<UDQuestCondition>(GetTransientPackage(),
						TaskInformation.CompleteCondition->GetClass(), NAME_None, RF_NoFlags, TaskInformation.CompleteCondition);
				
					TaskInformation.CompletedReward = NewObject<UItemData>(GetTransientPackage(),
                        TaskInformation.CompletedReward->GetClass(), NAME_None, RF_NoFlags, TaskInformation.CompletedReward);
					
					TaskInformations.Add(TaskInformation);
				}
			}
		}
	}
	
	BroadcastOnGetTasks(TaskInformations, bSuccess);
}

void FPlayerServerDataInterface::OnReceiveDeliverTask(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	FItemDataHandle ItemData;
	
	Reader << bSuccess;
	if (bSuccess)
	{
		Reader << ItemData;
	}
	
	BroadcastOnDeliverTask(ItemData.Get(),bSuccess);
}

void FPlayerServerDataInterface::OnReceiveAcceptTask(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnAcceptTask(bSuccess);
}

void FPlayerServerDataInterface::OnReceiveUpdateTaskState(FPacketArchiveReader& Reader)
{
	
}

void FPlayerServerDataInterface::OnReceiveModifyTrackingState(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;
	BroadcastOnModifyTaskTracking(bSuccess);
}

void FPlayerServerDataInterface::OnReceivePropertiesChange(FPacketArchiveReader& Reader)
{
	bool bSuccess;
	Reader << bSuccess;

	if (bSuccess)
	{
		Reader << CachedProperties;

		GetPlayerDataDelegate().OnPropertiesChange.Broadcast(CachedProperties);
	}
}

#undef LOCTEXT_NAMESPACE
