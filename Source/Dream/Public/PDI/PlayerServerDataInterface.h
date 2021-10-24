#pragma once

#include "CoreMinimal.h"
#include "IHttpRequest.h"
#include "OnlineSubsystem.h"
#include "PlayerDataInterface.h"

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

class FPlayerServerDataInterface : public FPlayerDataInterface
{

public:

	FPlayerServerDataInterface() = default;
	virtual ~FPlayerServerDataInterface();

	virtual void Initialize(FInitializeDelegate Delegate) override;
	
	virtual void AddPlayerRewards(UItemData* Rewards, FCommonCompleteNotify Delegate) override;
	
	virtual void EquipWeapon(int64 WeaponId, int32 EquipmentIndex, FCommonCompleteNotify Delegate) override;

	virtual void EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate) override;

	virtual void LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate) override;

	virtual void LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate) override;

	virtual void GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate) override;

	virtual void RunServer(const FRunServerParameter& Parameter, FGetServerComplete Delegate) override;

	virtual void PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate) override;

	virtual void GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate) override;
	virtual void GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate) override;

	virtual void GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate) override;
	virtual void GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate) override;

	virtual void GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate) override;

	virtual void DeliverTask(const int64& TaskId, FTaskRewardDelegate Delegate) override;
	virtual void AcceptTask(const int64& TaskId, FCommonCompleteNotify Delegate) override;
	virtual void UpdateTaskState(const FQuestActionHandle& Handle) override;
	
	virtual void RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName, FRegisterServerComplete Delegate) override;
	virtual void UnRegisterServer() override;
	virtual void UpdateActivePlayers(bool bIncrement) override;

	virtual void Login(FCommonCompleteNotify Delegate) override;
	
	virtual void Logout() override;

	virtual FString GetServerToken() const override;

	virtual const FPlayerProperties& GetCachedProperties() const override;
	
	virtual void RefreshPlayerProperties() override;

protected:

	void OnLoginCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate);
	void OnGetWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetWeaponComplete Delegate);
	void OnGetPlayerInfoCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetPlayerInfoComplete Delegate);
	void OnAddItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnRunServerCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetServerComplete Delegate);
	void OnSwitchWeaponCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate);
	void OnRegisterCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FRegisterServerComplete Delegate);
	void OnPayItemCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FCommonCompleteNotify Delegate);
	void OnGetStoreItemsCompleteTrigger(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGetStoreItemsComplete Delegate);

	FHttpRequestRef MakePostRequest(const FString& URL, FString ContentType = TEXT("application/json")) const;
	FHttpRequestRef MakeGetRequest(const FString& URL) const;

private:

	FHttpRequestRef MakeRequest() const;

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

	FPlayerProperties CachedProperties;
};

