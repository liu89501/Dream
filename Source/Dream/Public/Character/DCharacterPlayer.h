// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable All
#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "DCharacterBase.h"
#include "PlayerDataStore.h"
#include "DPlayerController.h"
#include "GameFramework/Character.h"
#include "DCharacterPlayer.generated.h"

class AShootWeapon;
class UDModuleBase;


USTRUCT(BlueprintType)
struct DREAM_API FMontageSet
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* ShootAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* ReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* EquipAnim;

};

UCLASS()
class DREAM_API ADCharacterPlayer : public ADCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADCharacterPlayer();

	/** 第三人称相机组件 */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* TPCamera;
	/** 第三人称相机弹簧臂 */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* TPCameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAudioComponent* WalkAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneCaptureComponent2D* CharacterCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UPerkEffectSystemComponent* PerkEffectSystem;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterPlayer)
	float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterPlayer)
	float BaseLookUpRate;
	
	/* 瞄准时摄像机弹簧臂得偏移量 */
	UPROPERTY(EditAnywhere, Category = CharacterPlayer)
	FVector CameraArmOffset;
	UPROPERTY(EditAnywhere, Category = CharacterPlayer)
	FVector CameraArmAimOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = CharacterPlayer)
	float SprintSpeed;
	UPROPERTY(BlueprintReadWrite, Category = CharacterPlayer)
	float NormalSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "CharacterPlayer")
	bool bAimed;
	UPROPERTY(BlueprintReadWrite, Category = "CharacterPlayer")
	bool bSprinted;
	UPROPERTY(BlueprintReadWrite, Category = "CharacterPlayer")
	bool bCanTurnPawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Animation")
	UAnimMontage* CombatToRelax;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Animation")
	float CombatToRelaxAttachWaitTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Animation")
	UAnimMontage* RelaxToCombat;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Animation")
	float RelaxToCombatAttachWaitTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Animation")
	TMap<TEnumAsByte<EWeaponAnimGroup::Type>, FMontageSet> WeaponMontage;

	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|UI")
	TSubclassOf<class UPlayerHUD> PlayerHUDClass;

	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|UI")
	TSubclassOf<class UDamageWidgetComponent> DamageWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Weapon")
	FName WeaponSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Weapon")
	FName WeaponRelaxSocketName;
	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|Weapon")
	FName LeftShoulderSocketName;
	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|Weapon")
	FName RightShoulderSocketName;

	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|Weapon")
	float OutOfCombatTime;

	/**
	 * 雷达扫描半径， 用于显示小地图相关的东西
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|MiniMap")
	float RadarScanRadius;
	/**
	 * 雷达扫描间隔
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|MiniMap")
	float ScanInterval;
	/**
	 * 雷达扫描半径， 用于显示小地图相关的东西
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|MiniMap")
	TArray<TEnumAsByte<EObjectTypeQuery>> ScanObjectTypes;

#if WITH_EDITOR

	UPROPERTY(EditAnywhere, Category = CharacterPlayer)
	UDataTable* TestInitAttributes;
	
#endif

public:

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	AShootWeapon* GetActiveWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	float PlayMontage(UAnimMontage* PawnAnim, UAnimMontage* WeaponAnim);

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	class ADPlayerController* GetPlayerController() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, meta = (ScriptName="OnServerDeath", DisplayName="OnServerDeath"), Category = "CharacterPlayer")
	void BP_OnServerDeath(const AActor* Causer);

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName="OnDeath", DisplayName="OnDeath"), Category = "CharacterPlayer")
	void BP_OnDeath();

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnToggleWeaponAim", DisplayName = "OnToggleWeaponAim"), Category = "CharacterPlayer")
	void BP_OnToggleWeaponAim(bool bNewAimed);

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName="OnRepPlayerState", DisplayName="OnRepPlayerState"), Category = "CharacterPlayer")
	void BP_OnRepPlayerState();

	/*/*
		武器变更事件
	#1#
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnActiveWeaponChanged", DisplayName = "OnActiveWeaponChanged"), Category = "CharacterPlayer")
	void BP_OnActiveWeaponChanged();*/

	/* 客户端开始重生 */
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnStartResurrection", DisplayName = "OnStartResurrection"), Category = "CharacterPlayer")
	void BP_OnStartResurrection(int32 ResurrectionTime);

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	void SetPlayerHUDVisible(bool bVisible);

	UFUNCTION(server, reliable)
	void ServerSetCombatStatus(bool bNewCombatStatus);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	FRotator GetRemoteControllerRotation() const;

	UFUNCTION(BlueprintImplementableEvent, Category = CharacterPlayer)
	void BP_OnHitTargetActor(float MakeDamage);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool PickUpMagazine(EAmmoType AmmoType);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanShoot() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanEquip() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanAim() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	int32 GetWeaponAmmunition() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void SetInventoryWeapon(int32 Index, /*int32 WeaponAttackPower,*/ TSubclassOf<AShootWeapon> NewWeaponClass);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	const TArray<TSubclassOf<AShootWeapon>>& GetLocalEquippedWeaponClass() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void EquippedModule(int32 Index, TSubclassOf<UDModuleBase> ModuleClass);

	/*UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void ClearCombatStatus();*/

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
    void ToggleCrossHairVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=CharacterPlayer)
	void SwitchWeapon(int32 NewWeaponIndex);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=CharacterPlayer)
	void ToggleActiveWeaponStatus();

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void ActivateCharacterCapture();
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void DeactivateCharacterCapture();
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void ResetCharacterWeaponCapture();

	/**
	* 	添加一个永不消失的Sprite(除非手动删除)到小地图中
	* 	return 添加到数组中的索引
	*/
	UFUNCTION(BlueprintCallable, Category="CharacterPlayer|MiniMap")
	void AddInfiniteActors(const TArray<AActor*>& TargetActors);
	UFUNCTION(BlueprintCallable, Category="CharacterPlayer|MiniMap")
	void AddInfiniteActor(AActor* TargetActor);
	UFUNCTION(BlueprintCallable, Category="CharacterPlayer|MiniMap")
	void RemoveInfiniteActor(AActor* TargetActor);
	UFUNCTION(BlueprintCallable, Category="CharacterPlayer|MiniMap")
	void ClearInfiniteActors();

	/**
	 * @param Location 创建widget组件的世界位置
	 * @param DamageValue 伤害值
	 * @param bCritical 此次伤害是否暴击
	 * @param bIsHealthSteal 表示这个伤害是否是玩家造成的生命偷取
	 */
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void SpawnDamageWidget(const FVector& Location, float DamageValue, bool bCritical, bool bIsHealthSteal);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void ShowHitEnemyTips(bool bEnemyDeath);

public:

	void SetActiveWeapon(AShootWeapon* NewWeapon);

	void SetCombatStatus(bool bNewCombatStatus);

	bool GetMiniMapTips(TArray<FMiniMapData>& Data);

	const FMontageSet* GetCurrentActionMontage() const;

	FName GetWeaponSlotNameFromIndex(int32 WeaponSlotIndex) const
	{
		return WeaponSlotIndex == 0 ? WeaponRelaxSocketName : WeaponSlotIndex == 1 ? LeftShoulderSocketName : RightShoulderSocketName;
	}

protected:

	UFUNCTION()
	void OnRep_CombatStatus();

	UFUNCTION()
	void OnRep_ActiveWeapon();

	/** 开火 */
	void StartFire();
	void StopFire();
	void HandleFire();

	/** 换弹夹 */
	void ReloadMagazine();
	void HandleStopReload();
	void ReloadFinished();

	UFUNCTION(Reliable, Server)
	void ServerReloadMagazine();
	UFUNCTION(Reliable, NetMulticast)
	void MulticastReloadMagazine();

	/** 装备武器(切换武器) */
	void EquipWeapon();
	void EquipmentFinished();
	UFUNCTION(Reliable, Server)
	void ServerEquipWeapon();
	UFUNCTION(Reliable, NetMulticast)
	void MulticastEquipWeapon();

	UFUNCTION(Server, Reliable)
	void ServerSetInventoryWeapon(int32 Index, TSubclassOf<AShootWeapon> NewWeaponClass);
	
	UFUNCTION(Server, Reliable)
	void ServerEquippedModule(int32 Index, TSubclassOf<UDModuleBase> ModuleClass);

	/** 瞄准 */
	void StartAim();
	void StopAim();

	void SwitchCombat();

	/** 冲刺 */
	void ToggleSprint();
	void StartSprint();
	void StopSprint();
	UFUNCTION(server, reliable)
	void ServerStartSprint();
	UFUNCTION(server, reliable)
	void ServerStopSprint();
	
	UFUNCTION(server, reliable)
	void ServerStartAim();
	UFUNCTION(server, reliable)
	void ServerStopAim();

	void UpdateAmmoUI();

	/** 跳 */
	void StartJump();
	void StopJump();

	/** 下蹲 */
	void StartCrouch();
	void StopCrouch();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* InController) override;

	virtual void OnRep_PlayerState() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void Destroyed() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	UFUNCTION(Client, reliable)
	void ClientHitEnemy(const FDamageTargetInfo& DamageInfo);
	virtual void HitEnemy(const FDamageTargetInfo& DamageInfo) override;

	virtual int32 GetPickUpMagazineNumber(EAmmoType AmmoType) const;

	/* 尝试将Pawn状态修改未空闲 */
	/*void AttemptSetStatusToRelax();

	void ModStatusToRelax();*/

	void InitializeUI();

	void AimedMoveSpeedChange(bool bNewAim);

	void SprintMoveSpeedChange(bool bNewSprint);

	void OnInitPlayer(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage);

	void UpdateHealthUI();

	void RadarScanTick();

	virtual void RefreshAttributes();

	/* 客户端Pawn重生 一般处理UI相关逻辑 */
	UFUNCTION(Client, Reliable)
	void ClientResurrection(int32 ResurrectionTime);

	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	virtual void OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration);
	virtual void OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect);

	UPROPERTY(Replicated)
	FRotator ReplicatedCtrlRotation;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon)
	AShootWeapon* ActiveWeapon;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CombatStatus, Category = CharacterPlayer)
	bool bCombatStatus;

	UPROPERTY()
	EWeaponStatus WeaponStatus;

	UPROPERTY(EditAnywhere)
	TArray<FAbilitySlot> DefaultAbilitySlot;
	
	UPROPERTY()
	TArray<FAbilitySlot> AbilitySlot;

private:

	void DoAddMiniMapTips(TArray<FMiniMapData>& Data, const TArray<AActor*>& ScanActors);


private:

	/**
	 *  无视小地图扫描半径的Actor 将一直显示在小地图上 需要手动删除
	 */
	UPROPERTY()
	TArray<AActor*> InfiniteActors;
	/**
	 *  小地图扫描半径内的Actor
	 */
	UPROPERTY()
	TArray<AActor*> ActorWithinRadius;

	//TSharedPtr<class SPlayerHUD> PlayerHUD;
	UPROPERTY()
	class UPlayerHUD* PlayerHUD;

	/** 当前正在使用的武器 在 WeaponInventory 的索引 */
	int32 ActiveWeaponIndex;

	/** 当前玩家已装备的武器 仅服务器有效 */
	UPROPERTY()
	TArray<AShootWeapon*> WeaponInventory;

	/** 当前玩家已装备的武器 只对本地玩家有效 服务器此数组为空 目前只用于UI显示 */
	TArray<TSubclassOf<AShootWeapon>> LocalEquippedWeaponClass;

	/**
	 * 当前装备的模块
	 */
	TArray<TSubclassOf<UDModuleBase>> EquippedModules;

	FTimerHandle Handle_CombatStatus;
	FTimerHandle Handle_CanTurn;

	FTimerHandle Handle_RadarScan;

	/* 武器相关 */
	FTimerHandle Handle_Shoot;
	FTimerHandle Handle_Reload;
	FTimerHandle Handle_Equip;

	/* 记录进入战斗状态的次数 */
	//FThreadSafeCounter CombatStatusCounter;
	/* 记录开火按键是否按下 */
	bool bFireButtonDown;
};
