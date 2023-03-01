// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "DCharacterBase.h"
#include "DPropsType.h"
#include "DMPlayerController.h"
#include "GameplayAbilitySpec.h"
#include "PlayerDataInterfaceType.h"
#include "DMCharacterType.h"
#include "GameFramework/Character.h"
#include "DCharacterPlayer.generated.h"

class AShootWeapon;
class ADCharacterPlayer;
class UDModuleBase;
class SPlayerHUD;

enum class EAmmoType : uint8;

DECLARE_DYNAMIC_DELEGATE(FDynamicOnInteractiveCompleted);

struct DebugDMCharacterCVar
{
	static TAutoConsoleVariable<int32> CVar;
};


USTRUCT()
struct FMantleSpec
{
	GENERATED_USTRUCT_BODY()

	FMantleSpec()
		: MantleType(EMantleType::None),
		  StartingPosition(0),
		  PlayRate(0)
	{
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << MantleType;

		if (Ar.IsSaving())
		{
			WriteFixedCompressedFloat<255, 16>(StartingPosition, Ar);
			WriteFixedCompressedFloat<255, 16>(PlayRate, Ar);
		}
		else
		{
			ReadFixedCompressedFloat<255, 16>(StartingPosition, Ar);
			ReadFixedCompressedFloat<255, 16>(PlayRate, Ar);
		}

		Ar << MantlingTarget;
		Ar << AnimatedStartOffset;
		
		bOutSuccess = true;
		return true;
	}

	UPROPERTY()
	EMantleType MantleType;

	UPROPERTY()
	float StartingPosition;

	UPROPERTY()
	float PlayRate;

	UPROPERTY()
	FVector_NetQuantize100 MantlingTarget;

	UPROPERTY()
	FVector_NetQuantize100 AnimatedStartOffset;
};

template<>
struct TStructOpsTypeTraits< FMantleSpec > : TStructOpsTypeTraitsBase2< FMantleSpec >
{
	enum 
	{
		WithNetSerializer = true
    };
};

USTRUCT(BlueprintType)
struct FCameraViewLerp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ArmLength;

	UPROPERTY(EditAnywhere)
	FVector ArmSocketOffset;

	UPROPERTY(EditAnywhere)
	FRotator CameraRotation;

	FCameraViewLerp LerpTo(const FCameraViewLerp& Target, float Alpha) const;
};

UCLASS(Blueprintable)
class UCharacterMesh : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* MasterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<USkeletalMesh*> SlaveMeshs;
};

UCLASS()
class DREAM_API ADCharacterPlayer : public ADCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADCharacterPlayer();

	/** 第三人称相机组件 */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	class UCameraComponent* TPCamera;
	/** 第三人称相机弹簧臂 */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	class USpringArmComponent* TPCameraArm;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	class UAudioComponent* AudioComponent;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	class UMinimapScanComponent* ScanComponent;


	/** 人物的Yaw角度限制 超过这个范围会旋转自身 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterPlayer)
	float MinRotateLimit;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterPlayer)
	float MaxRotateLimit;


	/** begin ======================= 摄像机相关插值参数 ============================ */
	FCameraViewLerp DefaultView;
	
	UPROPERTY(EditDefaultsOnly, Category = "CharacterPlayer")
	FCameraViewLerp View_Aiming;

	UPROPERTY(EditDefaultsOnly, Category = "CharacterPlayer")
	FCameraViewLerp View_Rolling;
	
	UPROPERTY(EditDefaultsOnly, Category = "CharacterPlayer")
	UCurveFloat* RollingCameraCurve;
	/** end ======================= 摄像机相关插值参数 ============================ */

	/** 冲刺速度 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = CharacterPlayer)
	float SprintSpeed;
	
	/** 步行速度 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = CharacterPlayer)
	float WalkSpeed;
	
	/** 跑步速度 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = CharacterPlayer)
	float RunSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterPlayer")
	bool bAimed;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Category = "CharacterPlayer")
	TArray<TSubclassOf<UGameplayAbility>> TestAbilities;

#endif

public:

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	AShootWeapon* GetActiveWeapon() const;

	/**
	 * 获取当前玩家正在使用的是几号位的武器
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	int32 GetActiveWeaponIndex() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	class ADMPlayerController* GetPlayerController() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, meta = (ScriptName="OnServerDeath", DisplayName="OnServerDeath"), Category = "CharacterPlayer")
	void BP_OnServerDeath(const AActor* Causer);

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnToggleWeaponAim", DisplayName = "OnToggleWeaponAim"), Category = "CharacterPlayer")
	void BP_OnToggleWeaponAim(bool bNewAimed);

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName="OnRepPlayerState", DisplayName="OnRepPlayerState"), Category = "CharacterPlayer")
	void BP_OnRepPlayerState();

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	void SetPlayerHUDVisible(bool bVisible);
	
	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	void RemovePlayerHUD();

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool PickUpMagazine(EAmmoType AmmoType);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanEquip() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	bool CanAim() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void LearningTalents(int64 Talents);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
    void ToggleCrosshairVisible(bool bVisible);

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

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void SetMouseInputScale(int32 Value);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void SetCharacterMesh(UCharacterMesh* CharacterMesh);

	/** 交互UI */
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void EnableInteractiveButton(float InteractiveTime, FText Desc, FDynamicOnInteractiveCompleted Event);

	/** 显示交互按钮 */
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void DisplayInteractiveButton(FText Desc);
	
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void DisableInteractiveButton();

	/**
	* 一般用于在交互时需要调用一下这个，避免出现在瞄准状态下打开商店时导致无法取消瞄准状态
	*/
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void StopAllActions();

	/**
	 * 恢复当前武器的弹药量 (不会消耗武器的储备弹药)
	 */
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void RecoveryActiveWeaponAmmunition(int32 AmmoNum);

	
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	bool IsLocalCharacter() const;

	/** 非服务器上的角色 */
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	bool IsNotServerCharacter() const;

	FRotator GetReplicatedControlRotation() const;
	
public:

	void EquipWeapon(const FGearDesc& GearDesc);

	void EquipModule(const FGearDesc& GearDesc);

	void GetCameraLocationAndRotation(FVector& CameraLocation, FRotator& CameraRotation);

	float PlayMontage(UAnimMontage* PawnAnim);

	float PlayReloadingMontage();
	void StopReloadingMontage();
	
	float PlayEquippingMontage();

	/** Camera 相关 */
	void SetCameraFieldOfView(float NewFOV) const;
	void CameraAimTransformLerp(float Alpha);

	void SetCameraView(const FCameraViewLerp& NewView);

	/** 血量是否有损失 */
	bool IsUnhealthy() const;

	/** 更新 OverlayDetailID, 这个主要是用于不同武器的动画混合 */
	void UpdateOverlayDetailID(int32 NewDetailID);

protected:

	UFUNCTION()
	void OnRep_ActiveWeapon();

	/** 状态切换相关 */
	void StateSwitchToCombat();
	void StateSwitchToRelax();
	void StateSwitchToRelaxImmediately();
	
	void SetStateToRelax();
	
	UFUNCTION(server, reliable)
    void ServerSetCombatState(bool bNewCombatState);

	/** 开火 */
	void StartFire();
	void StopFire();
	void ConfirmFire();
	void HandleFire();

	void UpdateMagazineUI() const;

	/** 换弹夹 */
	void ReloadMagazine();
	void StopReloadMagazine();
	void ReloadFinished();

	/** 装备武器(切换武器) */
	void ToggleWeapon();
	void HandleToggleWeapon(int32 WeaponIndex);
	void ToggleWeaponFinished(int32 WeaponIndex);

	UFUNCTION(Reliable, Server)
	void ServerToggleWeapon(int32 NewWeaponIndex);

	void ToggleWeaponToFirst();
	void ToggleWeaponToSecond();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(const FGearDesc& GearDesc);
	void DoServerEquipWeapon(const FGearDesc& GearDesc);
	
	UFUNCTION(Server, Reliable)
	void ServerEquipModule(const FGearDesc& GearDesc);
	void DoServerEquipModule(const FGearDesc& GearDesc);

	UFUNCTION(Server, Reliable)
	void ServerLearningTalent(int64 Talents);

	/** 瞄准 */
	void StartAim();
	void StopAim();

	UFUNCTION(Server, Reliable)
	void ServerSetDesiredGait(EMovementGait NewGait);
	void SetDesiredGait(EMovementGait NewGait);
	
	/** 冲刺 */
	void ToggleSprint();
	void StartSprint();
	void StopSprint();

	/** 攀爬 */
	void MantleStart();
	void HandleMantleStart();
	void PlayMantleAnim();
	
	UFUNCTION()
    void OnRep_MantlingSpec();
	UFUNCTION(Server, Reliable)
	void ServerSetMantlingSpec(const FMantleSpec& NewMantleSpec);
	void SetMantlingSpec(const FMantleSpec& NewMantleSpec);

	
	UFUNCTION(Server, Reliable)
	void Server_Rolling(ERollingDirection NewRollingDirection);
	UFUNCTION(NetMulticast, UnReliable)
	void Multicast_Rolling(ERollingDirection NewRollingDirection);

	void OnRollingViewLerp(float Value);
	void PlayRollingAnimation(ERollingDirection NewRollingDirection);
	void HandleRolling(ERollingDirection NewRollingDirection);
	void Rolling();
	void OnRollingFinished();


	/** 切换Overlay */
	void ToggleOverlay();
	UFUNCTION(Server, Reliable)
	void ServerSetOverlayState(EOverlayState NewOverlay);

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	bool CanMove() const;

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

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	
	virtual void BeginPlay() override;

	void InitializePlayerHUD();

	SPlayerHUD* GetOrCreatePlayerHUD();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* InController) override;

	virtual void OnRep_PlayerState() override;

	void SetActiveWeapon(AShootWeapon* NewWeapon);

	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Destroyed() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;
	
	//virtual void MaxHealthChanged(const FOnAttributeChangeData& AttrData);

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle) override;

	UFUNCTION(Client, unreliable)
    void ClientReceiveDamage(AActor* Causer);

	UFUNCTION(Client, unreliable)
	void ClientHitEnemy(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget);
	virtual void HitEnemy(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget) override;

	/* 尝试将Pawn状态修改未空闲 */
	/*void AttemptSetStatusToRelax();

	void ModStatusToRelax();*/

	// [server]
	void InitializePlayerGears();
    void OnInitializePlayerGears(bool bValidResult, const struct FPlayerDesc& PlayerInfo);

	void OnPlayerPropertiesChanged(const FPlayerProperties& Properties);

	/** 收集装备属性 */
	void GatherGearsAttributes(FEquipmentAttributes& GearsAttributes);

	/**
	 * 将装备的perk能力应用到自身 (如果有)
	 */
	void ApplyGearsAbilitiesToSelf(const TArray<int32>& GearPerks);
	
	/** 属性相关 */
	void RefreshAttributeBaseValue();
	
    void FastRefreshWeaponAttribute(const FEquipmentAttributes& PrevWeaponAttrs);
	
	UFUNCTION(Server, Reliable)
    void ServerSetCharacterLevel(int32 NewLevel);

	/*UFUNCTION(Server, UnReliable)
	void ServerUpdateMovingDir(EMovingDirection NewMovingDirection);*/

	UFUNCTION(Server, Reliable)
	void ServerUpdateCharacterMesh(UCharacterMesh* CharacterMesh);

	UFUNCTION()
	void OnRep_CharacterMesh();
	void UpdateCharacterMesh();

	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	virtual void OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration);
	virtual void OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect);

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// 移动或战斗状态下更新Actor的旋转
	void SmoothUpdateRotation(float DeltaSeconds, float RotationRate, float RotRateConst, const FRotator& Target);

	void LimitRotation(float DeltaSeconds, float YawMin, float YawMax, float InterpSpeed);

	void UpdateGroundedRotation(float DeltaSeconds);
	void UpdateEssentialValues(float DeltaSeconds);

	void MantlingUpdate(float Alpha);
	void OnMantleFinished();

	float GetMappedSpeed();

	FVector GetInputVector() const;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon)
	AShootWeapon* ActiveWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = CharacterPlayer)
	bool bCombatStatus;
	uint16 CombatStatusCount;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterMesh, Category = CharacterPlayer)
	UCharacterMesh* CurrentCharacterMesh;

	bool bDesiredShoot;

	FVector2D AxisInput;

	UPROPERTY(Replicated)
	FVector_NetQuantizeNormal ControllerRotation;

	/** =============== AnimData ================ */

	FVector PrevVelocity;

	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	FVector Acceleration;
	
	FRotator LastVelocityRotation;

	FRotator LastMovementInputRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	float MovementInputAmount;
	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	bool bHasMovementInput;

	FRotator TargetRotation;
	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	float AimYawRate;
	float PrevAimYaw;

	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	FRotator AimingRotation;

	// [local] 默认的步态
	EMovementGait DefaultGait;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "CharacterPlayer")
	EMovementGait Gait;

	UPROPERTY(BlueprintReadWrite, Category = CharacterPlayer)
	EMovementState MovementState;

	UPROPERTY(BlueprintReadWrite, Category = CharacterPlayer)
	EMovementAction MovementAction;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = CharacterPlayer)
	EOverlayState OverlayState;

	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	int32 OverlayDetailID;

	UPROPERTY(ReplicatedUsing = OnRep_MantlingSpec)
	FMantleSpec MantlingSpec;

	UPROPERTY()
	FTimeline Timeline_Mantling;
	
	UPROPERTY()
	FTimeline Timeline_Rolling;

	/** Mantling Temporary Vars */
	UPROPERTY()
	UCurveVector* MantlingCorrectionCurve;
	
	FVector TemporaryMantlingTarget;
	FVector TemporaryActualStartOffset;
	FVector TemporaryAnimatedStartOffset;

	FRotator FaceRotation;

	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	bool bIsMoving;
	
	UPROPERTY(BlueprintReadOnly, Category = CharacterPlayer)
	float Speed;

	bool bDisableGroundedRotation;

private:

	struct FMinimapDataIterator GetMinimapDataIterator() const;

private:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	int32 BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	int32 BaseLookUpRate;
	
	/**
	 * 存放武器perk的Handle
	 */
	TArray<FGameplayAbilitySpecHandle> CacheWeaponPerkHandles;
	

	TSharedPtr<SPlayerHUD> PlayerHUD;

	/** 当前正在使用的武器 在 WeaponInventory 的索引 */
	UPROPERTY(Replicated)
	uint8 ActiveWeaponIndex;

	/*UPROPERTY(Replicated)
	uint8 WeaponOverlayDetailID;*/

	/** 当前玩家已装备的武器 仅服务器有效 */
	UPROPERTY()
	TArray<AShootWeapon*> WeaponInventory;
	/** 当前装备的模块 */
	UPROPERTY()
	TArray<UDModuleBase*> EquippedModules;

	/** 已学习的天赋 */
	UPROPERTY()
	TArray<UClass*> LearnedTalents; 

	/** 当前玩家已装备的武器 只对本地玩家有效 服务器此数组为空 目前只用于UI显示 */
	//TArray<TSubclassOf<AShootWeapon>> LocalEquippedWeaponClass;

	/** 战斗状态Handle */
	FTimerHandle Handle_CombatStatus;
	
	/* 武器相关 */
	FTimerHandle Handle_Shoot;
	FTimerHandle Handle_Reload;
	FTimerHandle Handle_Equip;
	FTimerHandle Handle_Rolling;

	FDelegateHandle Handle_Properties;

	/** 用于生命恢复的计时器 */
	FTimerTemplate RecoveryTimer;
	FIntervalGate HealthUpdateFrequency;


	FTimeInterval RollingLimit;
	FTimeInterval MantleLimit;
	FTimeInterval ToggleOverlayLimit;
};


