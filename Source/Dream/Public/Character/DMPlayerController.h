// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PlayerDataInterfaceType.h"
#include "GameFramework/PlayerController.h"
#include "DMPlayerController.generated.h"

enum class EAmmoType : uint8;
class UCharacterMesh;

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

USTRUCT()
struct FDropReward
{
	GENERATED_BODY()

public:

	UPROPERTY()
	int32 RewardItemGuid;

	UPROPERTY()
	FGuid RewardUniqueId;
};

USTRUCT()
struct FTemporaryGearInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 EquippedIdx;
	
	UPROPERTY()
	FEquipmentAttributes Attributes;
	
	UPROPERTY()
	UClass* GearClass;
};

USTRUCT()
struct FTemporaryPlayerInfo
{
	GENERATED_BODY()

public:

	FTemporaryPlayerInfo()
		: Skin(nullptr)
		, LearnedTalents(0)
		, Level(0)
	{
	}

	void Initialize(const FPlayerInfo& PlayerInfo);

	UPROPERTY()
	TArray<FTemporaryGearInfo> Weapons;
	
	UPROPERTY()
	TArray<FTemporaryGearInfo> Modules;

	UPROPERTY()
	UCharacterMesh* Skin;

	UPROPERTY()
	int64 LearnedTalents;

	UPROPERTY()
	int32 Level;
};


DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerAmmunitionChange, float/* New Ammunition */)
DECLARE_DELEGATE_TwoParams(FPlayerInfoSignature, bool bResult, const FTemporaryPlayerInfo&)

/**
 *
 */
UCLASS()
class DREAM_API ADMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** 弹药量 按百分比来存的 */
	float Ammunition_L1;
	float Ammunition_L2;
	float Ammunition_L3;

public:

	ADMPlayerController();

	/*
		发送聊天消息到所有连接到服务器的玩家
		@param Message TalkMessage
	*/
	UFUNCTION(BlueprintCallable, Category = DMController)
	void SendTalkMessage(const FTalkMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = DMController)
	void OnPopupRewards(const FPropsInfo& PropsInfo, ERewardNotifyMode NotifyMode, int32 RewardNum);

	/*
		接收服务器的 聊天消息
		@param Message 聊天消息
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = DMController)
	void OnReceiveTalk(const FTalkMessage& Message);

	/* 任务状态更新 */
	UFUNCTION(BlueprintImplementableEvent, Category = DMController, meta = (ScriptName = "OnTrackingTaskUpdated", DisplayName = "OnTrackingTaskUpdated"))
	void BP_OnTrackingTaskUpdated(const TArray<FTaskInProgressInfo>& UpdatedTasks);

	UFUNCTION(BlueprintCallable, Category = DMController)
	void ExitTeam();

	UFUNCTION(BlueprintCallable, Category = DMController)
    void PopupRewards(const FItemListHandle& ItemList);

	UFUNCTION(BlueprintCallable, Category = DMController)
	void AddGuideActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = DMController)
	void RemoveGuideActor(AActor* Actor);

	/**
	 * Call Server中的Actor ， 用于没有拥有者的actor(无法RPC服务器)
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin="TargetActor", DefaultToSelf="TargetActor"), Category = DMController)
	void CallServerActor(FGameplayTag CallTag, AActor* TargetActor);


public:

	// [Server]
	void HandlePlayerCharacterDie();

	// [Server]
	void CancelRespawnCharacter();

	UFUNCTION(Client, Reliable)
    void ClientChangeConnectedServer(uint32 Address, uint32 Port);

	virtual void ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason) override;

	bool AddWeaponAmmunition(EAmmoType AmmoType, float RecoveryAmount);

	float& GetWeaponAmmunition(EAmmoType AmmoType);

	void SetWeaponAmmunition(EAmmoType AmmoType, float NewAmmunition);

	FOnPlayerAmmunitionChange& GetAmmunitionChangeDelegate()
	{
		return OnAmmunitionChange;
	}

	void GetPlayerInformation(FPlayerInfoSignature Delegate);

protected:

	virtual void BeginPlay() override;

	virtual void BeginInactiveState() override;

	virtual void OnRespawnPlayer();

	virtual void FailedToSpawnPawn() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest) override;

	UFUNCTION(NetMulticast, UnReliable)
	void MulticastMessage(const FTalkMessage& Message);
	UFUNCTION(Server, UnReliable)
	void ServerSendMessage(const FTalkMessage& Message);

	virtual void ClientWasKicked_Implementation(const FText& KickReason) override;

	UFUNCTION(Server, Reliable)
	void ServerCallServerActor(const FGameplayTag& DelegateTag, AActor* TargetActor);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void OnGetPlayerInfo(const FPlayerInfo& PlayerInfo, bool bQueryResult, FPlayerInfoSignature Delegate);

	void OnUpdatedTasks(const TArray<FTaskInProgressInfo>& UpdatedTasks);

	void OnReceiveRewardMessage(const FItemListHandle& ItemList);

	TArray<struct FGuideElement> GetGuideElements() const;
	
private:

	UPROPERTY()
	FTemporaryPlayerInfo TemporaryPlayerInfo;
	bool bInitializeTemporaryPlayerInfo;

	FOnPlayerAmmunitionChange OnAmmunitionChange;

	FDelegateHandle Handle_UpdateTask;
	FDelegateHandle Handle_ReceiveRewards;

	FTimerHandle Handle_RespawnPlayer;

	// 引导点
	UPROPERTY()
	TArray<AActor*> GuideActors;
};



