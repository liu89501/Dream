#include "Props/Weapon/ShootWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DGameplayStatics.h"
#include "DGE_WeaponBaseDamage.h"
#include "DMPlayerController.h"
#include "DMProjectSettings.h"
#include "Engine.h"
#include "DreamType.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "Character/DCharacterPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "DreamGameplayType.h"

#define BulletHitInfo_ID_Bits 4

uint8 FBulletHitInfo::GetStructID() const
{
	return 0;
}

bool FBulletHitInfo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	return true;
}

bool FBulletHitInfo_SingleBullet::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	//Super::NetSerialize(Ar, Map, bOutSuccess);

	uint8 Verify = 0;

	if (Ar.IsSaving())
	{
		if (HitResult.IsValid())
		{
			Verify = 1;
		}
	}

	Ar.SerializeBits(&Verify, 1);

	if ((Verify & 1) == 1)
	{
		if (!HitResult.IsValid())
		{
			HitResult = MakeShared<FHitResult>();
		}
		
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}

	bOutSuccess = true;
	return true;
}

bool FBulletHitInfo_MultiBullet::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	//Super::NetSerialize(Ar, Map, bOutSuccess);

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
		SafeNetSerializeTArray_WithNetSerialize<16>(Ar, Hits, Map);
	}

	bOutSuccess = true;
	return true;
}

TSharedPtr<FBulletHitInfo> GetBulletHitInfoFromStructID(uint8 StructID)
{
	switch (StructID)
	{
		case 1: return MakeShared<FBulletHitInfo_SingleBullet>();
		case 2: return MakeShared<FBulletHitInfo_MultiBullet>();
		default: return nullptr;
	}
}

template <typename Type>
Type* FBulletHitInfoHandle::Get() const
{
	if (Data.IsValid())
	{
		if (Data->StaticStruct() == Type::StaticStruct())
		{
			return (Type*)Data.Get();
		}
	}

	return nullptr;
}

bool FBulletHitInfoHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 bValidData = Data.IsValid();
	Ar.SerializeBits(&bValidData, 1);

	if (bValidData == 1)
	{
		uint8 StructID = 0;
		if (Ar.IsSaving())
		{
			StructID = Data->GetStructID();
		}
		
		Ar.SerializeBits(&StructID, BulletHitInfo_ID_Bits);

		if (Ar.IsLoading())
		{
			if (!Data.IsValid())
			{
				Data = GetBulletHitInfoFromStructID(StructID);
			}
		}

		Data->NetSerialize(Ar, Map, bOutSuccess);
	}

	return bOutSuccess;
}

AShootWeapon::AShootWeapon():
	ShellFragment(1),
	RateOfFire(600),
	AccuracyThreshold(300.f),
	TraceDistance(20000.f),
	TraceCenterOffset(2.f),
	MinRecoil(30.f),
	MaxRecoil(90.f),
	RecoveryRecoilDelay(.25f),
	CrossSpreadMatParamName(TEXT("Spread")),
	CameraOffsetScale(FVector2D(0, 1.f)),
	MuzzleSocketName(TEXT("Muzzle")),
	LastFireTime(0)

{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	WeaponMesh->SetCollisionResponseToChannel(Collision_ObjectType_Projectile, ECR_Ignore);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	WeaponMesh->SetReceivesDecals(false);
}

void AShootWeapon::BeginPlay()
{
	Super::BeginPlay();

	OwningShooter = Cast<ADCharacterPlayer>(GetOwner());

	ensureMsgf(OwningShooter, TEXT("OwningShooter Invalid"));

	if (!IsLocalWeapon())
	{
		return;
	}

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

	if (AimCurve)
	{
		FOnTimelineFloatStatic AimTickDelegate;
		AimTickDelegate.BindUObject(this, &AShootWeapon::AimTimelineTick);
		AimTimeline.AddInterpFloat(AimCurve, AimTickDelegate);

		FOnTimelineEvent OnEnded;
		OnEnded.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(AShootWeapon, OnAimEnded));
		AimTimeline.AddEvent(AimCurve->FloatCurve.GetLastKey().Time, OnEnded);
	}

	InitializeAmmunition();

	SetActorTickEnabled(true);

	if (ADMPlayerController* OwningController = GetOwningController())
	{
		Handle_PlayerAmmunitionChange = OwningController->GetAmmunitionChangeDelegate().AddUObject(this, &AShootWeapon::OnPlayerAmmunitionChanged);
	}
}

void AShootWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (Handle_PlayerAmmunitionChange.IsValid())
	{
		if (OwningShooter && !OwningShooter->IsPendingKillPending())
		{
			ADMPlayerController* Controller = OwningShooter->GetPlayerController();

			if (Controller && !Controller->IsPendingKillPending())
			{
				Controller->GetAmmunitionChangeDelegate().Remove(Handle_PlayerAmmunitionChange);
			}
		}
	}
}

void AShootWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitAmmo = AmmoNum;
	InitReserveAmmo = ReserveAmmo;

	FireInterval = 1.f / (RateOfFire / 60.f);

	TArray<USceneComponent*> ChildrenComps;
	WeaponMesh->GetChildrenComponents(true, ChildrenComps);

	for (USceneComponent* Child : ChildrenComps)
	{
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Child))
		{
			Primitive->SetReceivesDecals(false);
		}
	}
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
		Shooter->SetCameraFieldOfView(FMath::LerpStable(90.f, AimFOV, Value));
		Shooter->CameraAimTransformLerp(Value);
	}
}

float AShootWeapon::GetCurrentRecoil() const
{
	return CurrentSpread;
}

void AShootWeapon::GetLineTracePoint(FVector& Point, FRotator& Direction) const
{
	if (OwningShooter && OwningShooter->Controller)
	{
		OwningShooter->Controller->GetPlayerViewPoint(Point, Direction);
	}
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

	TSharedPtr<FBulletHitInfo> HitInfoPtr;

	if (ShellFragment > 1)
	{
		TArray<FHitResult> Hits;
		Hits.AddDefaulted(ShellFragment);
		
		for (int32 I = 0; I < ShellFragment; I++)
		{
			HandleLineTrace(ViewLoc, ViewRot, Hits[I]);
		}

		HitInfoPtr = MakeShared<FBulletHitInfo_MultiBullet>(Hits);
	}
	else
	{
		FHitResult Hit;
		HandleLineTrace(ViewLoc, ViewRot, Hit);
		
		HitInfoPtr = MakeShared<FBulletHitInfo_SingleBullet>(Hit);
	}

	FBulletHitInfoHandle Handle(HitInfoPtr);

	// 增加后坐力
	RecoilTimeline.PlayFromStart();

	BP_OnFiring();

	SpawnAmmoAndFX(Handle);

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

	GetWorld()->LineTraceSingleByChannel(OutHit, ViewLoc, TraceEnd, ECC_Visibility, QueryParams);

	OutHit.TraceStart = WeaponMesh->GetSocketLocation(MuzzleSocketName);

	if (!OutHit.bBlockingHit)
	{
		OutHit.ImpactPoint = TraceEnd;
		OutHit.Location = TraceEnd;
	}
}

void AShootWeapon::SpawnAmmoAndFX(const FBulletHitInfoHandle& HitInfo)
{
	if (FBulletHitInfo_SingleBullet* SingleBullet = HitInfo.Get<FBulletHitInfo_SingleBullet>())
	{
		if (FHitResult* HitResult = SingleBullet->GetHitResult())
		{
			HandleSpawnAmmo(*HitResult);
		}
	}
	else if (FBulletHitInfo_MultiBullet* MultiBullet = HitInfo.Get<FBulletHitInfo_MultiBullet>())
	{
		const TArray<FHitResult>& HitResults = MultiBullet->GetHits();
		for (const FHitResult& HitResult : HitResults)
		{
			HandleSpawnAmmo(HitResult);
		}
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		ServerSpawnAmmo(HitInfo);
	}
	else
	{
		if (OwningShooter)
		{
			OwningShooter->PlayMontage(CharacterAnim.ShootAnim);
			PlayWeaponShootingAnim();
		}

		// 枪口的世界位置
		FVector MuzzleLoc;
		FRotator MuzzleRot;
		GetMuzzlePoint(MuzzleLoc, MuzzleRot);

		UGameplayStatics::SpawnSoundAtLocation(this, WeaponMuzzleFX.Sound,
                                               WeaponMesh->GetSocketLocation(MuzzleSocketName));
		
		UGameplayStatics::SpawnEmitterAtLocation(this, WeaponMuzzleFX.Particles, MuzzleLoc, MuzzleRot,
                                                 WeaponMuzzleFX.Size, true, EPSCPoolMethod::AutoRelease);
	}
}

void AShootWeapon::ServerSpawnAmmo_Implementation(const FBulletHitInfoHandle& HitInfo)
{
	HitInfoHandle = HitInfo;
}

// [ROLE_Simulation]
void AShootWeapon::OnRep_HitInfoHandle()
{
	SpawnAmmoAndFX(HitInfoHandle);
}

FTransform AShootWeapon::GetPreviewRelativeTransform() const
{
	return PreviewTransform;
}

void AShootWeapon::GetMuzzlePoint(FVector& Point, FRotator& Direction) const
{
	Direction = WeaponMesh->GetSocketRotation(MuzzleSocketName);
	Point = WeaponMesh->GetSocketLocation(MuzzleSocketName) + Direction.RotateVector(MuzzleLocationOffset);
}

void AShootWeapon::OnAimEnded()
{
	BP_OnWeaponAimed();
}

void AShootWeapon::ApplyRadialDamage(const FRadialDamageProjectileInfo& Radial)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, Radial.Origin, Radial.Origin, FQuat::Identity,
                                    ECC_Visibility, FCollisionShape::MakeSphere(Radial.DamageRadius), QueryParams);


	for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(Hits); It; ++It)
	{
		// 这里主要作用是触发引擎里默认的一些效果。 像是对击中的物体添加一个力
		//(*It).Actor->TakeDamage(DNUMBER_ONE, DmgEvent, nullptr, GetOwner());

		DoApplyDamageEffect(*It, Radial.Origin);
	}
}

void AShootWeapon::ApplyPointDamage(const FHitResult& HitInfo)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	//UGameplayStatics::ApplyPointDamage(HitInfo.GetActor(), DNUMBER_ONE, HitInfo.ImpactNormal, HitInfo, nullptr, GetOwner(), nullptr);

	DoApplyDamageEffect(HitInfo, GetOwner()->GetActorLocation());
}

void AShootWeapon::InitializeAmmunition()
{
	if (ADMPlayerController* OwningController = GetOwningController())
	{
		float AmmunitionPercentage = OwningController->GetWeaponAmmunition(AmmoType);

		int32 AvailableAmmo = FMath::RoundHalfFromZero(InitReserveAmmo * AmmunitionPercentage);

		AmmoNum = FMath::Clamp(AvailableAmmo, 0, InitAmmo);
		ReserveAmmo = AvailableAmmo - AmmoNum;
	}
}

void AShootWeapon::OnPlayerAmmunitionChanged(float NewAmmunition)
{
	ReserveAmmo = FMath::RoundHalfFromZero(InitReserveAmmo * NewAmmunition);
}

void AShootWeapon::ServerSetWeaponState_Implementation(EWeaponState NewState)
{
	SetWeaponState(NewState);
}

void AShootWeapon::SetWeaponState(EWeaponState NewState)
{
	// [local/server]
	
	WeaponState = NewState;
	
	if (GetLocalRole() != ROLE_Authority)
	{
		ServerSetWeaponState(NewState);
	}
}

void AShootWeapon::OnRep_WeaponState(EWeaponState LastState)
{
	if (LastState != EWeaponState::Idle)
	{
		if (WeaponMesh->AnimScriptInstance)
		{
			WeaponMesh->AnimScriptInstance->StopAllMontages(0.1f);
		}
	}
	
	if (WeaponState == EWeaponState::Reloading)
	{
		PlayWeaponReloadingAnim();
		OwningShooter->PlayMontage(CharacterAnim.ReloadAnim);
	}
	else if (WeaponState == EWeaponState::Equipping)
	{
		OwningShooter->PlayMontage(CharacterAnim.EquipAnim);
	}
}

void AShootWeapon::PlayWeaponShootingAnim()
{
	if (WeaponMesh->AnimScriptInstance)
	{
		WeaponMesh->AnimScriptInstance->Montage_Play(ReloadAnim);
	}
}

void AShootWeapon::PlayWeaponReloadingAnim()
{
	if (WeaponMesh->AnimScriptInstance)
	{
		WeaponMesh->AnimScriptInstance->Montage_Play(ShootAnim);
	}
}

void AShootWeapon::OnReloadFinished()
{
	int32 ReloadAmmo = FMath::Min(InitAmmo - AmmoNum, ReserveAmmo);
	
	AmmoNum += ReloadAmmo;

	ADMPlayerController* Controller = GetOwningShooter()->GetPlayerController();
	Controller->SetWeaponAmmunition(AmmoType, static_cast<float>(ReserveAmmo - ReloadAmmo) / InitReserveAmmo);
}

void AShootWeapon::DoApplyDamageEffect(const FHitResult& Hit, const FVector& Origin) const
{
	AActor* HitActor = Hit.GetActor();
	if (HitActor == nullptr)
	{
		return;
	}

	if (IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor))
	{
		float DamageFalloff = 1.f;
		if (DamageFalloffCurve)
		{
			DamageFalloff = DamageFalloffCurve->GetFloatValue(FVector::Distance(Origin, Hit.ImpactPoint));
		}

		FDreamGameplayEffectContext* EffectContext = UDGameplayStatics::MakeDreamEffectContext(OwningShooter, DamageFalloff, Hit);
		EffectContext->SetDamageType(static_cast<EDDamageType>(WeaponType));

		FGameplayEffectSpec Spec(GetDefault<UDGE_WeaponBaseDamage>(), FGameplayEffectContextHandle(EffectContext));
		Spec.DynamicAssetTags = DynamicAssetTags;
		Spec.DynamicGrantedTags = DynamicGrantedTags;
		TargetASI->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(Spec);
	}
}

void AShootWeapon::SetWeaponEnable(bool bEnable)
{
	if (IsLocalWeapon())
	{
		BP_OnWeaponEnable(bEnable);
		SetActorTickEnabled(bEnable);
		if (!bEnable)
		{
			RecoverTimelineTick(1.f);
			AimTimelineTick(0.f);
		}
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

EWeaponState AShootWeapon::GetWeaponState() const
{
	return WeaponState;
}

bool AShootWeapon::IsState(EWeaponState State) const
{
	return WeaponState == State;
}

bool AShootWeapon::CanFire() const
{
	return GetWorld()->GetTimeSeconds() > (LastFireTime + FireInterval);
}

bool AShootWeapon::CanReload() const
{
	return AmmoNum < InitAmmo && ReserveAmmo > 0 && WeaponState < EWeaponState::Reloading;
}

const FSlateBrush& AShootWeapon::GetDynamicCrosshairBrush()
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

float AShootWeapon::GetRemainAmmoPercent() const
{
	return static_cast<float>(AmmoNum) / InitAmmo;
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
	return OwningShooter;
}

ADMPlayerController* AShootWeapon::GetOwningController() const
{
	return OwningShooter ? OwningShooter->GetPlayerController() : nullptr;
}

void AShootWeapon::SpreadRecovery()
{
	RecoveryTimeline.PlayFromStart();
}

bool AShootWeapon::IsLocalWeapon() const
{
	if (GetNetMode() == NM_Standalone)
	{
		return true;
	}

	if (OwningShooter && OwningShooter->GetLocalRole() == ROLE_AutonomousProxy)
	{
		return true;
	}

	return false;
}

void AShootWeapon::AttachToCharacter(bool bActiveSocket, USkeletalMeshComponent* Mesh)
{
	UDMProjectSettings* ProjectSettings = UDMProjectSettings::GetProjectSettings();

	AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform,
	                  bActiveSocket
		                  ? ProjectSettings->GetWepActiveSockName()
		                  : ProjectSettings->GetWepHolsterSockName());

	SetActorRelativeTransform(bActiveSocket ? WeaponSocketOffset : WeaponHolsterSocketOffset);
}

void AShootWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShootWeapon, HitInfoHandle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AShootWeapon, WeaponState, COND_SkipOwner);
}
