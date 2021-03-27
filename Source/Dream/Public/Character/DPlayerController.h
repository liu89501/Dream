// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sockets.h"
#include "DreamType.h"
#include "SharedPointer.h"
#include "IPv4Endpoint.h"
#include "PlayerDataStore.h"
#include "GameFramework/PlayerController.h"
#include "DPlayerController.generated.h"

/**
 *
 */
UCLASS()
class DREAM_API ADPlayerController : public APlayerController
{
	GENERATED_BODY()

	DECLARE_DELEGATE_TwoParams(FConfirmRebornCharacter, ADPlayerController* /* Controller */, const FTransform& /* SpawnTransform */)

public:

	int32 LevelOneAmmunition;
	int32 LevelTwoAmmunition;
	int32 LevelThreeAmmunition;

public:

	ADPlayerController();

	/* 对目标玩家发出组队申请 */
	UFUNCTION(BlueprintCallable, Category = DController)
	void SendTeamUpApply(class APlayerState* TargetPlayerState);

	/*
		发送击杀消息到所有连接到服务器的玩家
		@param Message KillMessage
	*/
	UFUNCTION(BlueprintCallable, Category = DController)
	void SendKillMessage(const FKillMessage& Message);

	/*
		发送聊天消息到所有连接到服务器的玩家
		@param Message TalkMessage
	*/
	UFUNCTION(BlueprintCallable, Category = DController)
	void SendTalkMessage(const FTalkMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnReceiveRewardMessage", DisplayName = "OnReceiveRewardMessage"))
	void BP_ReceiveRewardMessage(const FPropsInfo& PropsInfo);

	/*
		接收服务器的 击杀消息
		@param Message 击杀消息结构体
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnReceiveKillMessage", DisplayName = "OnReceiveKillMessage"))
	void BP_OnReceiveKillMessage(const FKillMessage& Message);
	/*
		接收服务器的 聊天消息
		@param Message 聊天消息
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnReceiveTalkMessage", DisplayName = "OnReceiveTalkMessage"))
	void BP_OnReceiveTalkMessage(const FTalkMessage& Message);

	/* 接收组队申请消息 */
	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnReceiveTeamApplyMessage", DisplayName = "OnReceiveTeamApplyMessage"))
	void BP_OnReceiveTeamApplyMessage(const FPlayerAccountInfo& PlayerInfo);

	UFUNCTION(BlueprintCallable, Category = DController)
	void ReturnToMainMenuWithTextReason(const FText& ErrorMessage);

	UFUNCTION(BlueprintCallable, Category = DController)
	void ExitGame();

	UFUNCTION(BlueprintCallable, Category = DController)
	void BeginGame();

	UFUNCTION(BlueprintCallable, Category = DController)
	void Travel(const FString& Address);

	UFUNCTION(BlueprintCallable, Category = DController)
	void AgreeTeamUpApply(const FPlayerAccountInfo& PlayerInfo);

	UFUNCTION(BlueprintCallable, Category = DController)
	void ExitTeam();

	UFUNCTION(BlueprintCallable, Category = DController, BlueprintAuthorityOnly)
    void ProcessRebornCharacter(const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = DController)
	void PersistenceWeapons(const TArray<TSubclassOf<class AShootWeapon>>& Weapons);

	/**
	 * 增加或减少玩家身上的钱
	 */
	UFUNCTION(BlueprintCallable, Category = DController)
	int64 IncreaseOrDecreaseMoney(bool bIncrease, int64 Amount);

	UFUNCTION(BlueprintCallable, Category = DController)
	const FPlayerInfo& GetOnlinePlayerInfo() const;

	
	/* TEST ---------------------------------- */
	UFUNCTION(Exec)
    void TestLoginServer();

	UFUNCTION(Exec)
    void TestCreateSession();

	UFUNCTION(Exec)
    void TestFindSession();
	/* TEST ---------------------------------- */


public:

	UFUNCTION(Client, Reliable)
	void ClientReceiveRewardMessage(UClass* PropsClass);

	bool AddWeaponAmmunition(EAmmoType AmmoType, int32 AmmunitionAmount);
	UFUNCTION(Server, Reliable)
	void ServerAddWeaponAmmunition(EAmmoType AmmoType, int32 AmmunitionAmount);

	int32& GetWeaponAmmunition(EAmmoType AmmoType);
	int32 GetWeaponAmmunition(EAmmoType AmmoType) const;

	void SetWeaponAmmunition(EAmmoType AmmoType, int32 NewValue);
	void DecrementAmmunition(EAmmoType AmmoType, int32 Amount);

	int32 GetDefaultAmmunition(EAmmoType AmmoType) const;

	FConfirmRebornCharacter& GetConfirmRebornDelegate()
	{
		return OnConfirmReborn;
	}

protected:

	UFUNCTION(Server, Reliable)
	void ServerPersistenceWeapons(const TArray<TSubclassOf<class AShootWeapon>>& Weapons);

	virtual void HandleTeamApply();

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest) override;

	/*
		使用在线子系统 给当前服务器所有用户组播信息
		@param Type 这个消息的类型
		@param Message 要发送的消息
	*/
	void SendMulticastMessage(TEnumAsByte<EMessageType::Type> Type, const FString& Message);
	void SendMessageToPlayer(const FUniqueNetIdRepl& UserNetId, TEnumAsByte<EMessageType::Type> Type, const FString& Message);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastMessage(EMessageType::Type Type, const FString& Message);
	UFUNCTION(Server, Reliable)
	void ServerMulticastMessage(EMessageType::Type Type, const FString& Message);
	UFUNCTION(Client, Reliable)
	void ClientReceiveMessage(EMessageType::Type Type, const FString& Message);

	/*
		处理接受到的服务器数据
	*/
	virtual void HandleRawMessage(EMessageType::Type Type, const FString& Message);

	void OnGetPlayerInfoComplete(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void RecvData(const TSharedPtr<class FArrayReader, ESPMode::ThreadSafe>& RawData, const FIPv4Endpoint& Endpoint);

	class FUdpSocketReceiver* UdpReceiver;
	class FSocket* ClientSocket;

	FConfirmRebornCharacter OnConfirmReborn;

	UPROPERTY()
	FPlayerInfo OnlinePlayerInfo;
};
