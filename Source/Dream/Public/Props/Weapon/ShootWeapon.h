// ReSharper disable All
#pragma once

#include "CoreMinimal.h"


#include "DreamGameplayPerk.h"
#include "DreamType.h"
#include "PropsInterface.h"
#include "SlateBrush.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "AbilitySystemGlobals.h"
#include "Components/TimelineComponent.h"
#include "Projectile/DProjectile.h"
#include "AbilitySystemGlobals.h"
#include "ShootWeapon.generated.h"

struct FBulletHitInfoStructContainer;
static const FBulletHitInfoStructContainer& StructContainer();

USTRUCT()
struct FBulletHitInfo
{
	GENERATED_BODY()

public:
	virtual ~FBulletHitInfo() = default;

	virtual uint8 GetStructID() const
	{
		return 0;	
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

/*public:

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize10 LocalMuzzleLocation;*/
	
};

template<>
struct TStructOpsTypeTraits<FBulletHitInfo> : public TStructOpsTypeTraitsBase2<FBulletHitInfo>
{
	enum
	{
		WithNetSerializer = true
    };
};

USTRUCT()
struct FBulletHitInfo_SingleBullet : public FBulletHitInfo
{
	GENERATED_BODY()

public:

	void SetHitResult(const FHitResult& NewHitResult)
	{
		HitResult = TSharedPtr<FHitResult>(new FHitResult(NewHitResult));
	}

	FHitResult* GetHitResult() const
	{
		return HitResult.Get();
	}
	
	virtual uint8 GetStructID() const override
	{
		return 1;
	}
	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

private:

	TSharedPtr<FHitResult> HitResult;
};


template<>
struct TStructOpsTypeTraits<FBulletHitInfo_SingleBullet> : public TStructOpsTypeTraitsBase2<FBulletHitInfo_SingleBullet>
{
	enum
	{
		WithNetSerializer = true
    };
};

USTRUCT()
struct FBulletHitInfo_MultiBullet : public FBulletHitInfo
{
	GENERATED_BODY()

public:
	
	virtual uint8 GetStructID() const override
	{
		return 2;
	}
	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	const TArray<FHitResult>& GetHits() const
	{
		return Hits;
	}

	void SetHits(const TArray<FHitResult>& NewHits)
	{
		Hits = NewHits;
	}

private:

	UPROPERTY()
	TArray<FHitResult> Hits;
};

template<>
struct TStructOpsTypeTraits<FBulletHitInfo_MultiBullet> : public TStructOpsTypeTraitsBase2<FBulletHitInfo_MultiBullet>
{
	enum
	{
		WithNetSerializer = true
    };
};

USTRUCT()
struct FBulletHitInfoHandle
{
	GENERATED_BODY()

public:
	FBulletHitInfoHandle()
	{
	}

	FBulletHitInfoHandle(FBulletHitInfo* DataPtr)
	{
		Data = TSharedPtr<FBulletHitInfo>(DataPtr);
	}

	

	template<typename Type>
    Type* Get() const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

private:

	TSharedPtr<FBulletHitInfo> Data;
};

template<>
struct TStructOpsTypeTraits<FBulletHitInfoHandle> : public TStructOpsTypeTraitsBase2<FBulletHitInfoHandle>
{
	enum
	{
		WithNetSerializer = true
    };
};

struct FBulletHitInfoStructContainer
{

public:

	FBulletHitInfoStructContainer()
	{
		ScriptStructCache.Add(0, FBulletHitInfo::StaticStruct());
		ScriptStructCache.Add(1, FBulletHitInfo_SingleBullet::StaticStruct());
		ScriptStructCache.Add(2, FBulletHitInfo_MultiBullet::StaticStruct());
    }

	UScriptStruct* GetScriptStruct(uint8 ID) const
	{
		UScriptStruct* const* ScriptStructPtr = ScriptStructCache.Find(ID);
		return ScriptStructPtr ? *ScriptStructPtr : nullptr;
	}

private:

	TMap<uint8, UScriptStruct*> ScriptStructCache;
};

static const FBulletHitInfoStructContainer& StructContainer()
{
	static FBulletHitInfoStructContainer StructContainer;
	return StructContainer;
}


UENUM()
enum class EFireMode : uint8
{
	SemiAutomatic UMETA(DisplayName="半自动"),
	FullyAutomatic UMETA(DisplayName="全自动"),
	Accumulation UMETA(DisplayName="蓄力")
};

USTRUCT(BlueprintType)
struct FWeaponShootMuzzle
{
	GENERATED_USTRUCT_BODY()

public:

	FWeaponShootMuzzle() :
        Particles(nullptr),
        Sound(nullptr),
        OffsetLocation(FVector::ZeroVector),
        Size(FVector::OneVector)
	{
	};

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Particles;
	UPROPERTY(EditAnywhere)
	class USoundBase* Sound;
	UPROPERTY(EditAnywhere)
	FVector OffsetLocation;
	UPROPERTY(EditAnywhere)
	FVector Size;

	FORCEINLINE bool IsValid() const
	{
		return Particles != nullptr && Sound != nullptr;
	}
};

USTRUCT(BlueprintType)
struct FWeaponAttribute
{
	GENERATED_BODY()

	/**
	 * 攻击力
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=WeaponAttr)
	int32 AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=WeaponAttr)
	float CriticalDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=WeaponAttr)
	float CriticalRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=WeaponAttr)
	float HealthSteal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=WeaponAttr)
	TArray<TSubclassOf<UDreamGameplayPerk>> WeaponPerks;
};

USTRUCT(BlueprintType)
struct FWeaponAnimAdjustData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector RHandIKEffectorLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector RHandIKJointTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform LeftHandIKEffectorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator LHandRotation;
};

UCLASS(Abstract)
class DREAM_API AShootWeapon : public AActor, public IPropsInterface
{
	GENERATED_BODY()

public:

	AShootWeapon();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	int32 WeaponID;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FPropsInfo WeaponInfo;

	/**
	 * 武器射击一次发射的弹片数量
	 */
	UPROPERTY(EditDefaultsOnly, Category=Weapon, meta = (ClampMin = 1, ClampMax = 20))
	int32 ShellFragment;
	
	/**
	 * 武器开火模式
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	EFireMode FireMode;

	/** 每分钟射速 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	int32 RateOfFire;

	/** 每个弹夹的子弹数量 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	int32 AmmoNum;
	
	/** 弹药类型 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	EAmmoType AmmoType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapon|Anim")
	bool bUseFABRIK;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapon|Anim", meta = (EditCondition = "bUseFABRIK"))
	FTransform FABRIKEffectorTransform;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapon|Anim", meta = (EditCondition = "bUseFABRIK"))
	FTransform FABRIKEffectorTransformIdle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FWeaponAnimAdjustData AnimAdjust;
	
	/** 武器属性 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FWeaponAttribute WeaponAttribute;
	
	/** 瞄准时的摄像机FOV */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	float AimFOV;

	/** 武器的瞄准速度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	float AimSpeed;

	/** 瞄准时的精准度阈值 数值越大越精准 为0时表示完全没有偏移 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	float AccuracyThreshold;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FTransform WeaponSocketOffset;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FTransform WeaponRelaxSocketOffset;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FTransform LeftSocketOffset;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	FTransform RightSocketOffset;

	/** WeaponUI */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|UI")
	FSlateBrush CrosshairBrush;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|UI")
	FSlateBrush MagazineBrush;

	/** (射程) 枪械瞄准时的射线检测长度 */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float TraceDistance;
	/**
	 * 允许射线检测离准心多少， (偏离准心仍算击中目标)
	 */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float TraceCenterOffset;
	
	/** 开始衰减的距离 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Weapon)
	UCurveFloat* DamageFalloffCurve;

	UPROPERTY(EditAnywhere, Category = Weapon)
	FName MuzzleSocketName;

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
	UPROPERTY()
	UCurveFloat* AimCurve;
	
	/**
	 * 	对相机应用后坐力的抖动时XY方向的强度
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon|Recoil")
	FVector2D CameraOffsetScale;


	/** ================== weapon relevant anim ================== */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	UAnimMontage* ShootAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	UAnimMontage* ReloadAnim;
	/** ================== weapon relevant anim ================== */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	FWeaponShootMuzzle WeaponMuzzleFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	TEnumAsByte<EWeaponAnimGroup::Type> AnimGroup;

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
	float GetCurrentRecoil() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	float GetRemainAmmoPercent() const;

	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetMuzzlePoint", DisplayName = "GetMuzzlePoint"), Category = Weapon)
	void BP_GetMuzzlePoint(FVector& Point, FRotator& Direction) const;
	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetLineTracePoint", DisplayName = "GetLineTracePoint"), Category = Weapon)
	void BP_GetLineTracePoint(FVector& Point, FRotator& Direction) const;

	/*
		判断当前武器 是否属于网络模式下的本地玩家
		isLocalPlayer  and  NetMode > NM_Standalone
	*/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool IsNetLocalPlayerCtrl() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool IsLocalPlayerCtrl() const;

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
	
public:

	FPropsInfo GetPropsInfo_Implementation() const override;

	// ServerOnly
	virtual void ApplyPointDamage(const FHitResult& HitInfo);
	virtual void ApplyRadialDamage(const FRadialDamageProjectileInfo& RadialDamage);

	FSlateBrush GetDynamicCrosshairBrush();
	FSlateBrush GetDynamicMagazineBrush();

	virtual void SetWeaponAim(bool NewAimed);

	virtual void SetWeaponEnable(bool bEnable);

	void DecrementTotalAmmo(int32 Amount) const;

	FORCEINLINE float GetFireInterval() const
	{
		return 1.f / (RateOfFire / 60.f);
	}

	FORCEINLINE float GetLastFireTimeSeconds() const
	{
		return LastFireTime;
	}

protected:

	float CurrentSpread;

	FTimeline RecoveryTimeline;
	FTimeline RecoilTimeline;
	FTimeline AimTimeline;

	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	bool bAimed;

protected:

	void HandleFire();
	void HandleLineTrace(const FVector& ViewLoc, const FRotator& ViewRot, FHitResult& OutHit) const;
	
	UFUNCTION(Reliable, Server)
	void ServerApplyRadialDamage(const FRadialDamageProjectileInfo& RadialDamage);

	void SpawnAmmo(const FBulletHitInfoHandle& HitInfo);
	UFUNCTION(Reliable, NetMulticast)
	void NetMulticastSpawnAmmo(const FBulletHitInfoHandle& HitInfo);
	UFUNCTION(Reliable, Server)
	void ServerSpawnAmmo(const FBulletHitInfoHandle& HitInfo);

	virtual void HandleSpawnAmmo(const FHitResult& HitResult) {};

	void DoApplyDamageEffect(const FHitResult& Hit, const FVector& Origin) const;
	//void DoApplyDamageEffectMulti(const TArray<FHitResult>& Hits);

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SpreadRecovery();
	virtual void GetLineTracePoint(FVector& Point, FRotator& Direction) const;

	virtual void GetMuzzlePoint(FVector& Point, FRotator& Direction) const;


	UFUNCTION()
	virtual void OnAimEnded();

	// 记录默认值
	int32 DAmmoNum;

private:

	friend class ADCharacterPlayer;
	friend class ADProjectile;
	friend class ADPlayerShooter;

	void RecoilTimelineTick(float Value);
	void CameraOffsetTimelineTick(float Value) const;
	void RecoverTimelineTick(float Value);
	void AimTimelineTick(float Value) const;

	/** 最后一次开火时间点 */
	float LastFireTime;

	FTimerHandle Handle_ReSpread;

	UPROPERTY()
	UMaterialInstanceDynamic* CrosshairDynamic;
	UPROPERTY()
	UMaterialInstanceDynamic* MagazineDynamic;
	
};
