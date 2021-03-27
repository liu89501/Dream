#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "IHttpRequest.h"
#include "PlayerDataStoreType.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FLoginComplete, const FString& /* ErrorMessage */);
DECLARE_MULTICAST_DELEGATE_OneParam(FSwitchWeaponComplete, const FString& /* ErrorMessage */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGetWeaponComplete, const TArray<FPlayerWeapon>&, const FString& /* ErrorMessage */);
DECLARE_MULTICAST_DELEGATE_OneParam(FAddWeaponComplete, bool /*bSuccessfully*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGetPlayerInfoComplete, const FPlayerInfo&, const FString& /* ErrorMessage */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGetServerComplete, const FString& /*ServerAddress*/, const FString& /* ErrorMessage */);
DECLARE_MULTICAST_DELEGATE_OneParam(FRegisterServerComplete, const FString& /*ServerId*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FPayItemComplete, const FString& /* ErrorMsg */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGetStoreItemsComplete, const TArray<FStoreItem>&, const FString& /*ErrorMessage*/);

class PLAYERDATASTORE_API FPlayerDataStore
{

public:

	FPlayerDataStore(IOnlineSubsystem* OSSystem);
	~FPlayerDataStore();

	bool AddWeapons(const TArray<FPlayerWeaponAdd>& NewWeapons);

	bool EquipmentWeapon(int32 WeaponId, int32 EquipmentIndex);

	bool GetStoreItems(int32 StoreId);

	bool RunServer(const FRunServerParameter& Parameter);

	bool PayItem(int32 ItemId);

	bool GetPlayerWeapons(EGetWeaponCondition Condition = EGetWeaponCondition::All);
	bool GetPlayerInfo();

	bool RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName);
	void UnRegisterServer();

	void UpdateActivePlayers(bool bIncrement);

	bool Login();
	void Logout();

	FORCEINLINE const FString& GetServerToken() const
	{
		return ServerToken;
	}

	IOnlineSubsystem* GetOSS() const;

public:

	FLoginComplete OnLoginComplete;
	FSwitchWeaponComplete OnSwitchWeaponComplete;
	FGetWeaponComplete OnGetWeaponComplete;
	FAddWeaponComplete OnAddWeaponComplete;
	FGetServerComplete OnRunServerComplete;
	FGetServerComplete OnGetServerComplete;
	FRegisterServerComplete OnRegisterComplete;
	FGetPlayerInfoComplete OnGetPlayerInfoComplete;
	FPayItemComplete OnPayItemComplete;
	FGetStoreItemsComplete OnGetStoreItemComplete;

protected:

	void OnLoginCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnGetWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnGetPlayerInfoCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnAddWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnAddItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnRunServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnSwitchWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnRegisterCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnPayItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnGetStoreItemsCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);


	FHttpRequestRef MakePostRequest(const FString& URL, FString ContentType = TEXT("application/json"));
	FHttpRequestRef MakeGetRequest(const FString& URL);

private:

	FHttpRequestRef MakeRequest();

	/* OnlineSubsystem */
	IOnlineSubsystem* OSS;

	/* Server Http Address */
	FString ServerURL;

	/* Server Authorization Token */
	FString ServerToken;

	/* Register ServerID */
	FString ServerID;

	/* Client Login Cookie */
	FString Cookie;
	
	/* Encrypt Tool */
	FAESUtils AESUtils;
};

