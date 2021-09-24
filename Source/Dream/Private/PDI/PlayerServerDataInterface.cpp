// ReSharper disable All
#include "PDI/PlayerServerDataInterface.h"
#include "OnlineIdentityInterface.h"
#include "JsonUtilities.h"
#include "JsonWriter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "PDI/PlayerLocalDataInterface.h"

DEFINE_LOG_CATEGORY(LogPDS)

typedef TJsonWriterFactory< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriterFactory;
typedef TJsonWriter< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriter;

#define LOCTEXT_NAMESPACE "FPlayerServerDataInterface"


FAESUtils::FAESUtils(const FString& InSecretKey)
{
	FBase64::Decode(InSecretKey, SecretKey);
}

void FAESUtils::EncryptData(const FString& Content, FString& Encrypt)
{
	FTCHARToUTF8 UTF8Content(*Content);
	const uint8* ContentPtr = reinterpret_cast<const uint8*>(UTF8Content.Get());
	TArray<uint8> Data(ContentPtr, UTF8Content.Length());

	/**
	PKCS5Padding
	*/
	int32 Padding = FAES::AESBlockSize - (UTF8Content.Length() % FAES::AESBlockSize);

	TArray<uint8> PaddingBlock;
	PaddingBlock.SetNum(Padding);
	FMemory::Memset(PaddingBlock.GetData(), static_cast<uint8>(Padding), Padding);
	Data.Append(PaddingBlock);

	FAES::EncryptData(Data.GetData(), Data.Num(), SecretKey.GetData(), SecretKey.Num());
	Encrypt = FBase64::Encode(Data);
}

FString FAESUtils::DecryptData(const FString& EncryptContent)
{
	TArray<uint8> DecodeContent;
	FBase64::Decode(EncryptContent, DecodeContent);
	FAES::DecryptData(DecodeContent.GetData(), DecodeContent.Num(), SecretKey.GetData(), SecretKey.Num());

	uint8 Padding = DecodeContent.Last();
	DecodeContent.RemoveAt(DecodeContent.Num() - Padding, Padding);
	uint8* ContentData = DecodeContent.GetData();
	*(ContentData + DecodeContent.Num()) = '\0';

	FString DecryptText;
	TArray<TCHAR>& Chars = DecryptText.GetCharArray();
	FUTF8ToTCHAR TChar(reinterpret_cast<ANSICHAR*>(ContentData));
	Chars.Append(TChar.Get(), TChar.Length());
	return DecryptText;
}

FPlayerServerDataInterface::~FPlayerServerDataInterface()
{
	if (IsRunningDedicatedServer())
	{
		FPlayerServerDataInterface::UnRegisterServer();
	}
	else
	{
		FPlayerServerDataInterface::Logout();
	}
	
	OSS = nullptr;
	UE_LOG_ONLINE(Log, TEXT("FPlayerServerDataInterface Free..."));
}

void FPlayerServerDataInterface::Initialize(FInitializeDelegate Delegate)
{
	OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
	
	GConfig->GetString(TEXT("MySettings"), TEXT("ServerURL"), ServerURL, GEngineIni);

	FString AESKey;
	GConfig->GetString(TEXT("MySettings"), TEXT("AESKey"), AESKey, GEngineIni);
	AESUtils = FAESUtils(AESKey);

	FParse::Value(FCommandLine::Get(), TEXT("ServerToken="), ServerToken);

	Delegate.ExecuteIfBound(true);
}

void FPlayerServerDataInterface::AddPlayerRewards(const TArray<UItemData*>& Rewards, FCommonCompleteNotify Delegate)
{
}

void FPlayerServerDataInterface::DeliverTask(const int64& TaskId, FCommonCompleteNotify Delegate)
{
	
}

void FPlayerServerDataInterface::RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName, FRegisterServerComplete Delegate)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/register"));

	bool bSuccess = true;

	TSharedPtr<FJsonObject> ServerInfo = MakeShareable(new FJsonObject);
	ServerInfo->SetNumberField(TEXT("Port"), Port);
	ServerInfo->SetNumberField(TEXT("MaxPlayers"), MaxPlayers);
	ServerInfo->SetStringField(TEXT("MapName"), MapName);

	FString JsonString;

	TSharedRef<FCondensedJsonStringWriter> Writer = FCondensedJsonStringWriterFactory::Create(&JsonString);
	bSuccess &= FJsonSerializer::Serialize(ServerInfo.ToSharedRef(), Writer);

	if (bSuccess)
	{
		FString EncryptString;
		AESUtils.EncryptData(JsonString, EncryptString);

		Request->SetContentAsString(EncryptString);
		Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnRegisterCompleteTrigger, Delegate);

		bSuccess &= Request->ProcessRequest();
	}

	if (!bSuccess)
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerServerDataInterface::UnRegisterServer()
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/unregister"), TEXT("text/plain"));
	Request->SetContentAsString(ServerID);
	Request->ProcessRequest();
}

void FPlayerServerDataInterface::UpdateActivePlayers(bool bIncrement)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/updatePlayers"));
	Request->SetContentAsString(FString::Printf(TEXT("{\"serverId\":\"%s\",\"increment\":%s}"), *ServerID, bIncrement ? TEXT("true") : TEXT("false")));
	Request->ProcessRequest();
}

void FPlayerServerDataInterface::Login(FCommonCompleteNotify Delegate)
{

	FString ErrorMessage;
	if (!Cookie.IsEmpty())
	{
		Delegate.ExecuteIfBound(ErrorMessage);
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
			Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnLoginCompleteTrigger, Delegate);
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
		Delegate.ExecuteIfBound(ErrorMessage);
	}
}

void FPlayerServerDataInterface::Logout()
{
	FHttpRequestRef Request = MakeGetRequest(TEXT("/player/logout"));
	Request->ProcessRequest();
}

FString FPlayerServerDataInterface::GetServerToken() const
{
	return ServerToken;
}

const FPlayerProperties& FPlayerServerDataInterface::GetCachedProperties() const
{
	return CachedProperties;
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

void FPlayerServerDataInterface::RunServer(const FRunServerParameter& Parameter, FGetServerComplete Delegate)
{
	bool bSuccess = true;
	FString JsonParameter;
	bSuccess &= FJsonObjectConverter::UStructToJsonObjectString<FRunServerParameter>(Parameter, JsonParameter);

	if (bSuccess)
	{
		FHttpRequestRef Request = MakePostRequest(TEXT("/player/runServer"));
		Request->SetContentAsString(JsonParameter);
		Request->OnProcessRequestComplete().BindRaw(this, &FPlayerServerDataInterface::OnRunServerCompleteTrigger, Delegate);
		bSuccess &= Request->ProcessRequest();
	}
	
	if (!bSuccess)
	{
		Delegate.ExecuteIfBound(TEXT(""), MSG_ERROR);
	}
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

void FPlayerServerDataInterface::IncreaseExperience(const FUserExperiencePair& UserExperience, FExperienceChangeDelegate Delegate)
{
}

FHttpRequestRef FPlayerServerDataInterface::MakeRequest() const
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

FHttpRequestRef FPlayerServerDataInterface::MakePostRequest(const FString& URL, FString ContentType) const
{
	FHttpRequestRef Request = MakeRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *ServerURL, *URL));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), ContentType);
	return Request;
}

FHttpRequestRef FPlayerServerDataInterface::MakeGetRequest(const FString& URL) const
{
	FHttpRequestRef Request = MakeRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *ServerURL, *URL));
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Cookie"), Cookie);
	return Request;
}

void FPlayerServerDataInterface::OnLoginCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate)
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

	Delegate.ExecuteIfBound(ErrorMessage);
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

void FPlayerServerDataInterface::OnRunServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetServerComplete Delegate)
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
	
	Delegate.ExecuteIfBound(ServerAddress, ErrorMessage);
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

void FPlayerServerDataInterface::OnRegisterCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FRegisterServerComplete Delegate)
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
				Delegate.ExecuteIfBound(ServerID);
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("注册服务器失败: %s"), *Object->GetStringField(TEXT("msg")));
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE