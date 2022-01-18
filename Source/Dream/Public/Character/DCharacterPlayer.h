// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DCharacterBase.h"
#include "DPropsType.h"
#include "GameplayAbilitySpec.h"
#include "PlayerDataInterfaceType.h"
#include "GameFramework/Character.h"
#include "DCharacterPlayer.generated.h"

class AShootWeapon;
class ADCharacterPlayer;
class UDModuleBase;
class UDProjectSettings;

enum class EAmmoType : uint8;


DECLARE_DYNAMIC_DELEGATE_OneParam(FDynamicOnInteractiveCompleted, ADCharacterPlayer*, PlayerCharacter);

UENUM(BlueprintType)
enum class EMovingDirection : uint8
{
	F,
    FL,
    FR,
    L,
    R,
    BL,
    BR,
    B
};

UENUM()
enum class EWeaponStatus : uint8
{
	Idle,
    Firing,
    Reloading,
    Equipping
};

USTRUCT()
struct FMoveInput
{
	GENERATED_BODY()
	
public:

	float X;

	float Y;

public:

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		if (Ar.IsSaving())
		{
			bOutSuccess &= WriteFixedCompressedFloat<1, 16>(X, Ar);
			bOutSuccess &= WriteFixedCompressedFloat<1, 16>(Y, Ar);
			return bOutSuccess;
		}

		ReadFixedCompressedFloat<1, 16>(X, Ar);
		ReadFixedCompressedFloat<1, 16>(Y, Ar);
		return bOutSuccess;
	}

	void Assign(FVector2D& V)
	{
		V.X = X;
		V.Y = Y;
	}

	bool Equals(const FVector2D& V)
	{
		return FMath::Abs(X - V.X) <= KINDA_SMALL_NUMBER && FMath::Abs(Y - V.Y) <= KINDA_SMALL_NUMBER;
	}
};

template<>
struct TStructOpsTypeTraits< FMoveInput > : public TStructOpsTypeTraitsBase2< FMoveInput >
{
	enum 
	{
		WithNetSerializer = true,
        WithNetSharedSerialization = true,
    };
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


	/** begin ======================= 摄像机瞄准插值参数 ============================ */
	
	/* 瞄准时摄像机弹簧臂得偏移量 */
	UPROPERTY(BlueprintReadOnly, Category = "CharacterPlayer|Camera")
	FVector DefaultArmSocketOffset;
	/** 瞄准时摄像机的位置 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "CharacterPlayer|Camera")
	FVector AimedArmSocketOffset;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterPlayer|Camera")
	FRotator DefaultCameraRotation;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "CharacterPlayer|Camera")
	FRotator AimedCameraRotation;

	/** end ======================= 摄像机瞄准插值参数 ============================ */

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = CharacterPlayer)
	float SprintSpeed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = CharacterPlayer)
	float AimMoveSpeed;
	UPROPERTY(BlueprintReadWrite, Category = CharacterPlayer)
	float NormalSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "CharacterPlayer")
	bool bAimed;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "CharacterPlayer")
	bool bSprinted;

	/**
	 * 默认属性值配置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Attribute")
	class UDBaseAttributesAsset* BaseAttributesSettings;

public:

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	AShootWeapon* GetActiveWeapon() const;

	/**
	 * 获取当前玩家正在使用的是几号位的武器
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	int32 GetActiveWeaponIndex() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	float PlayMontage(UAnimMontage* PawnAnim, UAnimMontage* WeaponAnim);

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	class ADPlayerController* GetPlayerController() const;

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
	EMovingDirection GetMovingDirection() const;

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
	void EquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void EquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void LearningTalents(const TArray<TSubclassOf<class UDreamGameplayAbility>>& TalentClasses);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
    void ToggleCrosshairVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=CharacterPlayer)
	void SwitchWeapon(int32 NewWeaponIndex);

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
	float GetCtrlYawDeltaCount() const;


	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void SetMouseInputScale(int32 Value);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void SetCharacterMesh(UCharacterMesh* CharacterMesh);

	/** 交互UI */
	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void EnableInteractiveButton(float InteractiveTime, FText Desc, FDynamicOnInteractiveCompleted Event);
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
	
public:

	const struct FCharacterMontage* GetCurrentActionMontage() const;

	/** Camera 相关 */
	void SetCameraFieldOfView(float NewFOV) const;
	void CameraAimTransformLerp(float Alpha) const;
	
	FORCEINLINE UCharacterMesh* GetCharacterMesh() const
	{
		return CurrentCharacterMesh;
	}

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

	/** 换弹夹 */
	void ReloadMagazine();
	void StopReloadMagazine();
	void ReloadFinished();

	UFUNCTION(Reliable, Server)
	void ServerReloadMagazine();
	UFUNCTION(Reliable, NetMulticast)
	void MulticastReloadMagazine();

	/** 装备武器(切换武器) */
	void SwitchWeapon();
	void HandleSwitchWeapon(int32 WeaponIndex);
	void SwitchFinished(int32 WeaponIndex);
	UFUNCTION(Reliable, Server)
	void ServerSwitchWeapon();
	UFUNCTION(Reliable, NetMulticast)
	void MulticastSwitchWeapon();

	void SwitchWeaponToFirst();
	void SwitchWeaponToSecond();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> NewWeaponClass);
	void DoServerEquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, UClass* NewWeaponClass);
	
	UFUNCTION(Server, Reliable)
	void ServerEquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs);
	void DoServerEquipModule(UClass* ModuleClass, const FEquipmentAttributes& Attrs);

	UFUNCTION(Server, Reliable)
	void ServerLearningTalent(const TArray<TSubclassOf<UDreamGameplayAbility>>& TalentClasses);

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

	void UpdateAmmoUI() const;

	/** 跳 */
	void StartJump();
	void StopJump();
	void SwitchCrouch();

	/** 下蹲 */
	void StartCrouch();
	void StopCrouch();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void ReplicationServerMoveDirection();

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

	void AimedMoveSpeedChange(bool bNewAim) const;

	void SprintMoveSpeedChange(bool bNewSprint) const;

	void OnInitPlayer(const FPlayerInfo& PlayerInfo, bool bSuccess);

	void OnPlayerPropertiesChanged(const FPlayerProperties& Properties);

	/** 属性相关 */
    void RefreshAttributeBaseValue();
	
    void FastRefreshWeaponAttribute(const FEquipmentAttributes& PrevWeaponAttrs);
	
	UFUNCTION(Server, Reliable)
    void ServerSetCharacterLevel(int32 NewLevel);

	UFUNCTION(Server, Reliable)
    void ServerInitializePlayer(const FPlayerInfo& PlayerInfo);

	UFUNCTION(Server, UnReliable)
	void ServerUpdateMovingInput(const FMoveInput& NewMovingInput);

	UFUNCTION(Server, Reliable)
	void ServerUpdateCharacterMesh(UCharacterMesh* CharacterMesh);

	UFUNCTION()
	void OnRep_CharacterMesh();

	void UpdateCharacterMesh();
	
	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	virtual void OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration);
	virtual void OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect);

private:

	struct FMinimapDataIterator GetMinimapDataIterator() const;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon)
	AShootWeapon* ActiveWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = CharacterPlayer)
	bool bCombatStatus;
	uint16 CombatStatusCount;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterMesh, Category = CharacterPlayer)
	UCharacterMesh* CurrentCharacterMesh;

	EWeaponStatus WeaponStatus;

	bool bFiring;

	// 前两个字节为X , 后两个字节为Y
	UPROPERTY(Replicated)
	FMoveInput MovingInput;
	
	FVector2D PrevMovingInput;

private:

	static UDProjectSettings* CDOProjectSettings;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	int32 BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	int32 BaseLookUpRate;
	
	/**
	 * 存放武器perk的Handle
	 */
	TArray<FGameplayAbilitySpecHandle> CacheWeaponPerkHandles;
	

	TSharedPtr<class SPlayerHUD> PlayerHUD;

	/** 当前正在使用的武器 在 WeaponInventory 的索引 */
	UPROPERTY(Replicated)
	int32 ActiveWeaponIndex;

	/** 当前玩家已装备的武器 仅服务器有效 */
	UPROPERTY()
	TArray<AShootWeapon*> WeaponInventory;
	/** 当前装备的模块 */
	UPROPERTY()
	TArray<UDModuleBase*> EquippedModules;

	UPROPERTY()
	TArray<TSubclassOf<UDreamGameplayAbility>> LearnedTalents; 

	/** 当前玩家已装备的武器 只对本地玩家有效 服务器此数组为空 目前只用于UI显示 */
	//TArray<TSubclassOf<AShootWeapon>> LocalEquippedWeaponClass;

	/** 战斗状态Handle */
	FTimerHandle Handle_CombatStatus;
	

	/* 武器相关 */
	FTimerHandle Handle_Shoot;
	FTimerHandle Handle_Reload;
	FTimerHandle Handle_Equip;

	FDelegateHandle Handle_Properties;
	FDelegateHandle Handle_PlayerInfo;

	/** 控制器 yaw 的累计值 */	
	float CtrlYawDeltaCount;
	float PrevCtrlYaw;
};

