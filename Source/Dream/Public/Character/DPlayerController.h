// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "ShootWeapon.h"
#include "GameFramework/PlayerController.h"
#include "DPlayerController.generated.h"

UENUM(BlueprintType)
namespace ETalkType
{
	enum Type
	{
		Current,
        World,
        Team,
        Private,
        NONE
    };
}

USTRUCT(BlueprintType)
struct FTalkMessage
{
	GENERATED_USTRUCT_BODY()

public:

	FTalkMessage() = default;

	FTalkMessage(FString InPlayerName, FString InContent, TEnumAsByte<ETalkType::Type> InTalkType)
        : PlayerName(InPlayerName), Content(InContent), TalkType(InTalkType)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETalkType::Type> TalkType;
};

/**
 *
 */
UCLASS()
class DREAM_API ADPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	int32 LevelOneAmmunition;
	int32 LevelTwoAmmunition;
	int32 LevelThreeAmmunition;

public:

	ADPlayerController();

	/*
		发送聊天消息到所有连接到服务器的玩家
		@param Message TalkMessage
	*/
	UFUNCTION(BlueprintCallable, Category = DController)
	void SendTalkMessage(const FTalkMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnReceiveRewardMessage", DisplayName = "OnReceiveRewardMessage"))
	void BP_ReceiveRewardMessage(const FPropsInfo& PropsInfo, ERewardNotifyMode NotifyMode, int32 RewardNum);

	/*
		接收服务器的 聊天消息
		@param Message 聊天消息
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnReceiveTalkMessage", DisplayName = "OnReceiveTalkMessage"))
	void BP_OnReceiveTalkMessage(const FTalkMessage& Message);

	/* 任务状态更新 */
	UFUNCTION(BlueprintImplementableEvent, Category = DController, meta = (ScriptName = "OnTrackingTaskUpdated", DisplayName = "OnTrackingTaskUpdated"))
	void BP_OnTrackingTaskUpdated(const TArray<FTaskInProgressInfo>& UpdatedTasks);

	UFUNCTION(BlueprintCallable, Category = DController)
	void ExitTeam();

	UFUNCTION(BlueprintCallable, Category = DController)
    void PopupRewards(const FItemListHandle& ItemList);

	UFUNCTION(BlueprintCallable, Category = DController)
	void AddGuideActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = DController)
	void RemoveGuideActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = DController)
	void BroadcastGameModeClientDelegate(FGameplayTag DelegateTag);

#if WITH_EDITOR

	/* TEST ---------------------------------- */
	UFUNCTION(Exec)
    void TestLoginServer();

	UFUNCTION(Exec)
    void TestCreateSession();

	UFUNCTION(Exec)
    void TestFindSession();
	
	UFUNCTION(Exec)
    void TestSocket() const;
	/* TEST ---------------------------------- */
	
#endif

	UFUNCTION(Client, Reliable)
	void ClientChangeConnectedServer(uint32 Address, uint32 Port);

public:

	virtual void ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason) override;

	bool AddWeaponAmmunition(EAmmoType AmmoType, int32 AmmunitionAmount);

	int32& GetWeaponAmmunition(EAmmoType AmmoType);
	int32 GetWeaponAmmunition(EAmmoType AmmoType) const;

	void SetWeaponAmmunition(EAmmoType AmmoType, int32 NewValue);
	void DecrementAmmunition(EAmmoType AmmoType, int32 Amount);

	int32 GetDefaultAmmunition(EAmmoType AmmoType) const;

protected:

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest) override;

	UFUNCTION(NetMulticast, UnReliable)
	void MulticastMessage(const FTalkMessage& Message);
	UFUNCTION(Server, UnReliable)
	void ServerSendMessage(const FTalkMessage& Message);

	virtual void ClientWasKicked_Implementation(const FText& KickReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerBroadcastGMClientDelegate(const FGameplayTag& DelegateTag);

private:

	void OnUpdatedTasks(const TArray<FTaskInProgressInfo>& UpdatedTasks);

	void OnReceiveRewardMessage(const FItemListHandle& ItemList);

	TArray<struct FGuideElement> GetGuideElements() const;
	
private:

	FDelegateHandle Handle_UpdateTask;
	FDelegateHandle Handle_ReceiveRewards;

	// 引导点
	UPROPERTY()
	TArray<AActor*> GuideActors;
};


