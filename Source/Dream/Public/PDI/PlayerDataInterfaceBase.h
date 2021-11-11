#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "PlayerDataInterface.h"
#include "TCPSocketReceiver.h"
#include "TCPSocketSender.h"

struct FAESUtils
{
	
public:

	FAESUtils() = default;

	FAESUtils(const FString& InSecretKey);

	void EncryptData(const FString& Content, FString& Encrypt);

	FString DecryptData(const FString& EncryptContent);

private:

	TArray<uint8> SecretKey;
};



class FPacketArchiveReader : public FMemoryReader
{

public:

	explicit FPacketArchiveReader(FReceivePacket InPacket)
		: FMemoryReader(*InPacket), Packet(InPacket)
	{
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override;
	virtual FArchive& operator<<(UObject*& Value) override;

	// 在这里放一个成员是为了 防止数据被GC
	FReceivePacket Packet;
};

class FPacketArchiveWriter : public FMemoryWriter
{
	
public:
	
	explicit FPacketArchiveWriter(TArray<uint8>& InBytes) : FMemoryWriter(InBytes)
	{
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override;
	virtual FArchive& operator<<(UObject*& Value) override;
};



DECLARE_DELEGATE_OneParam(FOnReceiveMessage, FPacketArchiveReader&);

#define CALLBACK_BINDING_RAW(Mark, Target, FUnc) BindReceiveFunction(Mark, FOnReceiveMessage::CreateRaw(Target, FUnc))
#define CALLBACK_BINDING_OBJECT(Mark, Target, FUnc) BindReceiveFunction(Mark, FOnReceiveMessage::CreateUObject(Target, FUnc))



template<int32 Mark>
struct TServiceBase
{
	static constexpr int32 MarkId = Mark;
};

template<int32 Mark> struct TService : TServiceBase<Mark> { };

#define DEFINED_SERVICE_INFO(Mark, PType, Alias) \
	template<> struct TService<Mark> : TServiceBase<Mark> { typedef PType Type; }; \
	typedef TService<Mark> Alias

DEFINED_SERVICE_INFO(1, FString, TService_ServerLogin);
DEFINED_SERVICE_INFO(2, FLoginParameter, TService_ClientLogin);
DEFINED_SERVICE_INFO(3, FRunServerParameter, TService_RunDedicatedServer);
DEFINED_SERVICE_INFO(4, FDedicatedServerInformation, TService_RegisterServer);
DEFINED_SERVICE_INFO(5, FUpdateServerPlayerParam, TService_UpdateServer);
DEFINED_SERVICE_INFO(6, FLaunchNotifyParam, TService_NotifyServer);

template<typename S>
TSharedRef<TArray<uint8>, ESPMode::ThreadSafe> PDIBuildParam(const typename S::Type& Param)
{
	TArray<uint8> Data;
	FPacketArchiveWriter Writer(Data);

	int32 MarkValue = S::MarkId;
	Writer << MarkValue;
	Writer << const_cast<typename S::Type&>(Param);
	
	TSharedRef<TArray<uint8>, ESPMode::ThreadSafe> Packet = MakeShared<TArray<uint8>, ESPMode::ThreadSafe>();
	
	int32 DataLength = Data.Num();
	Packet->Append(reinterpret_cast<uint8*>(&DataLength), sizeof(int32));
	Packet->Append(Data);
	return Packet;
}



/**
 * 实现了基本功能
 */
class FPlayerDataInterfaceBase : public FPlayerDataInterface//, public FTickerObjectBase
{
	
public:

	FPlayerDataInterfaceBase();
	
	virtual ~FPlayerDataInterfaceBase();
	
	virtual void Initialize() override;

	virtual void Login() override;
	virtual void Logout() override;
	
	// local
	virtual void RunNewDedicatedServer(const FRunServerParameter& Parameter) override;

	// server
	virtual void RegisterServer(const FDedicatedServerInformation& Information) override;
	virtual void UpdateActivePlayers(const FUpdateServerPlayerParam& Param) override;
	virtual void NotifyBackendServer(const FLaunchNotifyParam& Param) override;
	
	virtual FPlayerDataDelegate& GetPlayerDataDelegate() override;
	virtual FOnServerConnectionLose& OnServerConnectionLoseDelegate() override;

public:

	void OnReceiveLoginMessage(FPacketArchiveReader& Data);
	void OnReceiveGetServerMessage(FPacketArchiveReader& Data);
	void OnReceiveRegisterServerMessage(FPacketArchiveReader& Data);
	void OnReceiveNotifyBSMessage(FPacketArchiveReader& Data);

	FORCEINLINE FTCPSocketSender* GetSender() const
	{
		return SocketSender;
	}

protected:

	void OnSocketDisconnect();
	void OnReceiveData(FReceivePacket Packet);
	void BindReceiveFunction(int32 Mark, FOnReceiveMessage Delegate);

	FPlayerDataDelegate PlayerDataDelegate;
	FOnServerConnectionLose OnServerConnectionLose;

	FTCPSocketSender* SocketSender;
	FTCPSocketReceiver* SocketReceiver;

private:

	bool ConnectToServer();
	void ShutdownSocket();

	FSocket* Socket;

	/* Server Host Name */
	FString ServerHostName;
	
	/* Server Listen Port */
	int32 ServerPort;

	/** 第三方服务子系统名称 */
	FString OSSName;
	
	/* Encrypt Tool */
	FAESUtils AESUtils;

	TMap<int32, FOnReceiveMessage> CallbackDelegates;
	
};
