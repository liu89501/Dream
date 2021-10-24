// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable All
#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "DCharacterBase.h"
#include "DModuleBase.h"
#include "GameplayAbilitySpec.h"
#include "DPlayerController.h"
#include "PlayerDataInterfaceType.h"
#include "GameFramework/Character.h"
#include "DCharacterPlayer.generated.h"

class AShootWeapon;

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
	class UAudioComponent* AudioComponent;

	/**
	 * 这个character是否用作预览(会关掉一些不必要的功能)
	 */
	UPROPERTY(EditAnywhere, Category = CharacterPlayer)
	bool bIsPreviewCharacter;

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
	
	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|UI")
	TSubclassOf<class UPlayerHUD> PlayerHUDClass;

	UPROPERTY(EditAnywhere, Category = "CharacterPlayer|UI")
	TSubclassOf<class UDamageWidgetComponent> DamageWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Weapon")
	FName WeaponSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Weapon")
	FName WeaponHolsterSocketName;

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

	/**
	 * 默认属性值配置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Attribute")
	class UDBaseAttributesAsset* BaseAttributesSettings;

public:

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	AShootWeapon* GetActiveWeapon() const;

	/**
	 * 一般用于在交互时需要调用一下这个，避免出现在瞄准状态下打开商店时导致无法取消瞄准状态
	 */
	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void ForceModifyStateToIdle();

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
	int32 GetWeaponAmmunition() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void EquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void EquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void LearningTalents(const TArray<TSubclassOf<class UDreamGameplayAbility>>& TalentClasses);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
    void ToggleCrossHairVisible(bool bVisible);

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
	void SetCombatStatus(bool bNewCombatStatus);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
    UPlayerHUD* GetPlayerHUD() const;

	bool GetMiniMapTips(TArray<FMiniMapData>& Data);

	const struct FCharacterMontage* GetCurrentActionMontage() const;

	/** Camera 相关 */
	void SetCameraFieldOfView(float NewFOV);
	void CameraAimTransformLerp(float Alpha);
	
	/** GameplayAbility 相关 */

	/**
	 *	通过GameplayTag 触发Ability
	 *	@param Target Ability应用的目标 如果有的话 没有就是自身
	 */
	void TriggerAbilityFromTag(const FGameplayTag& Tag, AActor* Target);
	
protected:

	/*UFUNCTION()
	void OnRep_CombatStatus();*/

	UFUNCTION()
	void OnRep_ActiveWeapon();

	/** 开火 */
	void StartFire();
	void StopFire();
	void ConfirmFire();
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

	UFUNCTION(server, reliable)
    void ServerSetCombatStatus(bool bNewCombatStatus);

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

	/** 状态切换相关 */
	void ModStatusToRelax();
	void AttemptSetStatusToRelax();
	void ClearCombatStatusCounter();

	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Destroyed() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;
	
	//virtual void MaxHealthChanged(const FOnAttributeChangeData& AttrData);

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle) override;

	UFUNCTION(Client, reliable)
	void ClientHitEnemy(const FDamageTargetInfo& DamageInfo);
	virtual void HitEnemy(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget) override;

	virtual int32 GetPickUpMagazineNumber(EAmmoType AmmoType) const;

	/* 尝试将Pawn状态修改未空闲 */
	/*void AttemptSetStatusToRelax();

	void ModStatusToRelax();*/

	void InitializeUI();

	void AimedMoveSpeedChange(bool bNewAim);

	void SprintMoveSpeedChange(bool bNewSprint);

	void OnInitPlayer(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage);

	void OnPlayerExperienceChanged(int32 MaxExp, int32 CrtExp, int32 NewLevel);

	/** 属性相关 */
	void AdditiveAttributes(const FEquipmentAttributes& Attributes);
    void RefreshAttributeBaseValue();
    void FastRefreshWeaponAttribute(const FEquipmentAttributes& PrevWeaponAttrs);
	
	UFUNCTION(Server, Reliable)
    void ServerSetCharacterLevel(int32 NewLevel);

	void RadarScanTick();

	UFUNCTION(Server, Reliable)
    void ServerInitializePlayer(const FPlayerInfo& PlayerInfo);

	/* 客户端Pawn重生 一般处理UI相关逻辑 */
	UFUNCTION(Client, Reliable)
	void ClientResurrection(int32 ResurrectionTime);

	UFUNCTION(Server, UnReliable)
	void ServerUpdateMovingInput(const FVector2D& NewMovingInput);

	// todo 这里可能会有问题 试想如果一个人在视野之外这个RPC在其他客户端能否接收到，这是个问题需要测试
	UFUNCTION(Server, Reliable)
	void ServerUpdateCharacterMesh(UCharacterMesh* CharacterMesh);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdateCharacterMesh(UCharacterMesh* CharacterMesh);
	void ProcessUpdateCharacterMesh(UCharacterMesh* CharacterMesh);

	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	virtual void OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration);
	virtual void OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect);

protected:

	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon)
	AShootWeapon* ActiveWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = CharacterPlayer)
	bool bCombatStatus;

	UPROPERTY()
	EWeaponStatus WeaponStatus;

	UPROPERTY(Replicated)
	FVector2D MovingInput;
	
	FVector2D PrevMovingInput;
	

private:

	void DoAddMiniMapTips(TArray<FMiniMapData>& Data, const TArray<AActor*>& ScanActors);


private:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	float BaseLookUpRate;
	
	/**
	 * 存放武器perk的Handle
	 */
	TArray<FGameplayAbilitySpecHandle> CacheWeaponPerkHandles;
	
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
	UPROPERTY(Replicated)
	int32 ActiveWeaponIndex;

	/** 当前玩家已装备的武器 仅服务器有效 */
	UPROPERTY()
	TArray<AShootWeapon*> WeaponInventory;
	/** 当前装备的模块 */
	UPROPERTY()
	TMap<EModuleCategory, UDModuleBase*> EquippedModules;

	UPROPERTY()
	TArray<TSubclassOf<UDreamGameplayAbility>> LearnedTalents; 

	/** 当前玩家已装备的武器 只对本地玩家有效 服务器此数组为空 目前只用于UI显示 */
	//TArray<TSubclassOf<AShootWeapon>> LocalEquippedWeaponClass;

	/** 战斗状态Handle */
	FTimerHandle Handle_CombatStatus;
	
	//FTimerHandle Handle_CanTurn;

	/** 雷达扫描handle */
	FTimerHandle Handle_RadarScan;

	/* 武器相关 */
	FTimerHandle Handle_Shoot;
	FTimerHandle Handle_Reload;
	FTimerHandle Handle_Equip;

	FDelegateHandle Handle_Exp;

	/* 记录进入战斗状态的次数 */
	FThreadSafeCounter CombatStatusCounter;
	
	/* 记录开火按键是否按下 */
	bool bFireButtonDown;

	/** 控制器 yaw 的累计值 */	
	float CtrlYawDeltaCount;
	float PrevCtrlYaw;

	UPROPERTY()
	UClass* SlaveMeshAnimBPClass;
};

