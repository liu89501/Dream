// ReSharper disable All
#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "GameplayTagContainer.h"
#include "GearInterface.h"
#include "ProjectileDamageInterface.h"
#include "ShootWeapon.generated.h"

#define ENABLE_WEAPON_DEBUG !(UE_BUILD_TEST || UE_BUILD_SHIPPING)

#if ENABLE_WEAPON_DEBUG

struct DebugWeaponCVar
{
	static TAutoConsoleVariable<int32> DebugCVar;
};

#endif

class UDMUpgradeGearInfluence;

UENUM()
enum class EWeaponState : uint8
{
	Idle,
    Shooting,
    Reloading,
    Equipping
};

USTRUCT()
struct FHitInfo
{
	GENERATED_BODY()

public:
	virtual ~FHitInfo() = default;

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual TSharedPtr<FHitInfo> Clone() const;

	virtual UScriptStruct* GetScriptStruct() const
	{
		return FHitInfo::StaticStruct();
	}
};


USTRUCT()
struct FHitInfo_SingleBullet : public FHitInfo
{
	GENERATED_BODY()

public:

	FHitInfo_SingleBullet() : HitResult()
	{
	}

	FHitInfo_SingleBullet(const FHitResult& InHitResult)
		: HitResult(TSharedPtr<FHitResult>(new FHitResult(InHitResult)))
	{
	}

	FHitResult* GetHitResult() const
	{
		return HitResult.Get();
	}

	virtual UScriptStruct* GetScriptStruct() const
	{
		return FHitInfo_SingleBullet::StaticStruct();
	}

	virtual TSharedPtr<FHitInfo> Clone() const override;
	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

private:

	TSharedPtr<FHitResult> HitResult;
};


template<>
struct TStructOpsTypeTraits<FHitInfo_SingleBullet> : public TStructOpsTypeTraitsBase2<FHitInfo_SingleBullet>
{
	enum
	{
		WithNetSerializer = true
    };
};

USTRUCT()
struct FHitInfo_MultiBullet : public FHitInfo
{
	GENERATED_BODY()

public:

	FHitInfo_MultiBullet()
	{
	}

	FHitInfo_MultiBullet(const TArray<FHitResult>& InHits) : Hits(InHits)
	{
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	const TArray<FHitResult>& GetHits() const
	{
		return Hits;
	}

	virtual UScriptStruct* GetScriptStruct() const
	{
		return FHitInfo_MultiBullet::StaticStruct();
	}

	virtual TSharedPtr<FHitInfo> Clone() const override;

private:

	UPROPERTY()
	TArray<FHitResult> Hits;
};


template<>
struct TStructOpsTypeTraits<FHitInfo_MultiBullet> : public TStructOpsTypeTraitsBase2<FHitInfo_MultiBullet>
{
	enum
	{
		WithNetSerializer = true
    };
};

USTRUCT()
struct FHitInfoHandle
{
	GENERATED_BODY()

public:
	FHitInfoHandle()
	{
	}

	explicit FHitInfoHandle(FHitInfo* InHitInfo)
	{
		Data = TSharedPtr<FHitInfo>(InHitInfo);
	}

	template<typename Type>
    Type* Get() const;

	void Set(const TSharedPtr<FHitInfo>& HitInfo)
	{
		Data = HitInfo;
	}

	bool IsValid() const
	{
		return Data.IsValid();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	bool operator==(const FHitInfoHandle& Other) const;
	
	bool operator!=(const FHitInfoHandle& Other) const;

private:

	TSharedPtr<FHitInfo> Data;
};

template<>
struct TStructOpsTypeTraits<FHitInfoHandle> : public TStructOpsTypeTraitsBase2<FHitInfoHandle>
{
	enum
	{
		WithCopy = true,
		WithNetSerializer = true,
		WithIdenticalViaEquality = true
    };
};


UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAutomatic UMETA(DisplayName="半自动"),
	FullyAutomatic UMETA(DisplayName="全自动"),
	Accumulation UMETA(DisplayName="蓄力")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	AssaultRifle UMETA(DisplayName="突击步枪"),
	GrenadeLaunch UMETA(DisplayName="榴弹发射器"),
	Shotgun UMETA(DisplayName="霰弹枪"),
	SniperRifle UMETA(DisplayName="狙击步枪"),
	PrecisionRifle UMETA(DisplayName="精准步枪") 
};

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Level1,
    Level2,
    Level3
};

USTRUCT(BlueprintType)
struct FWeaponTrailVFX
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailEffect;
	UPROPERTY(EditAnywhere)
	float MinimumSpawnDistance;
	UPROPERTY(EditAnywhere)
	float TrailFlyingSpeed;
	UPROPERTY(EditAnywhere)
	FVector SpawnPositionOffset;
	UPROPERTY(EditAnywhere)
	FName TrailEndLocationParamName;
	UPROPERTY(EditAnywhere)
	FName TrailLifeTimeParamName;


	FWeaponTrailVFX()
        : TrailEffect(nullptr),
          MinimumSpawnDistance(200.f),
          TrailFlyingSpeed(25000.f),
          SpawnPositionOffset(FVector::ZeroVector),
          TrailEndLocationParamName(TEXT("EndPosition")),
          TrailLifeTimeParamName(TEXT("NCLifeTime"))
	{
	}
};

USTRUCT(BlueprintType)
struct FWeaponShootMuzzle
{
	GENERATED_USTRUCT_BODY()

public:

	FWeaponShootMuzzle() :
        Particles(nullptr),
        Sound(nullptr),
        Size(FVector::OneVector)
	{
	};

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Particles;
	UPROPERTY(EditAnywhere)
	class USoundBase* Sound;
	UPROPERTY(EditAnywhere)
	FVector Size;

	FORCEINLINE bool IsValid() const
	{
		return Particles != nullptr && Sound != nullptr;
	}
};


USTRUCT(BlueprintType)
struct DREAM_API FCharacterMontage
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

UCLASS(Abstract)
class DREAM_API AShootWeapon : public AActor, public IGearInterface, public IProjectileDamageInterface
{
	GENERATED_BODY()

public:

	AShootWeapon();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	EWeaponType WeaponType;

	/**
	 * 武器射击一次发射的弹片数量
	 */
	UPROPERTY(EditDefaultsOnly, Category=Weapon, meta = (ClampMin = 1, ClampMax = 16))
	int32 ShellFragment;
	
	/**
	 * 武器开火模式
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	EFireMode FireMode;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	TSubclassOf<class UCameraShakeBase> FireCameraShake;

	/** 每分钟射速 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	int32 RateOfFire;

	/** 每个弹夹的子弹数量 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	int32 AmmoNum;

	/** 储备弹药 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	int32 ReserveAmmo;
	
	/** 弹药类型 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, Category = Weapon)
	UDMUpgradeGearInfluence* UpgradeInfluenceAttributes;
	
	
	/** 瞄准时的摄像机FOV */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	float AimFOV;

	/** 瞄准时的精准度阈值 数值越大越精准 为0时表示完全没有偏移 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	float AccuracyThreshold;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FTransform WeaponSocketOffset;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FTransform WeaponHolsterSocketOffset;

	/** 武器预览时的变换 */
	UPROPERTY(EditAnywhere, Category = Weapon)
	FTransform PreviewTransform;
	
	/** WeaponUI */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|UI")
	FSlateBrush CrosshairBrush;

	/** (射程) 枪械瞄准时的射线检测长度 */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float TraceDistance;
	/**
	 * 允许射线检测离准心多少， (偏离准心仍算击中目标)
	 */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float TraceCenterOffset;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Weapon")
	UCurveFloat* AimCurve;
	
	/** 开始衰减的距离 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	UCurveFloat* DamageFalloffCurve;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil" )
	float MinRecoil;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil" )
	float MaxRecoil;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	float RecoveryRecoilDelay;

	/** 材质参数名 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	FName CrossSpreadMatParamName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	UCurveFloat* RecoilCurve;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	UCurveFloat* CameraOffsetCurve;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	UCurveFloat* RecoveryCurve;
	
	/**
	 * 	对相机应用后坐力的抖动时XY方向的Scale
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	FVector2D CameraOffsetScale;


	/** ================== weapon relevant anim ================== */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	int32 AnimID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	UAnimMontage* ShootAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	UAnimMontage* ReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	FCharacterMontage CharacterAnim;

	/*UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapon|Animation")
	FWeaponAnimAdjustData AnimAdjust;*/

	/** ================== weapon relevant anim ================== */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|VFX")
	FWeaponShootMuzzle WeaponMuzzleFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Muzzle")
	FVector MuzzleLocationOffset;
	UPROPERTY(EditAnywhere, Category = "Weapon|Muzzle")
	FName MuzzleSocketName;

	/**
	 * 造成伤害应用到 FGameplayEffectSpec 中的 DynamicAssetTags
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ability")
	FGameplayTagContainer DynamicAssetTags;
	/**
	* 造成伤害应用到 FGameplayEffectSpec 中的 DynamicGrantedTags
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ability")
	FGameplayTagContainer DynamicGrantedTags;
	
public:
	
	UFUNCTION(BlueprintCallable, Category = Weapon)
	class ADCharacterPlayer* GetOwningShooter() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	class ADMPlayerController* GetOwningController() const;
	
	UFUNCTION(BlueprintCallable, Category = Weapon)
	float GetCurrentRecoil() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	float GetRemainAmmoPercent() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool IsLocalWeapon() const;

	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnStartFire", DisplayName = "OnStartFire"), Category = "Weapon|Event")
	void BP_OnStartFire();
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnFiring", DisplayName = "OnFiring"), Category = "Weapon|Event")
	void BP_OnFiring();
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnStopFire", DisplayName = "OnStopFire"), Category = "Weapon|Event")
	void BP_OnStopFire();
	
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnWeaponAimed", DisplayName = "OnWeaponAimed"), Category = "Weapon|Event")
	void BP_OnWeaponAimed();
	
	UFUNCTION(BlueprintImplementableEvent, meta = (ScriptName = "OnWeaponEnable", DisplayName = "OnWeaponEnable"), Category = "Weapon|Event")
	void BP_OnWeaponEnable(bool bEnable);
	
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual void GetMuzzlePoint(FVector& Point, FRotator& Direction) const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void GetOwningPlayerCameraViewPoint(FVector& Location, FRotator& Rotation);

public:

	virtual FTransform GetPreviewRelativeTransform() const override;

	virtual UDMUpgradeGearInfluence* GetUpgradeAttributesInfluence() const override;

	const FSlateBrush& GetDynamicCrosshairBrush();

	virtual void SetWeaponAim(bool NewAimed);

	virtual void SetWeaponEnable(bool bEnable);

	float GetFireInterval() const
	{
		return FireInterval;
	}

	EWeaponState GetWeaponState() const;

	bool IsState(EWeaponState State) const;

	bool CanFire() const;

	bool CanReload() const;

	void AttachToCharacter(bool bActiveSocket, USkeletalMeshComponent* Mesh);

	void HandleFire();
	void HandleFireStop();
	
	void HandleLineTrace(FHitResult& OutHit) const;

	virtual void GetActualProjectileHitResult(const FHitResult& ViewHitResult, FHitResult& ActualHitResult) const;

	virtual void MakeHitInfoHandle(FHitInfoHandle& HitInfo) PURE_VIRTUAL(AShootWeapon::MakeHitInfoHandle, );

	UFUNCTION(Reliable, Server)
	void ServerSpawnProjectile(const FHitInfoHandle& HitInfo);
	void SpawnProjectile();

	virtual void HandleSpawnProjectile(const FHitResult& HitResult) PURE_VIRTUAL(AShootWeapon::HandleSpawnProjectile, );

	virtual void ApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin) override;

	// ServerOnly

	void InitializeAmmunition();

	UFUNCTION(Unreliable, Server)
	void ServerSetWeaponState(EWeaponState NewState);
	void SetWeaponState(EWeaponState NewState);

	void PlayWeaponShootingAnim();
	void PlayWeaponReloadingAnim();
	
	void OnReloadFinished();

	
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void SpreadRecovery();

	int32 GetInitAmmoNum() const;
	int32 GetInitReserveAmmoNum() const;

	/**
	 * 初始化武器属性
	 */
	void InitializeWeaponAttributes(const FEquipmentAttributes& InAttributes, float AdditionMagnitude);

	const FEquipmentAttributes& GetAttributes() const;

protected:
	
	UFUNCTION()
	virtual void OnAimEnded();

	virtual void RecoilTimelineTick(float Value);
	virtual void RecoverTimelineTick(float Value);
	virtual void CameraOffsetTimelineTick(float Value) const;
	virtual void AimTimelineTick(float Value) const;

	void OnPlayerAmmunitionChanged(EAmmoType ChangedAmmoType, float NewAmmunition);

	UFUNCTION()	
    void OnRep_WeaponState();

	UFUNCTION()
    void OnRep_HitInfoHandle();

	virtual void OnRep_AttachmentReplication() override;

protected:

	UPROPERTY()
	ADCharacterPlayer* OwningShooter;

	/** 武器属性 */
	UPROPERTY()
	FEquipmentAttributes WeaponAttribute;

	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	bool bAimed;

	UPROPERTY(ReplicatedUsing = OnRep_HitInfoHandle)
	FHitInfoHandle HitInfoHandle;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;
	EWeaponState LastWeaponState;

	float CurrentSpread;

	FTimeline RecoveryTimeline;
	FTimeline RecoilTimeline;
	FTimeline AimTimeline;

	int32 InitAmmo;
	int32 InitReserveAmmo;

	float FireInterval;

private:

	/** 最后一次开火时间点 */
	float LastFireTime;

	FTimerHandle Handle_ReSpread;
	
	FTimerHandle Handle_Reloading;

	FDelegateHandle Handle_PlayerAmmunitionChange;

	UPROPERTY()
	UMaterialInstanceDynamic* CrosshairDynamic;
	UPROPERTY()
	UMaterialInstanceDynamic* MagazineDynamic;
	
};


