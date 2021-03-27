﻿#include "PlayerDataStore.h"

#include "OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "JsonUtilities.h"
#include "JsonWriter.h"
#include "HTTP.h"
#include "HttpModule.h"
#include "IHttpResponse.h"

DEFINE_LOG_CATEGORY(LogPDS)

typedef TJsonWriterFactory< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriterFactory;
typedef TJsonWriter< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriter;

#define LOCTEXT_NAMESPACE "FPlayerDataStore"

FPlayerDataStore::FPlayerDataStore(IOnlineSubsystem* OSSystem)
	: OSS(OSSystem)
{
	GConfig->GetString(TEXT("ServerSettings"), TEXT("ServerURL"), ServerURL, GEngineIni);

	FString AESKey;
	GConfig->GetString(TEXT("SecretKey"), TEXT("AESKey"), AESKey, GEngineIni);
	AESUtils = FAESUtils(AESKey);

	FParse::Value(FCommandLine::Get(), TEXT("ServerToken="), ServerToken);
}

FPlayerDataStore::~FPlayerDataStore()
{
	if (IsRunningDedicatedServer())
	{
		UnRegisterServer();
	}
	else
	{
		Logout();
	}
	
	OSS = nullptr;
	UE_LOG_ONLINE(Log, TEXT("FPlayerDataStore Free..."));
}

bool FPlayerDataStore::RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/register"));

	TSharedPtr<FJsonObject> ServerInfo = MakeShareable(new FJsonObject);
	ServerInfo->SetNumberField(TEXT("Port"), Port);
	ServerInfo->SetNumberField(TEXT("MaxPlayers"), MaxPlayers);
	ServerInfo->SetStringField(TEXT("MapName"), MapName);

	FString JsonString;

	TSharedRef<FCondensedJsonStringWriter> Writer = FCondensedJsonStringWriterFactory::Create(&JsonString);
	FJsonSerializer::Serialize(ServerInfo.ToSharedRef(), Writer);

	FString EncryptString;
	AESUtils.EncryptData(JsonString, EncryptString);

	Request->SetContentAsString(EncryptString);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnRegisterCompleteTrigger);

	return Request->ProcessRequest();
}

void FPlayerDataStore::UnRegisterServer()
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/unregister"), TEXT("text/plain"));
	Request->SetContentAsString(ServerID);
	Request->ProcessRequest();
}

void FPlayerDataStore::UpdateActivePlayers(bool bIncrement)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/updatePlayers"));
	Request->SetContentAsString(FString::Printf(TEXT("{\"serverId\":\"%s\",\"increment\":%s}"), *ServerID, bIncrement ? TEXT("true") : TEXT("false")));
	Request->ProcessRequest();
}

bool FPlayerDataStore::Login()
{

	FString ErrorMessage;
	if (!Cookie.IsEmpty())
	{
		OnLoginComplete.Broadcast(ErrorMessage);
		return true;
	}
	
	bool bSuccessfully = false;
	IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	if (Identity.IsValid())
	{
		FString Ticket = Identity->GetAuthToken(0);

		TSharedPtr<FJsonObject> LoginParam = MakeShareable(new FJsonObject);
		LoginParam->SetStringField(TEXT("platformName"), OSS->GetSubsystemName().ToString());
		LoginParam->SetStringField(TEXT("thirdPartyUserTicket"), Ticket);

		FString OutputString;
		TSharedRef<FCondensedJsonStringWriter> Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
		if (FJsonSerializer::Serialize(LoginParam.ToSharedRef(), Writer))
		{
			FHttpRequestRef Request = MakePostRequest(TEXT("/player/login"));
			Request->SetContentAsString(OutputString);
			Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnLoginCompleteTrigger);
			bSuccessfully = Request->ProcessRequest();
			if (!bSuccessfully)
			{
				ErrorMessage = TEXT("Login request failed");
			}
		}
		else
		{
			ErrorMessage = TEXT("Failed to serialize login Json parameters");
		}
	}
	else
	{
		ErrorMessage = TEXT("OSS IdentityInterface Invalid");
	}

	if (!bSuccessfully)
	{
		OnLoginComplete.Broadcast(ErrorMessage);
	}

	return bSuccessfully;
}

void FPlayerDataStore::Logout()
{
	FHttpRequestRef Request = MakeGetRequest(TEXT("/player/logout"));
	Request->ProcessRequest();
}

IOnlineSubsystem* FPlayerDataStore::GetOSS() const
{
	return OSS;
}

bool FPlayerDataStore::AddWeapons(const TArray<FPlayerWeaponAdd>& NewWeapons)
{
	TArray<TSharedPtr<FJsonValue>> WeaponArray;

	for (FPlayerWeaponAdd Weapon : NewWeapons)
	{
		TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject<FPlayerWeaponAdd>(Weapon);
		WeaponArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
	}

	FString OutputString;
	TSharedRef<FCondensedJsonStringWriter> Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	if (!FJsonSerializer::Serialize(WeaponArray, Writer))
	{
		return false;
	}

	FString EncryptString;
	AESUtils.EncryptData(OutputString, EncryptString);

	FHttpRequestRef Request = MakePostRequest(TEXT("/server/player/addWeapons"), TEXT("text/plain"));
	Request->SetContentAsString(EncryptString);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnAddWeaponCompleteTrigger);

	return Request->ProcessRequest();
}

bool FPlayerDataStore::EquipmentWeapon(int32 WeaponId, int32 EquipmentIndex)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/player/weapon/switch"));
	Request->SetContentAsString(FString::Printf(TEXT("{\"weaponId\":%d,\"equipmentIndex\":%d}"), WeaponId, EquipmentIndex));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnSwitchWeaponCompleteTrigger);
	return Request->ProcessRequest();
}

bool FPlayerDataStore::GetStoreItems(int32 StoreId)
{
	FHttpRequestRef Request = MakeGetRequest(FString::Printf(TEXT("/player/storeItems/%d"), StoreId));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnGetStoreItemsCompleteTrigger);
	return Request->ProcessRequest();
}

bool FPlayerDataStore::RunServer(const FRunServerParameter& Parameter)
{
	FString JsonParameter;
	if (!FJsonObjectConverter::UStructToJsonObjectString<FRunServerParameter>(Parameter, JsonParameter))
	{
		return false;
	}

	FHttpRequestRef Request = MakePostRequest(TEXT("/player/runServer"));
	Request->SetContentAsString(JsonParameter);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnRunServerCompleteTrigger);
	return Request->ProcessRequest();
}

bool FPlayerDataStore::PayItem(int32 ItemId)
{
	FHttpRequestRef Request = MakeGetRequest(FString::Printf(TEXT("/player/payItem/%d"), ItemId));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnPayItemCompleteTrigger);
	return Request->ProcessRequest();
}

bool FPlayerDataStore::GetPlayerWeapons(EGetWeaponCondition Condition)
{
	FString URL(TEXT("/player/weapon"));

	if (Condition == EGetWeaponCondition::Equipped)
	{
		URL = TEXT("/player/weapon/equipped");
	}

	FHttpRequestRef Request = MakeGetRequest(URL);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnGetWeaponCompleteTrigger);
	return Request->ProcessRequest();
}

bool FPlayerDataStore::GetPlayerInfo()
{
	FHttpRequestRef Request = MakeGetRequest(TEXT("/player/info"));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataStore::OnGetPlayerInfoCompleteTrigger);
	return Request->ProcessRequest();
}

FHttpRequestRef FPlayerDataStore::MakeRequest()
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	if (IsRunningDedicatedServer())
	{
		Request->SetHeader(TEXT("Token"), ServerToken);
	}
	else
	{
		Request->SetHeader(TEXT("Cookie"), Cookie);
	}

	return Request;
}

FHttpRequestRef FPlayerDataStore::MakePostRequest(const FString& URL, FString ContentType)
{
	FHttpRequestRef Request = MakeRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *ServerURL, *URL));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), ContentType);
	return Request;
}

FHttpRequestRef FPlayerDataStore::MakeGetRequest(const FString& URL)
{
	FHttpRequestRef Request = MakeRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *ServerURL, *URL));
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Cookie"), Cookie);
	return Request;
}

void FPlayerDataStore::OnLoginCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FString ErrorMessage;

	if (bConnectedSuccessfully)
	{
		FString ResponseContent = Response->GetContentAsString();

		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseContent);
		bConnectedSuccessfully = FJsonSerializer::Deserialize(Reader, Object);
		if (bConnectedSuccessfully)
		{
			bConnectedSuccessfully = Object->GetBoolField(TEXT("status"));
			if (bConnectedSuccessfully)
			{
				FString RawCookie = Response->GetHeader(TEXT("Set-Cookie"));

				bConnectedSuccessfully = !RawCookie.IsEmpty();

				if (bConnectedSuccessfully)
				{
					int32 Index = 0;
					RawCookie.FindChar(';', Index);
					Cookie = RawCookie.Mid(0, Index).TrimEnd();
				}
			}
			else
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
				UE_LOG_ONLINE(Error, TEXT("后端服务器登陆失败: %s"), *ErrorMessage);
			}
		}
	}
	else
	{
		ErrorMessage = TEXT("Login Fail");
		UE_LOG_ONLINE(Error, TEXT("后端服务器登陆失败: bConnectedSuccessfully invalid"));
	}

	OnLoginComplete.Broadcast(ErrorMessage);
}

void FPlayerDataStore::OnGetWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
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

	OnGetWeaponComplete.Broadcast(Weapons, ErrorMessage);
}

void FPlayerDataStore::OnGetPlayerInfoCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
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

	OnGetPlayerInfoComplete.Broadcast(PlayerInfo, ErrorMessage);
}

void FPlayerDataStore::OnAddWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TArray<FPlayerWeapon> AddedWeapons;

	bool bSuccessfully = bConnectedSuccessfully;
	if (bSuccessfully)
	{
		FString Content = Response->GetContentAsString();

		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Content);

		UE_LOG_ONLINE(Verbose, TEXT("AddWeaponResponse: %s"), *Content);

		bSuccessfully = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessfully)
		{
			bSuccessfully = Object->GetBoolField(TEXT("status"));

			if (!bSuccessfully)
			{
				UE_LOG_ONLINE(Error, TEXT("武器添加接口异常: %s"), *Object->GetStringField(TEXT("msg")));
			}
		}
	}

	if (bSuccessfully)
	{
		OnAddWeaponComplete.Broadcast(bSuccessfully);
	}
}

void FPlayerDataStore::OnRunServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FString ServerAddress;
	FString ErrorMessage;
	
	if (bConnectedSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, Object))
		{
			if (Object->GetBoolField(TEXT("status")))
			{
				const TSharedPtr<FJsonObject>& Data = Object->GetObjectField(TEXT("data"));
				ServerAddress = Data->GetStringField(TEXT("joinAddress"));
			}
			else
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
		ErrorMessage = TEXT("连接异常");
	}

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG_ONLINE(Error, TEXT("运行服务器失败: %s"), *ErrorMessage);
	}
	
	OnRunServerComplete.Broadcast(ServerAddress, ErrorMessage);
}

void FPlayerDataStore::OnAddItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{

}

void FPlayerDataStore::OnPayItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
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

	OnPayItemComplete.Broadcast(ErrorMessage);
}

void FPlayerDataStore::OnGetStoreItemsCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FString ErrorMessage;
	TArray<FStoreItem> Items;
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
				FJsonObjectConverter::JsonArrayToUStruct<FStoreItem>(JsonValues, &Items, 0, 0);
			}
			else
			{
				ErrorMessage = Object->GetStringField(TEXT("msg"));
				UE_LOG_ONLINE(Error, TEXT("商店物品获取接口错误: %s"), *ErrorMessage);
			}
		}
	}
	
	OnGetStoreItemComplete.Broadcast(Items, ErrorMessage);
}

void FPlayerDataStore::OnSwitchWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
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

	OnSwitchWeaponComplete.Broadcast(ErrorMessage);
}

void FPlayerDataStore::OnRegisterCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, Object))
		{
			if (Object->GetBoolField(TEXT("status")))
			{
				ServerID = Object->GetStringField(TEXT("data"));

				if (OnRegisterComplete.IsBound())
				{
					OnRegisterComplete.Broadcast(ServerID);
				}
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("注册服务器失败: %s"), *Object->GetStringField(TEXT("msg")));
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE