#include "Props/Weapon/ShootWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DGameplayStatics.h"
#include "DGE_WeaponBaseDamage.h"
#include "Engine.h"
#include "DreamHUD.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Character/DCharacterPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DreamGameplayType.h"

bool FBulletHitInfo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    /*bool bValid = false;

    if (Ar.IsSaving())
    {
        if (!LocalMuzzleLocation.IsNearlyZero())
        {
            bValid = true;
        }
    }

    Ar.SerializeBits(&bValid, 1);

    if (bValid)
    {
        Ar << LocalMuzzleLocation;
    }*/
    
    bOutSuccess = true;
    return true;
}

bool FBulletHitInfo_SingleBullet::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    Super::NetSerialize(Ar, Map, bOutSuccess);
    
    uint8 Verify = 0;

    if (Ar.IsSaving())
    {
        if (HitResult.IsValid())
        {
            Verify = 1;
        }
    }

    Ar.SerializeBits(&Verify, 1);

    if (Verify)
    {
        if (Ar.IsLoading())
        {
            if (!HitResult.IsValid())
            {
                HitResult = MakeShared<FHitResult>();
            }
        }

        HitResult->NetSerialize(Ar, Map, bOutSuccess);
    }

    bOutSuccess = true;
    return true;
}

bool FBulletHitInfo_MultiBullet::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    Super::NetSerialize(Ar, Map, bOutSuccess);
    
    uint8 Verify = 0;

    if (Ar.IsSaving())
    {
        if (Hits.Num() > 0)
        {
            Verify = 1;
        }
    }

    Ar.SerializeBits(&Verify, 1);

    if (Verify)
    {
        SafeNetSerializeTArray_WithNetSerialize<20>(Ar, Hits, Map);
    }

    bOutSuccess = true;
    return true;
}

bool FBulletHitInfoHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    bool ValidData = Data.IsValid();
    Ar.SerializeBits(&ValidData, 1);

    if (ValidData)
    {
        uint8 StructID = 0;
        if (Ar.IsSaving())
        {
            StructID = Data->GetStructID();
        }
        Ar.SerializeBits(&StructID, 8);

        UScriptStruct* ScriptStruct = StructContainer().GetScriptStruct(StructID);
        
        if (Ar.IsLoading())
        {
            if (!Data.IsValid())
            {
                FBulletHitInfo* Ptr = static_cast<FBulletHitInfo*>(FMemory::Malloc(ScriptStruct->GetStructureSize()));
                ScriptStruct->InitializeStruct(Ptr);
                Data = TSharedPtr<FBulletHitInfo>(Ptr);
            }
        }

        if (ScriptStruct->StructFlags & STRUCT_NetSerializeNative)
        {
            ScriptStruct->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Data.Get());
        }
        else
        {
            DREAM_NLOG(Error, TEXT("FBulletHitInfoHandle::NetSerialize Error"));
        }
    }

    bOutSuccess = true;
    return true;
}

template <typename Type>
Type* FBulletHitInfoHandle::Get() const
{
    if (Data.IsValid())
    {
        if (Type::StaticStruct() == StructContainer().GetScriptStruct(Data->GetStructID()))
        {
            return static_cast<Type*>(Data.Get());
        }
    }
    return nullptr;
}

AShootWeapon::AShootWeapon():
    ShellFragment(1),
    RateOfFire(600),
    AimSpeed(.1f),
    AccuracyThreshold(300.f),
    TraceDistance(20000.f),
    TraceCenterOffset(2.f),
    MuzzleSocketName(TEXT("Muzzle")),
    MinRecoil(30.f),
    MaxRecoil(90.f),
    RecoveryRecoilDelay(.25f),
    CrossSpreadMatParamName(TEXT("Spread")),
    CameraOffsetScale(FVector2D(0, 1.f)),
    LastFireTime(0)

{
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    WeaponMesh->SetCollisionResponseToChannel(Collision_ObjectType_Projectile, ECollisionResponse::ECR_Ignore);

    if (IsRunningDedicatedServer())
    {
        PrimaryActorTick.bCanEverTick = false;
    }
    else
    {
        PrimaryActorTick.bCanEverTick = true;
        PrimaryActorTick.bStartWithTickEnabled = false;
    }

    bReplicates = true;
}

void AShootWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalPlayerCtrl())
    {
        CurrentSpread = MinRecoil;

        if (RecoveryCurve)
        {
            FOnTimelineFloatStatic ReSpreadDelegate;
            ReSpreadDelegate.BindUObject(this, &AShootWeapon::RecoverTimelineTick);
            RecoveryTimeline.AddInterpFloat(RecoveryCurve, ReSpreadDelegate);
        }

        if (RecoilCurve)
        {
            FOnTimelineFloatStatic SpreadDeltaDelegate;
            SpreadDeltaDelegate.BindUObject(this, &AShootWeapon::RecoilTimelineTick);
            RecoilTimeline.AddInterpFloat(RecoilCurve, SpreadDeltaDelegate);
        }

        if (CameraOffsetCurve)
        {
            FOnTimelineFloatStatic CameraRiseTickDelegate;
            CameraRiseTickDelegate.BindUObject(this, &AShootWeapon::CameraOffsetTimelineTick);
            RecoilTimeline.AddInterpFloat(CameraOffsetCurve, CameraRiseTickDelegate);
        }

        WeaponMesh->SetReceivesDecals(false);
        TArray<USceneComponent*> ChildrenComps;
        WeaponMesh->GetChildrenComponents(true, ChildrenComps);

        for (USceneComponent* Child : ChildrenComps)
        {
            if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Child))
            {
                Primitive->SetReceivesDecals(false);
            }
        }

        FRichCurve RichCurve;
        RichCurve.AddKey(0.f, 0.f);
        RichCurve.AddKey(AimSpeed, 1.f);

        AimCurve = NewObject<UCurveFloat>();
        AimCurve->FloatCurve = RichCurve;

        FOnTimelineFloatStatic AimTickDelegate;
        AimTickDelegate.BindUObject(this, &AShootWeapon::AimTimelineTick);
        AimTimeline.AddInterpFloat(AimCurve, AimTickDelegate);

        FOnTimelineEvent OnEnded;
        OnEnded.BindUFunction(this, TEXT("OnAimEnded"));
        AimTimeline.AddEvent(AimSpeed, OnEnded);

        DecrementTotalAmmo(AmmoNum);
    }
}

void AShootWeapon::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    DAmmoNum = AmmoNum;
}

void AShootWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RecoilTimeline.TickTimeline(DeltaTime);
    RecoveryTimeline.TickTimeline(DeltaTime);
    AimTimeline.TickTimeline(DeltaTime);
}

void AShootWeapon::RecoverTimelineTick(float Value)
{
    CurrentSpread = FMath::Lerp(CurrentSpread, MinRecoil, Value);
    if (CrosshairDynamic != nullptr)
    {
        CrosshairDynamic->SetScalarParameterValue(CrossSpreadMatParamName, CurrentSpread);
    }
}

void AShootWeapon::RecoilTimelineTick(float Value)
{
    CurrentSpread = FMath::Min(CurrentSpread + Value, MaxRecoil);
    if (CrosshairDynamic != nullptr)
    {
        CrosshairDynamic->SetScalarParameterValue(CrossSpreadMatParamName, CurrentSpread);
    }
}

void AShootWeapon::CameraOffsetTimelineTick(float Value) const
{
    float PitchDelta = Value * CameraOffsetScale.Y * GetWorld()->GetDeltaSeconds() * FMath::FRandRange(0.2f, 1.f);
    float YawDelta = Value * CameraOffsetScale.X * GetWorld()->GetDeltaSeconds() * FMath::FRandRange(-1.f, 1.f);

    if (ADCharacterPlayer* Shooter = GetOwningShooter())
    {
        Shooter->AddControllerYawInput(YawDelta);
        Shooter->AddControllerPitchInput(PitchDelta);
    }
}

void AShootWeapon::AimTimelineTick(float Value) const
{
    if (ADCharacterPlayer* Shooter = GetOwningShooter())
    {
        Shooter->TPCamera->SetFieldOfView(FMath::Lerp(90.f, AimFOV, Value));
        Shooter->TPCameraArm->SocketOffset = FMath::Lerp(Shooter->CameraArmOffset, Shooter->CameraArmAimOffset, Value);
    }
}

float AShootWeapon::GetCurrentRecoil() const
{
    return CurrentSpread;
}

void AShootWeapon::GetLineTracePoint(FVector& Point, FRotator& Direction) const
{
    GetOwningShooter()->Controller->GetPlayerViewPoint(Point, Direction);
}

void AShootWeapon::HandleFire()
{
    UWorld* World = GetWorld();

    LastFireTime = World->TimeSeconds;
    GetWorldTimerManager().SetTimer(Handle_ReSpread, this, &AShootWeapon::SpreadRecovery, RecoveryRecoilDelay, false);

    if (RecoveryTimeline.IsPlaying())
    {
        RecoveryTimeline.Stop();
    }

    FVector ViewLoc;
    FRotator ViewRot;
    GetLineTracePoint(ViewLoc, ViewRot);

    FBulletHitInfo* HitInfoPtr;

    if (ShellFragment > 1)
    {
        TArray<FHitResult> Hits;
        Hits.AddDefaulted(ShellFragment);
        for (int32 I = 0; I < ShellFragment; I++)
        {
            HandleLineTrace(ViewLoc, ViewRot, Hits[I]);
        }

        FBulletHitInfo_MultiBullet* Ptr = new FBulletHitInfo_MultiBullet;
        Ptr->SetHits(Hits);
        HitInfoPtr = Ptr;
    }
    else
    {
        FHitResult Hit;
        HandleLineTrace(ViewLoc, ViewRot, Hit);
        FBulletHitInfo_SingleBullet* Ptr = new FBulletHitInfo_SingleBullet;
        Ptr->SetHitResult(Hit);
        HitInfoPtr = Ptr;
    }

    FBulletHitInfoHandle Handle(HitInfoPtr);

    // 增加后坐力
    RecoilTimeline.PlayFromStart();

    BP_OnFiring();

    SpawnAmmo(Handle);
    ServerSpawnAmmo(Handle);

    AmmoNum--;
}

void AShootWeapon::HandleLineTrace(const FVector& ViewLoc, const FRotator& ViewRot, FHitResult& OutHit) const
{
    FRandomStream RandomStream(FMath::Rand());
    float AngleRadians = 0.f;

    if (bAimed)
    {
        if (AccuracyThreshold > 0)
        {
            AngleRadians = FMath::DegreesToRadians(CurrentSpread) / AccuracyThreshold;
        }
    }
    else
    {
        AngleRadians = FMath::DegreesToRadians(CurrentSpread) / 30;
    }

    FVector ShootDir = RandomStream.VRandCone(ViewRot.Vector(), AngleRadians, AngleRadians);
    FVector TraceEnd = ViewLoc + ShootDir * TraceDistance;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwningShooter());
    QueryParams.bReturnPhysicalMaterial = true;

    /*GetWorld()->SweepSingleByChannel(OutHit, ViewLoc, TraceEnd, FQuat::Identity,
                                     ECollisionChannel::ECC_Visibility,
                                     FCollisionShape::MakeSphere(TraceCenterOffset), QueryParams);*/

    GetWorld()->LineTraceSingleByChannel(OutHit, ViewLoc, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);

    OutHit.TraceStart = WeaponMesh->GetSocketLocation(MuzzleSocketName);
    
    if (!OutHit.bBlockingHit)
    {
        OutHit.ImpactPoint = TraceEnd;
        OutHit.Location = TraceEnd;
    }
}

FPropsInfo AShootWeapon::GetPropsInfo_Implementation() const
{
    return WeaponInfo;
}

void AShootWeapon::ApplyRadialDamage(const FRadialDamageProjectileInfo& Radial)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        ServerApplyRadialDamage(Radial);
    }
    else
    {
        ADCharacterPlayer* Shooter = GetOwningShooter();

        TArray<FHitResult> Hits;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Shooter);
        QueryParams.bReturnPhysicalMaterial = true;

        GetWorld()->SweepMultiByChannel(Hits, Radial.Origin, Radial.Origin, FQuat::Identity,
                                        ECollisionChannel::ECC_Visibility,
                                        FCollisionShape::MakeSphere(Radial.DamageRadius), QueryParams);

        for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(Hits); It; ++It)
        {
            DoApplyDamageEffect(*It, Radial.Origin);
        }
    }
}

void AShootWeapon::GetMuzzlePoint(FVector& Point, FRotator& Direction) const
{
    FVector MuzzleLoc = WeaponMesh->GetSocketLocation(MuzzleSocketName);
    Direction = WeaponMesh->GetSocketRotation(MuzzleSocketName);
    Point = MuzzleLoc + Direction.RotateVector(WeaponMuzzleFX.OffsetLocation);
}

void AShootWeapon::OnAimEnded()
{
    BP_OnWeaponAimed();
}

void AShootWeapon::SpawnAmmo(const FBulletHitInfoHandle& HitInfo)
{
    if (FBulletHitInfo_SingleBullet* SingleBullet = HitInfo.Get<FBulletHitInfo_SingleBullet>())
    {
        FHitResult* HitResult = SingleBullet->GetHitResult();
        if (HitResult)
        {
            HandleSpawnAmmo(*HitResult);
        }
    }
    else if (FBulletHitInfo_MultiBullet* MultiBullet = HitInfo.Get<FBulletHitInfo_MultiBullet>())
    {
        const TArray<FHitResult>& HitResults = MultiBullet->GetHits();
        for (FHitResult HitResult : HitResults)
        {
            HandleSpawnAmmo(HitResult);
        }
    }

    if (GetLocalRole() != ROLE_Authority)
    {
        if (ADCharacterPlayer* Shooter = GetOwningShooter())
        {
            const FMontageSet* MontageSet = Shooter->GetCurrentActionMontage();
            Shooter->PlayMontage(MontageSet->ShootAnim, ShootAnim);
        }

        // 枪口的世界位置
        FVector MuzzleLoc;
        FRotator MuzzleRot;

        GetMuzzlePoint(MuzzleLoc, MuzzleRot);

        UGameplayStatics::SpawnSoundAttached(WeaponMuzzleFX.Sound, WeaponMesh, MuzzleSocketName);
        UGameplayStatics::SpawnEmitterAttached(WeaponMuzzleFX.Particles, WeaponMesh, MuzzleSocketName,
                                               WeaponMuzzleFX.OffsetLocation, FRotator::ZeroRotator,
                                               WeaponMuzzleFX.Size,
                                               EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::AutoRelease);
    }
}

void AShootWeapon::ServerSpawnAmmo_Implementation(const FBulletHitInfoHandle& HitInfo)
{
    NetMulticastSpawnAmmo(HitInfo);
}

void AShootWeapon::ApplyPointDamage(const FHitResult& HitInfo)
{
    DoApplyDamageEffect(HitInfo, GetOwner()->GetActorLocation());
}

void AShootWeapon::ServerApplyRadialDamage_Implementation(const FRadialDamageProjectileInfo& Radial)
{
    ApplyRadialDamage(Radial);
}

void AShootWeapon::NetMulticastSpawnAmmo_Implementation(const FBulletHitInfoHandle& HitInfo)
{
    if (IsLocalPlayerCtrl())
    {
        return;
    }

    SpawnAmmo(HitInfo);
}

void AShootWeapon::DoApplyDamageEffect(const FHitResult& Hit, const FVector& Origin) const
{
    if (IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Hit.GetActor()))
    {
        ADCharacterPlayer* OwningShooter = GetOwningShooter();
        FDreamGameplayEffectContext* EffectContext = UDGameplayStatics::MakeDreamEffectContextHandle(
            OwningShooter, DamageFalloffCurve, Hit, Origin);
        //EffectContext->AddHitPoint(Hit.ImpactPoint);

        FGameplayEffectSpec Spec(GetDefault<UDGE_WeaponBaseDamage>(), FGameplayEffectContextHandle(EffectContext));
        Spec.DynamicAssetTags = DynamicAssetTags;
        Spec.DynamicGrantedTags = DynamicGrantedTags;
        TargetASI->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(Spec);
    }
}

/*void AShootWeapon::DoApplyDamageEffectMulti(const TArray<FHitResult>& Hits)
{
	const FHitResult& FirstResult = Hits[0];

	if (IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(FirstResult.GetActor()))
	{
		ADCharacterPlayer* OwningShooter = GetOwningShooter();

		FDreamGameplayEffectContext* EffectContext = UDGameplayStatics::MakeDreamEffectContextHandle(
            OwningShooter, DamageFalloffCurve, FirstResult, OwningShooter->GetActorLocation());

		for (FHitResult Hit : Hits)
		{
			EffectContext->AddHitPoint(Hit.ImpactPoint);
		}
		
		FGameplayEffectSpec Spec(GetDefault<UDGE_WeaponBaseDamage>(), FGameplayEffectContextHandle(EffectContext));
		Spec.DynamicAssetTags = DynamicAssetTags;
		Spec.DynamicGrantedTags = DynamicGrantedTags;
		TargetASI->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(Spec);
	}
}*/

void AShootWeapon::SetWeaponEnable(bool bEnable)
{
    BP_OnWeaponEnable(bEnable);
    SetActorTickEnabled(bEnable);
    if (!bEnable)
    {
        RecoverTimelineTick(1.f);
        AimTimelineTick(0.f);
    }
}

void AShootWeapon::SetWeaponAim(bool NewAimed)
{
    bAimed = NewAimed;

    if (bAimed)
    {
        AimTimeline.PlayFromStart();
    }
    else
    {
        AimTimeline.Reverse();
    }
}


FSlateBrush AShootWeapon::GetDynamicCrosshairBrush()
{
    if (UMaterialInterface* Material = Cast<UMaterialInterface>(CrosshairBrush.GetResourceObject()))
    {
        if (!CrosshairDynamic)
        {
            CrosshairDynamic = UMaterialInstanceDynamic::Create(Material, this);
            CrosshairBrush.SetResourceObject(CrosshairDynamic);
        }

        CrosshairDynamic->SetScalarParameterValue(CrossSpreadMatParamName, MinRecoil);
    }

    return CrosshairBrush;
}

FSlateBrush AShootWeapon::GetDynamicMagazineBrush()
{
    if (!MagazineDynamic)
    {
        if (UMaterialInterface* Material = Cast<UMaterialInterface>(MagazineBrush.GetResourceObject()))
        {
            MagazineDynamic = UMaterialInstanceDynamic::Create(Material, this);
            MagazineBrush.SetResourceObject(MagazineDynamic);
        }
    }

    return MagazineBrush;
}

float AShootWeapon::GetRemainAmmoPercent() const
{
    return static_cast<float>(AmmoNum) / DAmmoNum;
}

void AShootWeapon::BP_GetMuzzlePoint(FVector& Point, FRotator& Direction) const
{
    GetMuzzlePoint(Point, Direction);
}

void AShootWeapon::BP_GetLineTracePoint(FVector& Point, FRotator& Direction) const
{
    GetLineTracePoint(Point, Direction);
}

ADCharacterPlayer* AShootWeapon::GetOwningShooter() const
{
    return Cast<ADCharacterPlayer>(GetOwner());
}

void AShootWeapon::SpreadRecovery()
{
    RecoveryTimeline.PlayFromStart();
}

bool AShootWeapon::IsNetLocalPlayerCtrl() const
{
    if (ADCharacterPlayer* Shooter = GetOwningShooter())
    {
        return Shooter->IsLocallyControlled() && GetNetMode() > NM_Standalone;
    }

    return false;
}

bool AShootWeapon::IsLocalPlayerCtrl() const
{
    if (ADCharacterPlayer* Shooter = GetOwningShooter())
    {
        return Shooter->IsLocallyControlled();
    }

    return false;
}

void AShootWeapon::DecrementTotalAmmo(int32 Amount) const
{
    if (ADPlayerController* PC = GetOwningShooter()->GetPlayerController())
    {
        PC->DecrementAmmunition(AmmoType, Amount);
    }
}

void AShootWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
