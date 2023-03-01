#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"

#define PDI_LOCAL TEXT("Local")
#define PDI_SERVER TEXT("Server")

DECLARE_DELEGATE_OneParam(FCompletedSignature, bool);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCompleted, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetWeapons, const TArray<FPlayerWeapon>&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetPlayerInfo, const FPlayerInfo&, bool);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEquipWeapon, const FPlayerWeapon&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEquipModule, const FPlayerModule&, bool);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRegisterServer, const FString& /*ServerId*/, bool);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetStoreItems, const FStoreInformation&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetTalents, int64, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetTasks, const FSearchTaskResult&, bool);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpgradeGear, bool /** upgrade result */, bool /** request result */);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdatedTasks, const TArray<FTaskInProgressInfo>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnReceiveRewards, const FItemListHandle&);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMaterialsChange, const TArray<FPlayerMaterial>&);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLaunchServer, const FLaunchServerResult&, bool);
//DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerReady, const FString& /*ServerAddr*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPropertiesChange, const FPlayerProperties&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskConditionStateChange, float /* Progress */);

DECLARE_DELEGATE_OneParam(FOnReconnectServer, bool);

DECLARE_DELEGATE(FOnServerConnectionLose);


#define DEFINE_PDI_GROUP_DELEGATE_BASE(KeyType, DelegateType, DelegateName) \
private: \
	TMap<KeyType, DelegateType> DelegateName; \
public: \
	void Add##DelegateName(const KeyType& _Key, const DelegateType& _Delegate) \
	{ \
		DelegateName.Add(_Key, _Delegate); \
	} \
	\
	void Remove##DelegateName(const KeyType& _Key) \
	{ \
		DelegateName.Remove(_Key); \
	}


#define DEFINE_PDI_DELEGATE_BASE(DelegateType, DelegateName) \
private: \
	DelegateType DelegateName; \
public: \
	FDelegateHandle Add##DelegateName(const DelegateType::FDelegate& _Delegate) \
	{ \
		return DelegateName.Add(_Delegate); \
	} \
\
	void Remove##DelegateName(FDelegateHandle& _Handle) \
	{ \
		if (_Handle.IsValid()) \
		{ \
			DelegateName.Remove(_Handle); \
			_Handle.Reset();\
		} \
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
	

#define DEFINE_PDI_GROUP_DELEGATE(KeyType, DelegateType, DelegateName) \
	DEFINE_PDI_GROUP_DELEGATE_BASE(KeyType, DelegateType, DelegateName) \
	void Trigger##DelegateName(KeyType _Key) \
	{ \
		if (DelegateType* _DelegatePtr = DelegateName.Find(_Key)) \
		{ \
			_DelegatePtr->ExecuteIfBound();\
		} \
	}

#define DEFINE_PDI_GROUP_DELEGATE_OneParam(KeyType, DelegateType, DelegateName, Param1) \
	DEFINE_PDI_GROUP_DELEGATE_BASE(KeyType, DelegateType, DelegateName) \
	void Trigger##DelegateName(KeyType _Key, Param1 _P1) \
	{ \
		if (DelegateType* _DelegatePtr = DelegateName.Find(_Key)) \
		{ \
			_DelegatePtr->ExecuteIfBound(_P1);\
		} \
	}

#define DEFINE_PDI_GROUP_DELEGATE_TwoParams(KeyType, DelegateType, DelegateName, Param1, Param2) \
	DEFINE_PDI_GROUP_DELEGATE_BASE(KeyType, DelegateType, DelegateName) \
	void Trigger##DelegateName(KeyType _Key, Param1 _P1, Param2 _P2) \
	{ \
		if (DelegateType* _DelegatePtr = DelegateName.Find(_Key)) \
		{ \
			_DelegatePtr->ExecuteIfBound(_P1, _P2);\
		} \
	}

#define DEFINE_PDI_GROUP_DELEGATE_ThreeParams(KeyType, DelegateType, DelegateName, Param1, Param2, Param3) \
	DEFINE_PDI_GROUP_DELEGATE_BASE(KeyType, DelegateType, DelegateName) \
	void Trigger##DelegateName(KeyType _Key, Param1 _P1, Param2 _P2, Param3 _P3) \
	{ \
		if (DelegateType* _DelegatePtr = DelegateName.Find(_Key)) \
		{ \
			_DelegatePtr->ExecuteIfBound(_P1, _P2, _P3);\
		} \
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
	virtual void AddPlayerRewards(const FItemListParam& Rewards) = 0;

	virtual void EquipWeapon(const FEquipWeaponParam& Param) = 0;

	virtual void EquipModule(const FEquipModuleParam& Param) = 0;

	virtual void DecomposeItem(const FDecomposeParam& Param) = 0;

	virtual void UpgradeWeapon(const int64& WeaponId) = 0;
	
	virtual void UpgradeModule(const int64& ModuleId) = 0;
	
	virtual void LearningTalents(const int64& LearnedTalents) = 0;

	virtual void GetStoreItems(const FSearchStoreItemsParam& Param) = 0;

	virtual void PayItem(const int64& ItemId) = 0;

	virtual void GetPlayerInfo(const int32& Condition) = 0;
	
	virtual void ServerGetPlayerInfo(const FQueryPlayerParam& Param) = 0;
	
	virtual void GetTalents(const ETalentCategory& TalentCategory) = 0;

	virtual void GetTasks(const FSearchTaskParam& Param) = 0;

	virtual void DeliverTask(const int64& TaskId) = 0;
	virtual void AcceptTask(const FAcceptTaskParam& Param) = 0;
	virtual void ModifyTrackingState(const FModifyTrackingParam& Param) = 0;
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) = 0;

	virtual void LaunchDedicatedServer(const FLaunchServerParam& Parameter) = 0;
	virtual void NotifyServerLaunched(const FLaunchNotifyParam& Param) = 0;

	virtual void Login() = 0;
	virtual void Logout() = 0;
	
	virtual int32 GetClientPlayerID() = 0;

	virtual TSharedPtr<FInternetAddr> GetBackendServerAddr() = 0;

	virtual const FPlayerProperties& GetCachedProperties() const = 0;

	virtual FMaterialsHandle& GetMaterialsHandle() = 0;

	virtual FPlayerDataDelegate& GetPlayerDataDelegate() = 0;

	virtual FOnServerConnectionLose& OnServerConnectionLoseDelegate() = 0;

	virtual void ReconnectToSpecifiedServer(uint32 Address, uint32 Port, FOnReconnectServer Delegate) = 0;

	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnInitialize, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnLogin, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnLearnTalents, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnBuyItem, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnAcceptTask, bool);
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnModifyTaskTracking, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetStoreItems, OnGetStoreItems, const FStoreInformation&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetWeapons, OnGetWeapons, const TArray<FPlayerWeapon>&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetPlayerInfo, OnGetPlayerInfo, const FPlayerInfo&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetTalents, OnGetTalents, int64, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnGetTasks, OnGetTasks, const FSearchTaskResult&, bool);

	DEFINE_PDI_DELEGATE_TwoParams(FOnEquipWeapon, OnEquipWeapon, const FPlayerWeapon&, bool);
	DEFINE_PDI_DELEGATE_TwoParams(FOnEquipModule, OnEquipModule, const FPlayerModule&, bool);
	
	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnDeliverTask, bool);
	
	DEFINE_PDI_DELEGATE_TwoParams(FOnRegisterServer, OnRegisterServer, const FString&, bool);
	
	DEFINE_PDI_DELEGATE_OneParam(FOnUpdatedTasks, OnUpdateTaskCond, const TArray<FTaskInProgressInfo>&);
	DEFINE_PDI_DELEGATE_OneParam(FOnReceiveRewards, OnReceiveRewards, const FItemListHandle&);

	DEFINE_PDI_DELEGATE_TwoParams(FOnLaunchServer, OnLaunchServer, const FLaunchServerResult&, bool);

	DEFINE_PDI_DELEGATE_OneParam(FOnCompleted, OnDecomposeItem, bool);
	
	DEFINE_PDI_DELEGATE_TwoParams(FOnUpgradeGear, OnUpgradeGear, bool, bool);
};
