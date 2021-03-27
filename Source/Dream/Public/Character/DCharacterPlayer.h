// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "DCharacterBase.h"
#include "PlayerDataStore.h"
#include "DPlayerController.h"
#include "GameFramework/Character.h"
#include "DCharacterPlayer.generated.h"


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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shooter)
	float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shooter)
	float BaseLookUpRate;
	
	/* 瞄准时摄像机弹簧臂得偏移量 */
	UPROPERTY(EditAnywhere, Category = Shooter)
	FVector CameraArmOffset;
	UPROPERTY(EditAnywhere, Category = Shooter)
	FVector CameraArmAimOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Shooter)
	float SprintSpeed;
	UPROPERTY(BlueprintReadWrite, Category = Shooter)
	float NormalSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "Shooter")
	bool bAimed;
	UPROPERTY(BlueprintReadWrite, Category = "Shooter")
	bool bSprinted;
	UPROPERTY(BlueprintReadWrite, Category = "Shooter")
	bool bCanTurnPawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Animation")
	TMap<TEnumAsByte<EWeaponAnimGroup::Type>, FMontageSet> WeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Shooter|UI")
	TSubclassOf<class UPlayerHUD> PlayerHUDClass;

	UPROPERTY(EditAnywhere, Category = "Shooter|UI")
	TSubclassOf<class UDamageWidgetComponent> DamageWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon")
	FName WeaponSocketName;
	UPROPERTY(EditAnywhere, Category = "Shooter|Weapon")
	FName LeftShoulderSocketName;
	UPROPERTY(EditAnywhere, Category = "Shooter|Weapon")
	FName RightShoulderSocketName;

	UPROPERTY(EditAnywhere, Category = "Shooter|Weapon")
	float OutOfCombatTime;

	/**
	 * 雷达扫描半径， 用于显示小地图相关的东西
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|MiniMap")
	float RadarScanRadius;
	/**
	 * 雷达扫描间隔
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|MiniMap")
	float ScanInterval;
	/**
	 * 雷达扫描半径， 用于显示小地图相关的东西
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|MiniMap")
	TArray<TEnumAsByte<EObjectTypeQuery>> ScanObjectTypes;

#if WITH_EDITOR

	UPROPERTY(EditAnywhere, Category = Shooter)
	UDataTable* TestInitAttributes;
	
#endif

public:

	UFUNCTION(BlueprintCallable, Category = "Shooter")
	class AShootWeapon* GetActiveWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "Shooter")
	float PlayMontage(UAnimMontage* PawnAnim, UAnimMontage* WeaponAnim);

	UFUNCTION(BlueprintCallable, Category = "Shooter")
	class ADPlayerController* GetPlayerController() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, meta = (ScriptName="OnServerDeath", DisplayName="OnServerDeath"), Category = "Shooter")
	void BP_OnServerDeath(const AActor* Causer);

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName="OnDeath", DisplayName="OnDeath"), Category = "Shooter")
	void BP_OnDeath();

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnToggleWeaponAim", DisplayName = "OnToggleWeaponAim"), Category = "Shooter")
	void BP_OnToggleWeaponAim(bool bNewAimed);

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName="OnRepPlayerState", DisplayName="OnRepPlayerState"), Category = "Shooter")
	void BP_OnRepPlayerState();

	/*
		武器变更事件
	*/
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnActiveWeaponChanged", DisplayName = "OnActiveWeaponChanged"), Category = "Shooter")
	void BP_OnActiveWeaponChanged();

	/* 客户端开始重生 */
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnStartResurrection", DisplayName = "OnStartResurrection"), Category = "Shooter")
	void BP_OnStartResurrection(int32 ResurrectionTime);

	UFUNCTION(BlueprintCallable, Category = "Shooter")
	void SetPlayerHUDVisible(bool bVisible);

	UFUNCTION(server, reliable)
	void ServerSetPawnStatus(EPawnStatus NewPawnStatus);

	UFUNCTION(BlueprintCallable, Category = Shooter)
	FRotator GetRemoteControllerRotation() const;

	UFUNCTION(BlueprintImplementableEvent, Category = Shooter)
	void BP_OnHitTargetActor(float MakeDamage);

	UFUNCTION(BlueprintCallable, Category = Shooter)
	bool PickUpMagazine(EAmmoType AmmoType);

	UFUNCTION(BlueprintCallable, Category = Shooter)
	bool CanShoot() const;

	UFUNCTION(BlueprintCallable, Category = Shooter)
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = Shooter)
	bool CanEquip() const;

	UFUNCTION(BlueprintCallable, Category = Shooter)
	bool CanAim() const;

	UFUNCTION(BlueprintCallable, Category = Shooter)
	int32 GetWeaponAmmunition() const;

	UFUNCTION(BlueprintCallable, Category = Shooter)
	void SetInventoryWeapon(int32 Index, /*int32 WeaponAttackPower,*/ TSubclassOf<class AShootWeapon> NewWeaponClass);

	UFUNCTION(BlueprintCallable, Category = Shooter)
	const TArray<TSubclassOf<AShootWeapon>>& GetLocalEquippedWeaponClass() const;

	UFUNCTION(BlueprintCallable, Category = Shooter)
	void EquippedGear(TSubclassOf<class AGearBase> GearClass);

	UFUNCTION(BlueprintCallable, Category = Shooter)
	void ClearCombatStatus();

	UFUNCTION(BlueprintCallable, Category=Shooter)
    void ToggleCrossHairVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category=Shooter)
	void SwitchWeapon(int32 NewWeaponIndex);

	UFUNCTION(BlueprintCallable, Category=Shooter)
	void ActivateCharacterCapture();
	UFUNCTION(BlueprintCallable, Category=Shooter)
	void DeactivateCharacterCapture();
	UFUNCTION(BlueprintCallable, Category=Shooter)
	void ResetCharacterWeaponCapture();

	/**
	* 	添加一个永不消失的Sprite(除非手动删除)到小地图中
	* 	return 添加到数组中的索引
	*/
	UFUNCTION(BlueprintCallable, Category="Shooter|MiniMap")
	void AddInfiniteActors(const TArray<AActor*>& TargetActors);
	UFUNCTION(BlueprintCallable, Category="Shooter|MiniMap")
	void AddInfiniteActor(AActor* TargetActor);
	UFUNCTION(BlueprintCallable, Category="Shooter|MiniMap")
	void RemoveInfiniteActor(AActor* TargetActor);
	UFUNCTION(BlueprintCallable, Category="Shooter|MiniMap")
	void ClearInfiniteActors();

	/**
	 * @param Location 创建widget组件的世界位置
	 * @param DamageValue 伤害值
	 * @param bCritical 此次伤害是否暴击
	 * @param bIsHealthSteal 表示这个伤害是否是玩家造成的生命偷取
	 */
	UFUNCTION(BlueprintCallable, Category=Shooter)
	void SpawnDamageWidget(const FVector& Location, float DamageValue, bool bCritical, bool bIsHealthSteal);

	UFUNCTION(BlueprintCallable, Category=Shooter)
	void ShowHitEnemyTips(bool bEnemyDeath);

public:

	void SetActiveWeapon(AShootWeapon* NewWeapon);

	void SetPawnStatus(EPawnStatus NewPawnStatus);

	bool GetMiniMapTips(TArray<FMiniMapData>& Data);
	void DoAddMiniMapTips(TArray<FMiniMapData>& Data, const TArray<AActor*>& ScanActors);

	const FMontageSet* GetCurrentActionMontage() const;

	FName GetWeaponSlotNameFromIndex(int32 WeaponSlotIndex) const
	{
		return WeaponSlotIndex == 0 ? WeaponSocketName : WeaponSlotIndex == 1 ? LeftShoulderSocketName : RightShoulderSocketName;
	}

protected:

	UFUNCTION()
	void OnReq_ActiveWeapon();

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
	void ServerSetInventoryWeapon(int32 Index, TSubclassOf<class AShootWeapon> NewWeaponClass);
	
	UFUNCTION(Client, Reliable)
    void ClientSetLocalEquippedWeaponClass(int32 Index, UClass* WeaponClass);
	
	UFUNCTION(Server, Reliable)
	void ServerEquippedGear(TSubclassOf<class AGearBase> GearClass);

	/** 瞄准 */
	void StartAim();
	void StopAim();

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
	void AttemptSetStatusToRelax();

	void ModStatusToRelax();

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

	UPROPERTY(ReplicatedUsing = OnReq_ActiveWeapon)
	class AShootWeapon* ActiveWeapon;

	UPROPERTY()
	EWeaponStatus WeaponStatus;

	UPROPERTY(EditAnywhere)
	TArray<FAbilitySlot> DefaultAbilitySlot;
	
	UPROPERTY()
	TArray<FAbilitySlot> AbilitySlot;


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

	int32 ActiveWeaponIndex;

	/** 当前玩家已装备的武器 仅服务器有效 */
	UPROPERTY()
	TArray<class AShootWeapon*> WeaponInventory;

	/** 当前玩家已装备的武器 只对本地玩家有效 服务器此数组为空 目前只用于UI显示 */
	TArray<TSubclassOf<class AShootWeapon>> LocalEquippedWeaponClass;

	UPROPERTY()
	TMap<EGearType, class AGearBase*> Gears;

	FTimerHandle Handle_CombatToRelax;
	FTimerHandle Handle_CanTurn;

	FTimerHandle Handle_RadarScan;

	/* 武器相关 */
	FTimerHandle Handle_Shoot;
	FTimerHandle Handle_Reload;
	FTimerHandle Handle_Equip;

	/* 记录进入战斗状态的次数 */
	FThreadSafeCounter CombatStatusCounter;
	/* 记录开火按键是否按下 */
	bool bFireButtonDown;
};
