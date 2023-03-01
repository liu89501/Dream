#include "Props/Weapon/ShootWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DGameplayStatics.h"
#include "DMEffect_ShootingDamage.h"
#include "DMPlayerController.h"
#include "DMProjectSettings.h"
#include "DMUpgradeGearInfluence.h"
#include "Engine.h"
#include "DreamType.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "Character/DCharacterPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "DreamGameplayType.h"

#if ENABLE_WEAPON_DEBUG

TAutoConsoleVariable<int32> DebugWeaponCVar::DebugCVar(
    TEXT("p.dm.WeaponTrace"),
    0,
    TEXT("Whether to draw debug information.\n")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Cheat);

#endif

bool FHitInfo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	return true;
}

TSharedPtr<FHitInfo> FHitInfo::Clone() const
{
	return nullptr;
}

bool FHitInfo_SingleBullet::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
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

TSharedPtr<FHitInfo> FHitInfo_SingleBullet::Clone() const
{
	return MakeShared<FHitInfo_SingleBullet>(*this);
}

bool FHitInfo_MultiBullet::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
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

TSharedPtr<FHitInfo> FHitInfo_MultiBullet::Clone() const
{
	return MakeShared<FHitInfo_MultiBullet>(*this);
}

template <typename Type>
Type* FHitInfoHandle::Get() const
{
	if (Data.IsValid())
	{
		if (Data->GetScriptStruct() == Type::StaticStruct())
		{
			return static_cast<Type*>(Data.Get());
		}
	}

	return nullptr;
}

bool FHitInfoHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 bValidData = Data.IsValid() ? 1 : 0;
	
	Ar << bValidData;

	bOutSuccess = bValidData == 1;

	if (bOutSuccess)
	{
		UScriptStruct* ScriptStruct = nullptr;
		
		if (Ar.IsSaving())
		{
			ScriptStruct = Data->GetScriptStruct();
		}
		
		Ar << ScriptStruct;

		if (Ar.IsLoading())
		{
			if (ScriptStruct)
			{
				FHitInfo* HitInfo = static_cast<FHitInfo*>(FMemory::Malloc(ScriptStruct->GetStructureSize()));
				ScriptStruct->InitializeStruct(HitInfo);
				Data = TSharedPtr<FHitInfo>(HitInfo);
			}
			else
			{
				bOutSuccess = false;
				UE_LOG(LogDream, Error, TEXT("HitInfo ScriptStruct Invalid"));
			}
		}

		if (bOutSuccess)
		{
			Data->NetSerialize(Ar, Map, bOutSuccess);
		}
	}

	return bOutSuccess;
}

bool FHitInfoHandle::operator==(const FHitInfoHandle& Other) const
{
	if (Data.IsValid() != Other.Data.IsValid())
	{
		return false;
	}

	if (Data.Get() != Other.Data.Get())
	{
		return false;
	}
	
	return true;
}

bool FHitInfoHandle::operator!=(const FHitInfoHandle& Other) const
{
	return !(*this == Other);
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

	ConstructorHelpers::FObjectFinder<UDMUpgradeGearInfluence> UpgradeInf(TEXT("/Game/Main/Asset/Upgrade/DA_UpgradeInf_Atk.DA_UpgradeInf_Atk"));
	UpgradeInfluenceAttributes = UpgradeInf.Object;
}

void AShootWeapon::BeginPlay()
{
	if (GetIsReplicated())
	{
		OwningShooter = Cast<ADCharacterPlayer>(GetOwner());
		ensureMsgf(OwningShooter, TEXT("OwningShooter Invalid"));
	}
	
	if (IsLocalWeapon())
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

		//SetActorTickEnabled(true);

		if (ADMPlayerController* OwningController = GetOwningController())
		{
			Handle_PlayerAmmunitionChange = OwningController->GetAmmunitionChangeDelegate().AddUObject(this, &AShootWeapon::OnPlayerAmmunitionChanged);
		}
	}

	Super::BeginPlay();
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

	if (OwningShooter)
	{
		OwningShooter->AddControllerYawInput(YawDelta);
		OwningShooter->AddControllerPitchInput(PitchDelta);
	}
}

void AShootWeapon::AimTimelineTick(float Value) const
{
	if (OwningShooter)
	{
		OwningShooter->SetCameraFieldOfView(FMath::LerpStable(90.f, AimFOV, Value));
		OwningShooter->CameraAimTransformLerp(Value);
	}
}

float AShootWeapon::GetCurrentRecoil() const
{
	return CurrentSpread;
}

void AShootWeapon::HandleFire()
{
	LastFireTime = GetWorld()->TimeSeconds;
	GetWorldTimerManager().SetTimer(Handle_ReSpread, this, &AShootWeapon::SpreadRecovery, RecoveryRecoilDelay, false);

	if (RecoveryTimeline.IsPlaying())
	{
		RecoveryTimeline.Stop();
	}

	if (FireCameraShake)
	{
		if (ADMPlayerController* PlayerController = GetOwningController())
		{
			PlayerController->PlayerCameraManager->StartCameraShake(FireCameraShake);
		}
	}

	if (ShellFragment > 1)
	{
		TArray<FHitResult> Hits;
		Hits.AddDefaulted(ShellFragment);
		
		for (int32 I = 0; I < ShellFragment; I++)
		{
			HandleLineTrace(Hits[I]);
		}
		
		HitInfoHandle.Set(MakeShared<FHitInfo_MultiBullet>(Hits));
	}
	else
	{
		FHitResult Hit;
		HandleLineTrace(Hit);
		
		HitInfoHandle.Set(MakeShared<FHitInfo_SingleBullet>(Hit));
	}

	// 增加后坐力
	RecoilTimeline.PlayFromStart();

	BP_OnFiring();

	SpawnProjectile();
	ServerSpawnProjectile(HitInfoHandle);

	AmmoNum--;
}

void AShootWeapon::HandleFireStop()
{
	
}

void AShootWeapon::HandleLineTrace(FHitResult& OutHit) const
{
	check(OwningShooter);

	FVector CameraLoc;
	FRotator CameraRot;
	OwningShooter->GetCameraLocationAndRotation(CameraLoc, CameraRot);
	
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

	FVector ShootDir = RandomStream.VRandCone(CameraRot.Vector(), AngleRadians, AngleRadians);
	FVector TraceEnd = CameraLoc + ShootDir * TraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());
	

	FHitResult ViewHitResult;
	GetWorld()->LineTraceSingleByChannel(ViewHitResult, CameraLoc, TraceEnd, ECC_Visibility, QueryParams);

	GetActualProjectileHitResult(ViewHitResult, OutHit);

#if ENABLE_DRAW_DEBUG

	if (DebugWeaponCVar::DebugCVar.GetValueOnAnyThread() == 1)
	{
		UKismetSystemLibrary::DrawDebugLine(this, CameraLoc, TraceEnd, FLinearColor::Blue, 5.f, 1.f);
	}

#endif

}

void AShootWeapon::GetActualProjectileHitResult(const FHitResult& ViewHitResult, FHitResult& ActualHitResult) const
{
	ActualHitResult = ViewHitResult;
}

void AShootWeapon::ServerSpawnProjectile_Implementation(const FHitInfoHandle& HitInfo)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(AShootWeapon, HitInfoHandle, this);

	HitInfoHandle = HitInfo;

	SpawnProjectile();
}

void AShootWeapon::SpawnProjectile()
{
	if (FHitInfo_SingleBullet* SingleHit = HitInfoHandle.Get<FHitInfo_SingleBullet>())
	{
		if (FHitResult* HitResult = SingleHit->GetHitResult())
		{
			HandleSpawnProjectile(*HitResult);
		}
	}
	else if (FHitInfo_MultiBullet* MultiHit = HitInfoHandle.Get<FHitInfo_MultiBullet>())
	{
		// todo 或许可以聚合一下 ?
		const TArray<FHitResult>& HitResults = MultiHit->GetHits();
		for (const FHitResult& HitResult : HitResults)
		{
			HandleSpawnProjectile(HitResult);
		}
	}

	if (GetNetMode() != NM_DedicatedServer)
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

// [ROLE_Simulation]
void AShootWeapon::OnRep_HitInfoHandle()
{
	SpawnProjectile();
}


FTransform AShootWeapon::GetPreviewRelativeTransform() const
{
	return PreviewTransform;
}

UDMUpgradeGearInfluence* AShootWeapon::GetUpgradeAttributesInfluence() const
{
	return UpgradeInfluenceAttributes;
}

void AShootWeapon::GetMuzzlePoint(FVector& Point, FRotator& Direction) const
{
	Direction = WeaponMesh->GetSocketRotation(MuzzleSocketName);
	Point = WeaponMesh->GetSocketLocation(MuzzleSocketName) + Direction.RotateVector(MuzzleLocationOffset);
}

void AShootWeapon::GetOwningPlayerCameraViewPoint(FVector& Location, FRotator& Rotation)
{
	if (OwningShooter)
	{
		OwningShooter->GetCameraLocationAndRotation(Location, Rotation);
	}
}

int32 AShootWeapon::GetInitAmmoNum() const
{
	return InitAmmo;
}

int32 AShootWeapon::GetInitReserveAmmoNum() const
{
	return InitReserveAmmo;
}

void AShootWeapon::InitializeWeaponAttributes(const FEquipmentAttributes& InAttributes, float AdditionMagnitude)
{
	WeaponAttribute = InAttributes;

	if (UpgradeInfluenceAttributes && AdditionMagnitude > 0.f)
	{
		UpgradeInfluenceAttributes->AttemptCalculateAddition(WeaponAttribute, AdditionMagnitude);
	}
}

const FEquipmentAttributes& AShootWeapon::GetAttributes() const
{
	return WeaponAttribute;
}

void AShootWeapon::OnAimEnded()
{
	BP_OnWeaponAimed();
}

void AShootWeapon::ApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	AActor* HitActor = HitResult.GetActor();
	if (HitActor == nullptr)
	{
		return;
	}

	if (IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor))
	{
		float DamageFalloff = 1.f;
		if (DamageFalloffCurve)
		{
			DamageFalloff = DamageFalloffCurve->GetFloatValue(FVector::Distance(Origin, HitResult.ImpactPoint));
		}

		FDreamGameplayEffectContext* EffectContext = UDGameplayStatics::MakeDreamEffectContext(OwningShooter, DamageFalloff, HitResult);
		EffectContext->SetDamageType(static_cast<EDDamageType>(WeaponType));

		FGameplayEffectSpec Spec(GetDefault<UDMEffect_ShootingDamage>(), FGameplayEffectContextHandle(EffectContext));
		Spec.DynamicAssetTags = DynamicAssetTags;
		Spec.DynamicGrantedTags = DynamicGrantedTags;
		TargetASI->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(Spec);
	}
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

void AShootWeapon::OnReloadFinished()
{
	int32 ReloadAmmo = FMath::Min(InitAmmo - AmmoNum, ReserveAmmo);
	
	AmmoNum += ReloadAmmo;

	if (ADMPlayerController* Controller = OwningShooter->GetPlayerController())
	{
		Controller->SetWeaponAmmunition(AmmoType, static_cast<float>(ReserveAmmo - ReloadAmmo) / InitReserveAmmo);
	}
}

void AShootWeapon::OnPlayerAmmunitionChanged(EAmmoType ChangedAmmoType, float NewAmmunition)
{
	if (AmmoType == ChangedAmmoType)
	{
		ReserveAmmo = FMath::RoundHalfFromZero(InitReserveAmmo * NewAmmunition);
	}
}

void AShootWeapon::ServerSetWeaponState_Implementation(EWeaponState NewState)
{
	SetWeaponState(NewState);
}

void AShootWeapon::SetWeaponState(EWeaponState NewState)
{
	// [local/server]
	WeaponState = NewState;

	if (GetLocalRole() == ROLE_Authority)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AShootWeapon, WeaponState, this);
	}
	else
	{
		ServerSetWeaponState(NewState);
	}
}

void AShootWeapon::OnRep_WeaponState()
{
	if (WeaponState == EWeaponState::Reloading)
	{
		PlayWeaponReloadingAnim();
		OwningShooter->PlayMontage(CharacterAnim.ReloadAnim);
	}
	else if (WeaponState == EWeaponState::Equipping)
	{
		OwningShooter->PlayMontage(CharacterAnim.EquipAnim);
	}
	else if (WeaponState == EWeaponState::Idle && LastWeaponState != EWeaponState::Idle)
	{
		if (WeaponMesh->AnimScriptInstance)
		{
			WeaponMesh->AnimScriptInstance->StopAllMontages(0.2f);
		}

		if (UAnimInstance* AnimInstance = OwningShooter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.2f);		
		}
	}

	LastWeaponState = WeaponState;
}

void AShootWeapon::PlayWeaponShootingAnim()
{
	if (WeaponMesh->AnimScriptInstance && ShootAnim)
	{
		WeaponMesh->AnimScriptInstance->Montage_Play(ShootAnim);
	}
}

void AShootWeapon::PlayWeaponReloadingAnim()
{
	if (WeaponMesh->AnimScriptInstance && ReloadAnim)
	{
		WeaponMesh->AnimScriptInstance->Montage_Play(ReloadAnim);
	}
}


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

void AShootWeapon::OnRep_AttachmentReplication()
{
	Super::OnRep_AttachmentReplication();

	const FRepAttachment& AttachmentRep = GetAttachmentReplication();

	if (AttachmentRep.AttachSocket == UDMProjectSettings::Get()->GetWepActiveSockName())
	{
		if (ADCharacterPlayer* AttachedShooter = Cast<ADCharacterPlayer>(AttachmentRep.AttachParent))
		{
			AttachedShooter->UpdateOverlayDetailID(AnimID);
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

	FDoRepLifetimeParams Params;
	Params.Condition = COND_SkipOwner;
	Params.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(AShootWeapon, HitInfoHandle, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AShootWeapon, WeaponState, Params);
}
