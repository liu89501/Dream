#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"

#define PDI_LOCAL TEXT("Local")
#define PDI_SERVER TEXT("Server")

#define MSG_ERROR TEXT("Error")
#define MSG_SUCCESS TEXT("")

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCompleted, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetWeapons, const TArray<FPlayerWeapon>&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetPlayerInfo, const FPlayerInfo&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetServer, const FFindServerResult& /*Result*/, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRegisterServer, const FString& /*ServerId*/, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLaunchNotify, const FString& /*ServerAddr*/, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetStoreItems, const FStoreInformation&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetTalents, const TArray<FTalentInfo>&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetTasks, const TArray<FTaskInformation>&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTaskReward, UItemData*, bool);


DECLARE_MULTICAST_DELEGATE_OneParam(FOnPropertiesChange, const FPlayerProperties&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskConditionStateChange, float /* Progress */);

DECLARE_DELEGATE(FOnServerConnectionLose);

#define DEFINE_PDI_DELEGATE_BASE(DelegateType, DelegateName) \
private: \
	DelegateType DelegateName; \
public: \
	FDelegateHandle Add##DelegateName(DelegateType::FDelegate Delegate) \
	{ \
		return DelegateName.Add(Delegate); \
	} \
\
	void Remove##DelegateName(FDelegateHandle Handle) \
	{ \
		DelegateName.Remove(Handle); \
	}

#define DEFINE_PDI_DELEGATE(DelegateType, DelegateName) \
	DEFINE_PDI_DELEGATE_BASE(DelegateType, DelegateName) \
\
	void Broadcast##DelegateName() \
	{ \
		DelegateName.Broadcast(); \
	}

#define DEFINE_PDI_DELEGATE_OneParam(DelegateType, DelegateName, Param1) \
	DEFINE_PDI_DELEGATE_BASE(DelegateType, DelegateName) \
\
	void Broadcast##DelegateName(Param1 P1) \
	{ \
		DelegateName.Broadcast(P1); \
	}
	
#define DEFINE_PDI_DELEGATE_TwoParams(DelegateType, DelegateName, Param1, Param2) \
	DEFINE_PDI_DELEGATE_BASE(DelegateType, DelegateName) \
\
	void Broadcast##DelegateName(Param1 P1, Param2 P2) \
	{ \
		DelegateName.Broadcast(P1, P2); \
	}
	
#define DEFINE_PDI_DELEGATE_ThreeParams(DelegateType, DelegateName, Param1, Param2, Param3) \
	DEFINE_PDI_DELEGATE_BASE(DelegateType, DelegateName) \
\
	void Broadcast##DelegateName(Param1 P1, Param2 P2, Param3 P3) \
	{ \
		DelegateName.Broadcast(P1, P2, P3); \
	}
	

struct FPlayerDataDelegate
{
	FOnPropertiesChange OnPropertiesChange;
};

class FPlayerDataInterface
{
	
public:
	virtual ~FPlayerDataInterface() = default;

	virtual void Initialize() = 0;

	// Server
	virtual void AddPlayerRewards(const FItemDataHandle& Rewards) = 0;

	// Local
	virtual void EquipWeapon(const FEquipWeaponParam& Param) = 0;

	// Local
	virtual void EquipModule(const FEquipModuleParam& Param) = 0;

	// Local
	virtual void LearningTalents(const TArray<int32>& TalentIdArray) = 0;

	virtual void GetStoreItems(int32 StoreId) = 0;

	// Local
	virtual void PayItem(const FBuyItemParam& Param) = 0;

	virtual void GetPlayerInfo(EGetEquipmentCondition Condition) = 0;
	
	virtual void GetTalents(ETalentCategory TalentCategory) = 0;

	virtual void GetTasks(const FSearchTaskParam& Param) = 0;

	// Local
	virtual void DeliverTask(int32 TaskId) = 0;
	virtual void AcceptTask(const FAcceptTaskParam& Param) = 0;
	virtual void ModifyTrackingState(const FModifyTrackingParam& Param) = 0;

	// Server
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) = 0;
	virtual void RegisterServer(const FDedicatedServerInformation& Information) = 0;
	virtual void UpdateActivePlayers(const FUpdateServerPlayerParam& Param) = 0;
	virtual void NotifyBackendServer(const FLaunchNotifyParam& Param) = 0;

	// local
	virtual void RunNewDedicatedServer(const FRunServerParameter& Parameter) = 0;

	// Local
	virtual void Login() = 0;
	virtual void Logout() = 0;

	virtual const FPlayerProperties& GetCachedProperties() const = 0;

	virtual FPlayerDataDelegate& GetPlayerDataDelegate() = 0;

	virtual FOnServerConnectionLose& OnServerConnectionLoseDelegate() = 0;

	virtual void AddTaskConditionStateChangeDelegate(int32 TaskId, FOnTaskConditionStateChange Delegate) = 0;
	virtual void RemoveTaskConditionStateChangeDelegate(int32 TaskId) = 0;

	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnInitialize, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnLogin, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnEquipWeapon, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnEquipModule, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnLearnTalents, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnAddPlayerRewards, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnBuyItem, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnAcceptTask, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnModifyTaskTracking, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetStoreItems, OnGetStoreItems, const FStoreInformation&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetWeapons, OnGetWeapons, const TArray<FPlayerWeapon>&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetPlayerInfo, OnGetPlayerInfo, const FPlayerInfo&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetTalents, OnGetTalents, const TArray<FTalentInfo>&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetTasks, OnGetTasks, const TArray<FTaskInformation>&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnTaskReward, OnDeliverTask, UItemData*, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetServer, OnGetServer, const FFindServerResult&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnRegisterServer, OnRegisterServer, const FString&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnLaunchNotify, OnLaunchNotify, const FString&, bool);
};
