// ReSharper disable All
#include "PlayerDataInterfaceNull.h"

class FInternetAddrNull : public FInternetAddr
{
public:

	FInternetAddrNull(): FInternetAddr()
	{
	}

	virtual ~FInternetAddrNull()
	{
	}


	virtual bool operator==(const FInternetAddr& Other) const override
	{
		return true;
	}

	virtual bool CompareEndpoints(const FInternetAddr& InAddr) const override
	{
		return true;
	}

	virtual void SetIp(uint32 InAddr) override
	{
		
	}
	virtual void SetIp(const TCHAR* InAddr, bool& bIsValid) override
	{
		
	}
	virtual void GetIp(uint32& OutAddr) const override
	{
		
	}
	virtual void SetPort(int32 InPort) override
	{
		
	}
	virtual int32 GetPort() const override
	{
		return 0;
	}
	virtual void SetRawIp(const TArray<uint8>& RawAddr) override
	{
		
	}
	virtual TArray<uint8> GetRawIp() const override
	{
		TArray<uint8> Empty;
		return Empty;
	}
	virtual void SetAnyAddress() override
	{
		
	}
	virtual void SetBroadcastAddress() override
	{
		
	}
	virtual void SetLoopbackAddress() override
	{
		
	}
	virtual FString ToString(bool bAppendPort) const override
	{
		return TEXT("Null");
	}
	virtual uint32 GetTypeHash() const override
	{
		return 0;
	}
	virtual bool IsValid() const override
	{
		return true;
	}
	virtual TSharedRef<FInternetAddr> Clone() const override
	{
		return MakeShared<FInternetAddrNull>();
	}
};

FPlayerDataInterfaceNull::~FPlayerDataInterfaceNull()
{
}

void FPlayerDataInterfaceNull::Initialize()
{
}

void FPlayerDataInterfaceNull::AddPlayerRewards(const FItemListHandle& Rewards)
{
}

void FPlayerDataInterfaceNull::EquipWeapon(const FEquipWeaponParam& Param)
{
}

void FPlayerDataInterfaceNull::EquipModule(const FEquipModuleParam& Param)
{
}

void FPlayerDataInterfaceNull::LearningTalents(int64 LearnedTalents)
{
}

void FPlayerDataInterfaceNull::GetStoreItems(const FSearchStoreItemsParam& Param)
{
}

void FPlayerDataInterfaceNull::PayItem(int64 ItemId)
{
}

void FPlayerDataInterfaceNull::GetPlayerInfo(EGetEquipmentCondition Condition)
{
}

void FPlayerDataInterfaceNull::GetTalents(ETalentCategory TalentCategory)
{
}

void FPlayerDataInterfaceNull::GetTasks(const FSearchTaskParam& Param)
{
}

void FPlayerDataInterfaceNull::DeliverTask(int64 TaskId)
{
}

void FPlayerDataInterfaceNull::AcceptTask(const FAcceptTaskParam& Param)
{
}

void FPlayerDataInterfaceNull::ModifyTrackingState(const FModifyTrackingParam& Param)
{
}

void FPlayerDataInterfaceNull::UpdateTaskState(const FQuestActionHandle& Handle)
{
}

void FPlayerDataInterfaceNull::RegisterServer(const FDedicatedServerInformation& Information)
{
}

void FPlayerDataInterfaceNull::UpdateActivePlayers(const FUpdateServerPlayerParam& Param)
{
}

void FPlayerDataInterfaceNull::NotifyBackendServer(const FLaunchNotifyParam& Param)
{
}

void FPlayerDataInterfaceNull::SearchDedicatedServer(const FSearchServerParam& Parameter)
{
}

void FPlayerDataInterfaceNull::Login()
{
}

void FPlayerDataInterfaceNull::Logout()
{
}

int32 FPlayerDataInterfaceNull::GetClientPlayerID()
{
	return 0;
}

TSharedPtr<FInternetAddr> FPlayerDataInterfaceNull::GetBackendServerAddr()
{
	return MakeShared<FInternetAddrNull>();
}

const FPlayerProperties& FPlayerDataInterfaceNull::GetCachedProperties() const
{
	return EmptyProperties;
}

int32 FPlayerDataInterfaceNull::GetCacheItemCount(int32 ItemGuid)
{
	return 0;
}

void FPlayerDataInterfaceNull::IncreaseItemCount(int32 ItemGuid, int32 Delta)
{
}

FPlayerDataDelegate& FPlayerDataInterfaceNull::GetPlayerDataDelegate()
{
	return EmptyDelegate;
}

FOnServerConnectionLose& FPlayerDataInterfaceNull::OnServerConnectionLoseDelegate()
{
	return DummyOnServerConnectionLose;
}

void FPlayerDataInterfaceNull::ReconnectToSpecifiedServer(uint32 Address, uint32 Port, FOnReconnectServer Delegate)
{
}
