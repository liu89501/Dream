#pragma once

#include "CoreMinimal.h"
#include "IHttpRequest.h"
#include "PlayerDataInterface.h"

typedef TJsonWriterFactory< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriterFactory;
typedef TJsonWriter< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriter;

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

class FPlayerDataInterfaceBase : public FPlayerDataInterface
{
	
public:
	
	virtual ~FPlayerDataInterfaceBase();
	
	virtual void Initialize(FInitializeDelegate Delegate) override;

	// local
	virtual void GetAvailableDedicatedServer(const FString& ServerId, FGetServerComplete Delegate) override;
	// local
	virtual void RunNewDedicatedServer(const FRunServerParameter& Parameter, FRunServerComplete Delegate) override;
	
	virtual void RegisterServer(const FDedicatedServerInformation& Information, FRegisterServerComplete Delegate) override;
	virtual void UnRegisterServer() override;
	virtual void UpdateActivePlayers(bool bIncrement) override;
	virtual void Login(FCommonCompleteNotify Delegate) override;
	virtual void Logout() override;
	virtual FString GetServerToken() const override;
	virtual FPlayerDataDelegate& GetPlayerDataDelegate() override;

protected:

	void OnLoginCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate);
	void OnGetServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetServerComplete Delegate);
	void OnRegisterCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FRegisterServerComplete Delegate);
	void OnRunServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FRunServerComplete Delegate);
	
	FHttpRequestRef MakePostRequest(const FString& URL, FString ContentType = TEXT("application/json")) const;
	FHttpRequestRef MakeGetRequest(const FString& URL) const;

	
protected:

	FPlayerDataDelegate PlayerDataDelegate;

private:

	FHttpRequestRef MakeRequest() const;

	/* Server Http Address */
	FString ServerURL;

	/* Server Authorization Token */
	FString ServerToken;

	/* Register ServerID */
	FString ServerID;

	/* Client Login Cookie */
	FString Cookie;

	/** 第三方服务子系统名称 */
	FString OSSName;
	
	/* Encrypt Tool */
	FAESUtils AESUtils;
};

