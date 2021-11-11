// ReSharper disable All
#include "PlayerDataInterfaceBase.h"
#include "OnlineIdentityInterface.h"
#include "JsonUtilities.h"
#include "JsonWriter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TcpSocketBuilder.h"
#include "TCPSocketSender.h"

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
	DecodeContent.Add(0);

	FString DecryptText;
	TArray<TCHAR>& Chars = DecryptText.GetCharArray();
	FUTF8ToTCHAR TChar(reinterpret_cast<ANSICHAR*>(DecodeContent.GetData()));
	Chars.Append(TChar.Get(), TChar.Length());
	return DecryptText;
}

FArchive& FPacketArchiveReader::operator<<(FSoftObjectPath& Value)
{
	FString PathString;
	*this << PathString;
	Value.SetPath(PathString);
	return *this;
}

FArchive& FPacketArchiveReader::operator<<(UObject*& Value)
{
	FString PathName;
	*this << PathName;
	if (!PathName.IsEmpty())
	{
		Value = LoadObject<UObject>(nullptr, *PathName);
	}
	return *this;
}

FArchive& FPacketArchiveWriter::operator<<(FSoftObjectPath& Value)
{
	FString PathString = Value.ToString();
	*this << PathString;
	return *this;
}

FArchive& FPacketArchiveWriter::operator<<(UObject*& Value)
{
	FString PathName = Value->GetPathName();
	*this << PathName;
	return *this;
}

FPlayerDataInterfaceBase::FPlayerDataInterfaceBase()
	: SocketSender(nullptr),
	  SocketReceiver(nullptr),
	  Socket(nullptr)
{
	CALLBACK_BINDING_RAW(TService_ClientLogin::MarkId, this, &FPlayerDataInterfaceBase::OnReceiveLoginMessage);
	CALLBACK_BINDING_RAW(TService_RunDedicatedServer::MarkId, this, &FPlayerDataInterfaceBase::OnReceiveGetServerMessage);
	CALLBACK_BINDING_RAW(TService_RegisterServer::MarkId, this, &FPlayerDataInterfaceBase::OnReceiveRegisterServerMessage);
	CALLBACK_BINDING_RAW(TService_NotifyServer::MarkId, this, &FPlayerDataInterfaceBase::OnReceiveNotifyBSMessage);
}

FPlayerDataInterfaceBase::~FPlayerDataInterfaceBase()
{
	UE_LOG_ONLINE(Log, TEXT("FPlayerDataInterfaceBase Free..."));

	ShutdownSocket();
}

void FPlayerDataInterfaceBase::Initialize()
{
	GConfig->GetInt(TEXT("PDISettings"), TEXT("ServerPort"), ServerPort, GEngineIni);
	GConfig->GetString(TEXT("PDISettings"), TEXT("ServerHostName"), ServerHostName, GEngineIni);
	GConfig->GetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), OSSName, GEngineIni);
}

void FPlayerDataInterfaceBase::RegisterServer(const FDedicatedServerInformation& Information)
{
	SocketSender->Send(PDIBuildParam<TService_RegisterServer>(Information));
}

void FPlayerDataInterfaceBase::RunNewDedicatedServer(const FRunServerParameter& Parameter)
{
	SocketSender->Send(PDIBuildParam<TService_RunDedicatedServer>(Parameter));
}

void FPlayerDataInterfaceBase::UpdateActivePlayers(const FUpdateServerPlayerParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_UpdateServer>(Param));
}

void FPlayerDataInterfaceBase::NotifyBackendServer(const FLaunchNotifyParam& Param)
{
	SocketSender->Send(PDIBuildParam<TService_NotifyServer>(Param));
}

void FPlayerDataInterfaceBase::Login()
{
	if (!ConnectToServer())
	{
		BroadcastOnLogin(false);
		ShutdownSocket();
		return;
	}

	if (IsRunningDedicatedServer())
	{
		FString Token;
		FParse::Value(FCommandLine::Get(), TEXT("ServerToken="), Token);
		SocketSender->Send(PDIBuildParam<TService_ServerLogin>(Token));
	}
	else
	{
		FLoginParameter Parameter;
		
#if WITH_EDITOR

		Parameter.PlatformName = TEXT("Steam");
		Parameter.ThirdPartyUserTicket = TEXT("123456");

#else

		IOnlineIdentityPtr OnlineIdentity = Online::GetIdentityInterface();
		Parameter.PlatformName = OSSName;
		Parameter.ThirdPartyUserTicket = OnlineIdentity->GetAuthToken(0);

#endif
		
		SocketSender->Send(PDIBuildParam<TService_ClientLogin>(Parameter));
	}
}

void FPlayerDataInterfaceBase::Logout()
{
	ShutdownSocket();
}

FPlayerDataDelegate& FPlayerDataInterfaceBase::GetPlayerDataDelegate()
{
	return PlayerDataDelegate;
}

FOnServerConnectionLose& FPlayerDataInterfaceBase::OnServerConnectionLoseDelegate()
{
	return OnServerConnectionLose;
}

void FPlayerDataInterfaceBase::OnReceiveLoginMessage(FPacketArchiveReader& Data)
{
	bool bLoginSucess;
	Data << bLoginSucess;

	BroadcastOnLogin(bLoginSucess);
}

void FPlayerDataInterfaceBase::OnReceiveGetServerMessage(FPacketArchiveReader& Data)
{
	bool bSucess;
	FFindServerResult Result;

	Data << bSucess;
	if (bSucess)
	{
		Data << Result;
	}
	
	BroadcastOnGetServer(Result, bSucess);
}

void FPlayerDataInterfaceBase::OnReceiveRegisterServerMessage(FPacketArchiveReader& Data)
{
	bool bSucess;
	FString ServerId;
	
	Data << bSucess;

	if (bSucess)
	{
		Data << ServerId;
	}
	
	BroadcastOnRegisterServer(ServerId, bSucess);
}

void FPlayerDataInterfaceBase::OnReceiveNotifyBSMessage(FPacketArchiveReader& Data)
{
	bool bSucess;
	FString ServerAddress;

	Data << bSucess;

	if (bSucess)
	{
		Data << ServerAddress;
	}
	BroadcastOnLaunchNotify(ServerAddress, bSucess);
}

void FPlayerDataInterfaceBase::OnSocketDisconnect()
{
	OnServerConnectionLose.ExecuteIfBound();

	// todo 重新连接功能
}

void FPlayerDataInterfaceBase::OnReceiveData(FReceivePacket Data)
{
	TSharedRef<FPacketArchiveReader> ArchiveReader = MakeShared<FPacketArchiveReader>(Data);
	
	int32 Mark;
	*ArchiveReader << Mark;

	if (FOnReceiveMessage* Delegate = CallbackDelegates.Find(Mark))
	{
		AsyncTask(ENamedThreads::GameThread, [Delegate, ArchiveReader]
	    {
	        Delegate->Execute(*ArchiveReader);
	    });
	}
}

void FPlayerDataInterfaceBase::BindReceiveFunction(int32 Mark, FOnReceiveMessage Delegate)
{
	CallbackDelegates.Add(Mark, Delegate);
}

bool FPlayerDataInterfaceBase::ConnectToServer()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	TSharedPtr<FInternetAddr> Addr;
	
	FAddressInfoResult Result = SocketSubsystem->GetAddressInfo(*ServerHostName, nullptr, EAddressInfoFlags::Default, NAME_None);
	if (Result.ReturnCode == ESocketErrors::SE_NO_ERROR && Result.Results.Num() > 0)
	{
		Addr = Result.Results[0].Address;
	}

	if (!Addr.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("查找域名对应IP地址失败: %s"), SocketSubsystem->GetSocketError(Result.ReturnCode));
		return false;
	}

	Addr->SetPort(ServerPort);
	UE_LOG(LogOnline, Verbose, TEXT("Address: %s"), *Addr->ToString(true));

	Socket = FTcpSocketBuilder(TEXT("PDISocket"))
            .WithReceiveBufferSize(2 * 1024 * 1024)
            .WithSendBufferSize(1024 * 1024)
            .AsBlocking()
            .AsReusable();

	if (!Socket->Connect(*Addr))
	{
		ESocketErrors LastErrorCode = SocketSubsystem->GetLastErrorCode();
		UE_LOG(LogOnline, Error, TEXT("连接到服务端失败: %s"), SocketSubsystem->GetSocketError(LastErrorCode));
		return false;
	}

	SocketReceiver = new FTCPSocketReceiver(Socket);
	SocketReceiver->OnReceiveDelegate().BindRaw(this, &FPlayerDataInterfaceBase::OnReceiveData);
	SocketReceiver->OnShutdownDelegate().BindRaw(this, &FPlayerDataInterfaceBase::OnSocketDisconnect);
	SocketReceiver->Start();
	
	SocketSender = new FTCPSocketSender(Socket);

	return true;
}

void FPlayerDataInterfaceBase::ShutdownSocket()
{
	if (SocketSender != nullptr)
	{
		SocketSender->Stop();
		delete SocketSender;
	}

	if (Socket != nullptr)
	{
		Socket->Close();
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
	}

	// 套接字关了之后这边接收线程才能停， 不然会死锁卡住
	if (SocketReceiver != nullptr)
	{
		SocketReceiver->Stop();
		delete SocketReceiver;
	}
}

#undef LOCTEXT_NAMESPACE
