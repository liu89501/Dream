// ReSharper disable All
#include "PDI/PlayerServerDataInterface.h"
#include "OnlineIdentityInterface.h"
#include "JsonUtilities.h"
#include "JsonWriter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "OnlineSubsystem.h"
#include "SocketSubsystem.h"

#define LOCTEXT_NAMESPACE "FPlayerServerDataInterface"

void FPlayerServerDataInterface::AddPlayerRewards(UItemData* Rewards, FCommonCompleteNotify Delegate)
{
}

void FPlayerServerDataInterface::DeliverTask(const int64& TaskId, FTaskRewardDelegate Delegate)
{
	
}

void FPlayerServerDataInterface::AcceptTask(const int64& TaskId, FCommonCompleteNotify Delegate)
{
}

void FPlayerServerDataInterface::UpdateTaskState(const FQuestActionHandle& Handle)
{
}

void FPlayerServerDataInterface::ModifyTrackingState(const int64& TaskId, bool bTracking,
	FCommonCompleteNotify Delegate)
{
}

const FPlayerProperties& FPlayerServerDataInterface::GetCachedProperties() const
{
	return CachedProperties;
}

void FPlayerServerDataInterface::RefreshPlayerProperties()
{
}

void FPlayerServerDataInterface::EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate)
{
}

void FPlayerServerDataInterface::LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate)
{
}

void FPlayerServerDataInterface::LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate)
{
}

void FPlayerServerDataInterface::EquipWeapon(int64 WeaponId, int32 EquipmentIndex, FCommonCompleteNotify Delegate)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/player/weapon/switch"));
	Request->SetContentAsString(FString::Printf(TEXT("{\"weaponId\":%d,\"equipmentIndex\":%d}"), WeaponId, EquipmentIndex));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnSwitchWeaponCompleteTrigger, Delegate);
	if (!Request->ProcessRequest())
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerServerDataInterface::GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate)
{
	FHttpRequestRef Request = MakeGetRequest(FString::Printf(TEXT("/player/storeItems/%d"), StoreId));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnGetStoreItemsCompleteTrigger, Delegate);
	/*if (!Request->ProcessRequest())
	{
		Delegate.ExecuteIfBound(Empty, MSG_ERROR);
	}*/
}

void FPlayerServerDataInterface::PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate)
{
	FHttpRequestRef Request = MakeGetRequest(FString::Printf(TEXT("/player/payItem/%d"), ItemId));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnPayItemCompleteTrigger, Delegate);
	if (!Request->ProcessRequest())
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerServerDataInterface::GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate)
{
	FString URL(Condition == EGetEquipmentCondition::Equipped ? TEXT("/player/weapon") : TEXT("/player/weapon/equipped"));

	FHttpRequestRef Request = MakeGetRequest(URL);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnGetWeaponCompleteTrigger, Delegate);
	if (!Request->ProcessRequest())
	{
		TArray<FPlayerWeapon> Empty;
		Delegate.ExecuteIfBound(Empty, MSG_ERROR);
	}
}

void FPlayerServerDataInterface::GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate)
{
	FHttpRequestRef Request = MakeGetRequest(TEXT("/player/info"));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnGetPlayerInfoCompleteTrigger, Delegate);
	if (!Request->ProcessRequest())
	{
		FPlayerInfo PlayerInfo;
		Delegate.ExecuteIfBound(PlayerInfo, MSG_ERROR);
	}
}

void FPlayerServerDataInterface::GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate)
{
	
}

void FPlayerServerDataInterface::GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate)
{
}

void FPlayerServerDataInterface::GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate)
{
}

void FPlayerServerDataInterface::OnGetWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetWeaponComplete Delegate)
{
	TArray<FPlayerWeapon> Weapons;
	FString ErrorMessage;

	if (bConnectedSuccessfully)
	{
		FString ResponseContent = Response->GetContentAsString();
		
		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseContent);
		if (FJsonSerializer::Deserialize(Reader, Object))
		{
			if (Object->GetBoolField(TEXT("status")))
			{
				const TArray<TSharedPtr<FJsonValue>>& WeaponData = Object->GetArrayField(TEXT("data"));
				FJsonObjectConverter::JsonArrayToUStruct<FPlayerWeapon>(WeaponData, &Weapons, 0, 0);
			}
			else
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
			}
		}
		else
		{
			ErrorMessage = TEXT("Json 解析失败");
		}
	}
	else
	{
		ErrorMessage = TEXT("连接异常");
	}

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG_ONLINE(Error, TEXT("武器获取接口异常: %s"), *ErrorMessage);
	}

	Delegate.ExecuteIfBound(Weapons, ErrorMessage);
}

void FPlayerServerDataInterface::OnGetPlayerInfoCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetPlayerInfoComplete Delegate)
{
	FPlayerInfo PlayerInfo;
	FString ErrorMessage;

	if (bConnectedSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;

		FString ContentString = Response->GetContentAsString();
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ContentString);
		if (FJsonSerializer::Deserialize(Reader, Object))
		{
			if (Object->GetBoolField(TEXT("status")))
			{
				UE_LOG_ONLINE(Log, TEXT("玩家信息接口: %s"), *ContentString);
				const TSharedPtr<FJsonObject>& Data = Object->GetObjectField(TEXT("data"));
				FJsonObjectConverter::JsonObjectToUStruct(Data.ToSharedRef(), &PlayerInfo);
			}
			else
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
			}
		}
		else
		{
			ErrorMessage = TEXT("玩家信息获取接口Json解析失败");
		}
	}
	else
	{
		ErrorMessage = TEXT("GetPlayerInfo 接口连接异常");
	}

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG_ONLINE(Error, TEXT("玩家信息接口异常: %s"), *ErrorMessage);
	}

	Delegate.ExecuteIfBound(PlayerInfo, ErrorMessage);
}

void FPlayerServerDataInterface::OnAddItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{

}

void FPlayerServerDataInterface::OnPayItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate)
{
	FString ErrorMessage;
	bool bSuccessfully = bConnectedSuccessfully;
	if (bSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		bSuccessfully = FJsonSerializer::Deserialize(Reader, Object);

		if (bSuccessfully)
		{
			bSuccessfully = Object->GetBoolField(TEXT("status"));
			if (!bSuccessfully)
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
				UE_LOG_ONLINE(Error, TEXT("购买物品接口错误: %s"), *ErrorMessage);
			}
		}
	}

	Delegate.ExecuteIfBound(ErrorMessage);
}

void FPlayerServerDataInterface::OnGetStoreItemsCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetStoreItemsComplete Delegate)
{
	FString ErrorMessage;
	//TArray<FDItemInfo> Items;
	bool bSuccessfully = bConnectedSuccessfully;
	if (bSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		bSuccessfully = FJsonSerializer::Deserialize(Reader, Object);

		if (bSuccessfully)
		{
			bSuccessfully = Object->GetBoolField(TEXT("status"));
			if (bSuccessfully)
			{
				const TArray<TSharedPtr<FJsonValue>>& JsonValues = Object->GetArrayField(TEXT("data"));
				//FJsonObjectConverter::JsonArrayToUStruct<FDItemInfo>(JsonValues, &Items, 0, 0);
			}
			else
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
				UE_LOG_ONLINE(Error, TEXT("商店物品获取接口错误: %s"), *ErrorMessage);
			}
		}
	}
	
	//Delegate.ExecuteIfBound(Items, ErrorMessage);
}

void FPlayerServerDataInterface::OnSwitchWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate)
{
	FString ErrorMessage;
	if (bConnectedSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;
		TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, Object))
		{
			if (!Object->GetBoolField(TEXT("status")))
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
			}
		}
		else
		{
			ErrorMessage = TEXT("Json解析失败");
		}
	}
	else
	{
		ErrorMessage = TEXT("连接失败");
	}

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG_ONLINE(Error, TEXT("武器更换失败: %s"), *ErrorMessage);
	}

	Delegate.ExecuteIfBound(ErrorMessage);
}

#undef LOCTEXT_NAMESPACE