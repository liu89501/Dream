// ReSharper disable All
#include "PlayerDataInterfaceBase.h"
#include "OnlineIdentityInterface.h"
#include "JsonUtilities.h"
#include "JsonWriter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "NetworkMessage.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TcpSocketBuilder.h"

#define LOCTEXT_NAMESPACE "FPlayerDataInterfaceBase"


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

template <typename ServiceParam>
uint8* MessageUtils::BuildMessageBytes(const FMessage<ServiceParam>& Message)
{
	TArray<uint8> Data;
	FMemoryWriter Writer(Data);
	Writer << Message.Parameter;

	UScriptStruct* ParameterStruct = Message.GetParameterUStruct();
	ParameterStruct->GetCppStructOps()->NetSerialize(Writer, &Message.Parameter);
	
	/*LoginParam->SetStringField(TEXT("ServiceName"), Message.ServerName);
	LoginParam->SetStringField(TEXT("thirdPartyUserTicket"), Ticket);*/
}

FPlayerDataInterfaceBase::~FPlayerDataInterfaceBase()
{
	UE_LOG_ONLINE(Log, TEXT("FPlayerDataInterfaceBase Free..."));
}

void FPlayerDataInterfaceBase::Initialize(FInitializeDelegate Delegate)
{
	GConfig->GetString(TEXT("PDISettings"), TEXT("ServerURL"), ServerURL, GEngineIni);
	GConfig->GetInt(TEXT("PDISettings"), TEXT("ServerPort"), ServerPort, GEngineIni);
	GConfig->GetString(TEXT("PDISettings"), TEXT("ServerHostName"), ServerHostName, GEngineIni);
	GConfig->GetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), OSSName, GEngineIni);
	
	if (IsRunningDedicatedServer())
	{
		FString AESKey;
		GConfig->GetString(TEXT("PDISettings"), TEXT("AESKey"), AESKey, GEngineIni);
		AESUtils = FAESUtils(AESKey);
		
		FParse::Value(FCommandLine::Get(), TEXT("ServerToken="), ServerToken);
		FParse::Value(FCommandLine::Get(), TEXT("ServerId="), ServerID);
	}
	
	Delegate.ExecuteIfBound(true);
}

void FPlayerDataInterfaceBase::GetAvailableDedicatedServer(const FString& ServerId, FGetServerComplete Delegate)
{
	FHttpRequestRef Request = MakeGetRequest(FString::Printf(TEXT("/player/get_server?serverId=%s"), *ServerId));
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataInterfaceBase::OnGetServerCompleteTrigger, Delegate);
	bool bSuccess = Request->ProcessRequest();
	
	if (!bSuccess)
	{
		FFindServerResult DummyResult;
		Delegate.ExecuteIfBound(DummyResult, MSG_ERROR);
	}
}

void FPlayerDataInterfaceBase::RegisterServer(const FDedicatedServerInformation& Information, FRegisterServerComplete Delegate)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/register"));

	bool bSuccess = true;

	TSharedPtr<FJsonObject> ServerInfo = MakeShareable(new FJsonObject);
	ServerInfo->SetNumberField(TEXT("Port"), Information.Port);
	ServerInfo->SetNumberField(TEXT("MaxPlayers"), Information.MaxPlayers);
	ServerInfo->SetStringField(TEXT("MapName"), Information.MapName);
	ServerInfo->SetStringField(TEXT("GameModeName"), Information.GameModeName);
	ServerInfo->SetStringField(TEXT("ServerId"), ServerID);

	FString JsonString;
	TSharedRef<FCondensedJsonStringWriter> Writer = FCondensedJsonStringWriterFactory::Create(&JsonString);
	bSuccess &= FJsonSerializer::Serialize(ServerInfo.ToSharedRef(), Writer);

	if (bSuccess)
	{
		FString EncryptString;
		AESUtils.EncryptData(JsonString, EncryptString);

		Request->SetContentAsString(EncryptString);
		Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataInterfaceBase::OnRegisterCompleteTrigger, Delegate);

		bSuccess &= Request->ProcessRequest();
	}

	if (!bSuccess)
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerDataInterfaceBase::RunNewDedicatedServer(const FRunServerParameter& Parameter, FRunServerComplete Delegate)
{
	bool bResult = true;
	FString RequestParam;
	bResult &= FJsonObjectConverter::UStructToJsonObjectString(Parameter, RequestParam);
	FHttpRequestRef Request = MakePostRequest("/player/run_server");
	Request->SetContentAsString(RequestParam);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataInterfaceBase::OnRunServerCompleteTrigger, Delegate);
	bResult &= Request->ProcessRequest();

	if (!bResult)
	{
		Delegate.ExecuteIfBound(FString(), MSG_ERROR);
	}
}

void FPlayerDataInterfaceBase::UnRegisterServer()
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/unregister"), TEXT("text/plain"));
	Request->SetContentAsString(ServerID);
	Request->ProcessRequest();
}

void FPlayerDataInterfaceBase::UpdateActivePlayers(bool bIncrement)
{
	FHttpRequestRef Request = MakePostRequest(TEXT("/server/updatePlayers"));
	Request->SetContentAsString(FString::Printf(TEXT("{\"serverId\":\"%s\",\"increment\":%s}"), *ServerID, bIncrement ? TEXT("true") : TEXT("false")));
	Request->ProcessRequest();
}

void FPlayerDataInterfaceBase::Login(FCommonCompleteNotify Delegate)
{

	/*ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	check(SocketSubsystem);

	
	FAddressInfoResult Result = SocketSubsystem->GetAddressInfo(*ServerHostName, nullptr, EAddressInfoFlags::Default, NAME_None);
	if (Result.ReturnCode == ESocketErrors::SE_NO_ERROR && Result.Results.Num() > 0)
	{
		Result.Results[0].Address.;
	}

	FResolveInfo* ResolveInfo = SocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*ServerHostName));
	checkf(ResolveInfo->IsComplete(), TEXT("Resolve Server HostName Failure"));

	FIPv4Endpoint Endpoint;
	ResolveInfo->GetResolvedAddress().GetIp(Endpoint.Address.Value);
	Endpoint.Port = ServerPort;

    FSocket* PDISocket = FTcpSocketBuilder(TEXT("PDISocket"))
        .AsBlocking()
        .AsReusable()
		.WithReceiveBufferSize(2 * 1024 * 1024)
        .WithSendBufferSize(1024 * 1024);

	check()

	FSimpleAbstractSocket_FSocket SocketWrapper(PDISocket);*/

	
	FString ErrorMessage;
	if (!Cookie.IsEmpty())
	{
		Delegate.ExecuteIfBound(ErrorMessage);
	}

	bool bSuccessfully = false;
	
	IOnlineIdentityPtr Identity = Online::GetIdentityInterface();
	if (Identity.IsValid())
	{
		FString Ticket = Identity->GetAuthToken(0);

		TSharedPtr<FJsonObject> LoginParam = MakeShareable(new FJsonObject);
		LoginParam->SetStringField(TEXT("platformName"), OSSName);
		LoginParam->SetStringField(TEXT("thirdPartyUserTicket"), Ticket);

		FString OutputString;
		TSharedRef<FCondensedJsonStringWriter> Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
		if (FJsonSerializer::Serialize(LoginParam.ToSharedRef(), Writer))
		{
			FHttpRequestRef Request = MakePostRequest(TEXT("/player/login"));
			Request->SetContentAsString(OutputString);
			Request->OnProcessRequestComplete().BindRaw(this, &FPlayerDataInterfaceBase::OnLoginCompleteTrigger, Delegate);
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

void FPlayerDataInterfaceBase::Logout()
{
	FHttpRequestRef Request = MakeGetRequest(TEXT("/player/logout"));
	Request->ProcessRequest();
}

FString FPlayerDataInterfaceBase::GetServerToken() const
{
	return ServerToken;
}

FPlayerDataDelegate& FPlayerDataInterfaceBase::GetPlayerDataDelegate()
{
	return PlayerDataDelegate;
}

FHttpRequestRef FPlayerDataInterfaceBase::MakeRequest() const
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

FHttpRequestRef FPlayerDataInterfaceBase::MakePostRequest(const FString& URL, FString ContentType) const
{
	FHttpRequestRef Request = MakeRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *ServerURL, *URL));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), ContentType);
	return Request;
}

FHttpRequestRef FPlayerDataInterfaceBase::MakeGetRequest(const FString& URL) const
{
	FHttpRequestRef Request = MakeRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *ServerURL, *URL));
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Cookie"), Cookie);
	return Request;
}

void FPlayerDataInterfaceBase::OnLoginCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate)
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

void FPlayerDataInterfaceBase::OnGetServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetServerComplete Delegate)
{
	FFindServerResult Result;
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
				FJsonObjectConverter::JsonObjectToUStruct(Data.ToSharedRef(), &Result);
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
	
	Delegate.ExecuteIfBound(Result, ErrorMessage);
}

void FPlayerDataInterfaceBase::OnRegisterCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FRegisterServerComplete Delegate)
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

void FPlayerDataInterfaceBase::OnRunServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FRunServerComplete Delegate)
{
	FString RunningServerID;
	FString ErrorMessage;
	
	if (bConnectedSuccessfully)
	{
		TSharedPtr<FJsonObject> Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, Object))
		{
			if (Object->GetBoolField(TEXT("status")))
			{
				RunningServerID = Object->GetStringField(TEXT("data"));
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
	
	Delegate.ExecuteIfBound(RunningServerID, ErrorMessage);
}

#undef LOCTEXT_NAMESPACE