// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/DCharacterPlayer.h"
#include "AbilitySystemGlobals.h"
#include "DBaseAttributesAsset.h"
#include "UnrealNetwork.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Engine.h"
#include "DGameplayStatics.h"
#include "DEnemyBase.h"
#include "DGameplayEffectUIData.h"
#include "DGameplayTags.h"
#include "DGameUserSettings.h"
#include "DMAbilitiesStatics.h"
#include "DMAttributeSet.h"
#include "DMMantleAsset.h"
#include "DModuleBase.h"
#include "DreamGameplayAbility.h"
#include "DreamGameplayType.h"
#include "Components/DamageWidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "PlayerDataInterfaceStatic.h"
#include "DMPlayerController.h"
#include "DmProjectSettings.h"
#include "DreamWidgetStatics.h"
#include "MinimapScanComponent.h"
#include "UI/SPlayerHUD.h"

#define LOCTEXT_NAMESPACE "PlayerCharacter"

#define MAX_MOUSE_SENSITIVITY 135

#define TRIGGER_ABILITY(Tag, Target_) \
    { \
        FGameplayEventData Payload; \
        Payload.Instigator = this; \
        Payload.Target = Target_; \
        AbilitySystem->HandleGameplayEvent(CUSTOMIZE_TAG(Tag), &Payload); \
    }

FRootMotionSource_Mantling::FRootMotionSource_Mantling()
    : StartingPosition(0.f)
    , PositionCorrectionCurve(nullptr)
    , PathOffsetCurve(nullptr)
{
}

FRootMotionSource* FRootMotionSource_Mantling::Clone() const
{
    FRootMotionSource_Mantling* CopyPtr = new FRootMotionSource_Mantling(*this);
    return CopyPtr;
}

bool FRootMotionSource_Mantling::Matches(const FRootMotionSource* Other) const
{
    if (!FRootMotionSource::Matches(Other))
    {
        return false;
    }

    const FRootMotionSource_Mantling* Source_Mantling = static_cast<const FRootMotionSource_Mantling*>(Other);

    return FMath::IsNearlyEqual(StartingPosition, Source_Mantling->StartingPosition) &&
            PositionCorrectionCurve == Source_Mantling->PositionCorrectionCurve &&
            FVector::PointsAreNear(MantlingTarget, Source_Mantling->MantlingTarget, 0.1f) &&
            FVector::PointsAreNear(MantlingActualStartOffset, Source_Mantling->MantlingActualStartOffset, 0.1f) &&
            FVector::PointsAreNear(MantlingAnimatedStartOffset, Source_Mantling->MantlingAnimatedStartOffset, 0.1f);
}

void FRootMotionSource_Mantling::PrepareRootMotion(float SimulationTime, float MovementTickTime,
    const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
    RootMotionParams.Clear();

    if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
    {
    }

    SetTime(GetTime() + SimulationTime);
}

bool FRootMotionSource_Mantling::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
    {
        return false;
    }

    if (Ar.IsSaving())
    {
        WriteFixedCompressedFloat<255, 16>(StartingPosition, Ar);
    }
    else
    {
        ReadFixedCompressedFloat<255, 16>(StartingPosition, Ar);
    }
    
    Ar << PositionCorrectionCurve;
    Ar << PathOffsetCurve;
    Ar << MantlingTarget;
    Ar << FaceRotation;
    Ar << MantlingActualStartOffset;
    Ar << MantlingAnimatedStartOffset;

    bOutSuccess = true;
    return true;
}

UScriptStruct* FRootMotionSource_Mantling::GetScriptStruct() const
{
    return FRootMotionSource_Mantling::StaticStruct();
}

FString FRootMotionSource_Mantling::ToSimpleString() const
{
    return FString::Printf(TEXT("[ID:%u]FRootMotionSource_Mantling %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FRootMotionSource_Mantling::AddReferencedObjects(FReferenceCollector& Collector)
{
    Super::AddReferencedObjects(Collector);
    Collector.AddReferencedObject(PositionCorrectionCurve);
}

// Sets default values
ADCharacterPlayer::ADCharacterPlayer()
    : SprintSpeed(1000.f)
    , CombatStatusCount(0)
    , DefaultGait(EMovementGait::Running)
    , MovementState(EMovementState::Grounded)
    , OverlayState(EOverlayState::Rifle)
    , ActiveWeaponIndex(0)
    , HealthUpdateFrequency(0.1f)
    , MantleLimit(0.1f)
    , ToggleOverlayLimit(0.1f)
{

    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;

    MinRotateLimit = -60.f;
    MaxRotateLimit = 60.f;
    
    PrimaryActorTick.bCanEverTick = true;
    //PrimaryActorTick.bStartWithTickEnabled = false;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    TPCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPCameraArm"));
    TPCameraArm->SetupAttachment(RootComponent);
    TPCameraArm->SetRelativeLocation(FVector(0.f, 0.f, 85.f));
    TPCameraArm->bUsePawnControlRotation = true;
    TPCameraArm->bInheritYaw = true;
    TPCameraArm->bInheritRoll = false;
    TPCameraArm->bInheritPitch = false;

    TPCameraArm->TargetArmLength = 130.f;
    TPCameraArm->SocketOffset = FVector(0.f, 40.f, 0);

    TPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPCamera"));
    TPCamera->SetupAttachment(TPCameraArm);
    TPCamera->bUsePawnControlRotation = true;
    TPCamera->bAutoActivate = true;

    bUseControllerRotationYaw = false;

    GetMesh()->bReceivesDecals = false;

    //GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GetMesh()->SetCollisionResponseToChannel(Collision_ObjectType_Projectile, ECollisionResponse::ECR_Block);

    GetCapsuleComponent()->SetCollisionResponseToChannel(Collision_ObjectType_Projectile, ECollisionResponse::ECR_Ignore);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    GetCharacterMovement()->bOrientRotationToMovement = false;

    ScanComponent = CreateDefaultSubobject<UMinimapScanComponent>(TEXT("ScanComponent"));

    WeaponInventory.SetNumZeroed(2);
    EquippedModules.SetNumZeroed(static_cast<uint8>(EModuleCategory::Max));
    
    CharacterAttributes = CreateDefaultSubobject<UDMAttributeSet>(TEXT("AttributeSet"));
}

void ADCharacterPlayer::PossessedBy(AController* InController)
{
    // [Server]
    Super::PossessedBy(InController);

    InitializePlayerGears();

    //UE_LOG(LogDream, Error, TEXT("Role: %s, PossessedBy"), *UEnum::GetValueAsString(GetLocalRole()));
}

void ADCharacterPlayer::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    Gait = DefaultGait;

    MantlingTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);

    FOnTimelineFloatStatic TimelineTick;
    TimelineTick.BindUObject(this, &ADCharacterPlayer::MantlingUpdate);
    MantlingTimeline.AddInterpFloat(GSProject->GetMantlePositionCurve(), TimelineTick);

    FOnTimelineEventStatic OnFinished;
    OnFinished.BindUObject(this, &ADCharacterPlayer::OnMantleFinished);
    MantlingTimeline.SetTimelineFinishedFunc(OnFinished);

    // 这里无法判断isLocalController 可能是此时本地的Role还没有设置好
}

void ADCharacterPlayer::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    if (GetLocalRole() == ROLE_Authority && Controller)
    {
        ControllerRotation = GetControlRotation().Vector();
    }
}

void ADCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    GetMesh()->AddTickPrerequisiteActor(this);

    FRotator ActorRotation = GetActorRotation();
    TargetRotation = ActorRotation;
    LastVelocityRotation = ActorRotation;

    // [Local]
    if (IsLocalCharacter())
    {
        InitializePlayerHUD();
        
        DefaultCameraRotation = TPCamera->GetRelativeRotation();
        DefaultArmSocketOffset = TPCameraArm->SocketOffset;

        AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectAdded);
        AbilitySystem->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectRemoved);

        Handle_Properties = FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.AddUObject(this, &ADCharacterPlayer::OnPlayerPropertiesChanged);
    }

    AbilitySystem->InitAbilityActorInfo(this, this);
}

void ADCharacterPlayer::InitializePlayerHUD()
{
    PlayerHUD = SNew(SPlayerHUD)
                .InOwnerPlayer(this)
                .MinimapDataIterator_UObject(this, &ADCharacterPlayer::GetMinimapDataIterator);
            
    GEngine->GameViewport->AddViewportWidgetContent(PlayerHUD.ToSharedRef(), EWidgetOrder::Player);

    UpdateWeaponHUD();
}

void ADCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (Handle_Properties.IsValid())
    {
        FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.Remove(Handle_Properties);
    }
}

void ADCharacterPlayer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateEssentialValues(DeltaSeconds);

    MantlingTimeline.TickTimeline(DeltaSeconds);

    if (GetLocalRole() >= ROLE_AutonomousProxy)
    {
        UpdateGroundedRotation(DeltaSeconds);
    }
    
    if (GetLocalRole() == ROLE_Authority)
    {
        if (IsUnhealthy())
        {
            if (RecoveryTimer.IsCompleted())
            {
                float Delta = 0;
                if (HealthUpdateFrequency.Tick(DeltaSeconds, Delta))
                {
                    // 应用血量回复
                    float DeltaHealth = Delta * CharacterAttributes->GetMaxHealth();
                    AbilitySystem->ApplyModToAttribute(DMAttrStatics().HealthProperty, EGameplayModOp::Additive, DeltaHealth);
                }
            }
            else
            {
                RecoveryTimer.Tick(DeltaSeconds);
            }
        }
    }
}

/*void ADCharacterPlayer::TickRootMotionSource()
{
    if (MantlingRMS.IsValid())
    {
        if (MantlingRMS->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
        {
            OnMantleRootMotionFinished();
            MantlingRMS.Reset();
        }
    }
}*/

void ADCharacterPlayer::SmoothUpdateRotation(float DeltaSeconds, float RotRate, float RotRateConst, const FRotator& Target)
{
    // [Server/Client]
    TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaSeconds, RotRateConst);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0, TargetRotation.Yaw, 0), DeltaSeconds, RotRate));
}

void ADCharacterPlayer::LimitRotation(float DeltaSeconds, float YawMin, float YawMax, float InterpSpeed)
{
    FRotator ControlRotation = GetRepControllerRotation();
    FRotator Delta = (ControlRotation - GetActorRotation()).GetNormalized();

    if (!UKismetMathLibrary::InRange_FloatFloat(Delta.Yaw, YawMin, YawMax))
    {
        SmoothUpdateRotation(DeltaSeconds, InterpSpeed, 0.f, FRotator(0, ControlRotation.Yaw + (Delta.Yaw > 0.f ? YawMin : YawMax), 0));
    }
}

void ADCharacterPlayer::UpdateEssentialValues(float DeltaSeconds)
{
    FVector CurrentVelocity = GetVelocity();

    Speed = CurrentVelocity.Size2D();
    bIsMoving = Speed > 1.f;

    if (bIsMoving)
    {
        LastVelocityRotation = CurrentVelocity.Rotation();
    }

    Acceleration = (CurrentVelocity - PrevVelocity) / DeltaSeconds;
    PrevVelocity = CurrentVelocity;

    MovementInputAmount = GetCharacterMovement()->GetCurrentAcceleration().Size() / GetCharacterMovement()->GetMaxAcceleration();
    bHasMovementInput = MovementInputAmount > 0.f;

    FRotator RepControllerRotation = GetRepControllerRotation();
    AimYawRate = FMath::Abs((RepControllerRotation.Yaw - PrevAimYaw) / DeltaSeconds);
    PrevAimYaw = RepControllerRotation.Yaw;
    
}

void ADCharacterPlayer::UpdateGroundedRotation(float DeltaSeconds)
{
    if (MovementState != EMovementState::Grounded)
    {
        return;
    }

    if ((bIsMoving && bHasMovementInput || Speed > 150.f) && !HasAnyRootMotion())
    {
        if (bCombatStatus)
        {
            SmoothUpdateRotation(DeltaSeconds, 20.f, 1000.f, GetRepControllerRotation());
        }
        else
        {
            float CurveValue = GSProject->GetRotationRateCurve()->GetFloatValue(GetMappedSpeed());
            float RotationRate = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 300.f), FVector2D(1.f, 3.f), AimYawRate) * CurveValue;
            SmoothUpdateRotation(DeltaSeconds, RotationRate, 800.f, FRotator(0,LastVelocityRotation.Yaw,0));

            //UE_LOG(LogDream, Error, TEXT("Role: %s, SmoothUpdateRotation"), *UEnum::GetValueAsString(GetLocalRole()));
        }
    }
    else
    {
        if (bCombatStatus)
        {
            LimitRotation(DeltaSeconds, -60.f, 60.f, 20.f);
        }

        // 不适用于网络复制
        float RotationAmount = 0.f;
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->GetCurveValue("RotationAmount", RotationAmount);
        }

        if (FMath::Abs(RotationAmount) > KINDA_SMALL_NUMBER)
        {
            //UE_LOG(LogDream, Error, TEXT("RotationAmount %f"), RotationAmount);
            
            AddActorWorldRotation(FRotator(0, RotationAmount * (DeltaSeconds / 0.033334f), 0));
            TargetRotation = GetActorRotation();
        }
    }
}

float ADCharacterPlayer::GetMappedSpeed()
{
    if (Speed < WalkSpeed)
    {
        return FMath::GetMappedRangeValueClamped(FVector2D(0.f, WalkSpeed), FVector2D(0.f, 1.f), Speed);
    }

    if (Speed < RunSpeed)
    {
        return FMath::GetMappedRangeValueClamped(FVector2D(WalkSpeed, RunSpeed), FVector2D(1.f, 2.f), Speed);
    }

    return FMath::GetMappedRangeValueClamped(FVector2D(RunSpeed, SprintSpeed), FVector2D(2.f, 3.f), Speed);
}


float ADCharacterPlayer::PlayMontage(UAnimMontage* PawnAnim)
{
    float Duration = .1f;

    if (GetNetMode() == NM_DedicatedServer)
    {
        if (PawnAnim)
        {
            Duration = PawnAnim->SequenceLength / PawnAnim->RateScale;
        }
    }
    else
    {
        // 服务器获取的持续时间应该与客户端一致
        if (PawnAnim)
        {
            Duration = GetMesh()->AnimScriptInstance->Montage_Play(PawnAnim, 1.f, EMontagePlayReturnType::Duration);
        }
    }

    return Duration;
}

float ADCharacterPlayer::PlayReloadingMontage()
{
    if (ActiveWeapon)
    {
        ActiveWeapon->PlayWeaponReloadingAnim();
        return PlayMontage(ActiveWeapon->CharacterAnim.ReloadAnim);
    }
    
    return 0;
}

void ADCharacterPlayer::StopReloadingMontage()
{
    if (ActiveWeapon)
    {
        StopAnimMontage(ActiveWeapon->CharacterAnim.ReloadAnim);
        if (UAnimInstance* AnimInstance = ActiveWeapon->WeaponMesh->GetAnimInstance())
        {
            AnimInstance->Montage_Stop(0.1f, ActiveWeapon->ReloadAnim);
        }
    }
}

float ADCharacterPlayer::PlayEquippingMontage()
{
    if (ActiveWeapon)
    {
        return PlayMontage(ActiveWeapon->CharacterAnim.EquipAnim);
    }
    
    return 0;
}


ADMPlayerController* ADCharacterPlayer::GetPlayerController() const
{
    return Cast<ADMPlayerController>(Controller);
}

// Called to bind functionality to input
void ADCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UDGameUserSettings* Settings = Cast<UDGameUserSettings>(GEngine->GetGameUserSettings()))
    {
        SetMouseInputScale(Settings->GetMouseSensitivity());
    }
    
    PlayerInputComponent->BindAxis("MoveForward", this, &ADCharacterPlayer::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ADCharacterPlayer::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ADCharacterPlayer::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &ADCharacterPlayer::LookUpAtRate);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ADCharacterPlayer::MantleStart);

    PlayerInputComponent->BindAction("OpenFire", IE_Pressed, this, &ADCharacterPlayer::StartFire);
    PlayerInputComponent->BindAction("OpenFire", IE_Released, this, &ADCharacterPlayer::StopFire);

    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ADCharacterPlayer::SwitchWeapon);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADCharacterPlayer::ReloadMagazine);

    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ADCharacterPlayer::StartAim);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ADCharacterPlayer::StopAim);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADCharacterPlayer::ToggleSprint);
    
    PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ADCharacterPlayer::Rolling);
    
    PlayerInputComponent->BindAction("ToggleOverlay", IE_Pressed, this, &ADCharacterPlayer::ToggleOverlay);

    PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &ADCharacterPlayer::SwitchWeaponToFirst);
    PlayerInputComponent->BindAction("Weapon2", IE_Pressed, this, &ADCharacterPlayer::SwitchWeaponToSecond);
}

void ADCharacterPlayer::StartFire()
{
    if (!ActiveWeapon)
    {
        return;
    }

    bDesiredShoot = true;

    if (ActiveWeapon->FireMode != EFireMode::Accumulation)
    {
        if (!ActiveWeapon->CanFire())
        {
            return;
        }
    }

    if (ActiveWeapon->GetWeaponState() > EWeaponState::Shooting)
    {
        return;
    }

    ActiveWeapon->SetWeaponState(EWeaponState::Shooting);

    ActiveWeapon->BP_OnStartFire();

    bool bDelayFire = !bCombatStatus;

    StopSprint();
    StateSwitchToCombat();

    if (bDelayFire)
    {
        FTimerHandle Temporary;
        GetWorldTimerManager().SetTimer(Temporary, this, &ADCharacterPlayer::ConfirmFire, 0.15f);
    }
    else
    {
        ConfirmFire();
    }
}

void ADCharacterPlayer::ConfirmFire()
{
    switch (ActiveWeapon->FireMode)
    {
    case EFireMode::SemiAutomatic:
    case EFireMode::FullyAutomatic:
        
        HandleFire();
        
        break;
        
    case EFireMode::Accumulation:

        // 蓄力一定时间后开火
        GetWorldTimerManager().SetTimer(Handle_Shoot, this,
                &ADCharacterPlayer::HandleFire, ActiveWeapon->GetFireInterval());
        
        break;
    }
}

void ADCharacterPlayer::HandleFire()
{
    if (!(ActiveWeapon->IsState(EWeaponState::Shooting) && ActiveWeapon->AmmoNum > 0))
    {
        return;
    }

    ActiveWeapon->HandleFire();
    
    UpdateMagazineUI();

    if (ActiveWeapon->FireCameraShake)
    {
        if (ADMPlayerController* PlayerController = GetPlayerController())
        {
            PlayerController->ClientStartCameraShake(ActiveWeapon->FireCameraShake);
        }
    }

    bool bNoAmmo = ActiveWeapon->AmmoNum == 0;

    switch (ActiveWeapon->FireMode)
    {
    case EFireMode::SemiAutomatic:
    case EFireMode::Accumulation:
        StopFire(); // 半自动和蓄力 在这里直接调用StopFire
        break;
        
    case EFireMode::FullyAutomatic:
        if (!bNoAmmo)
        {
            GetWorldTimerManager().SetTimer(Handle_Shoot, this,
                &ADCharacterPlayer::HandleFire, ActiveWeapon->GetFireInterval());
        }
        break;
    }

    // 检测到没有弹药时自动换弹
    if (bNoAmmo)
    {
        FTimerHandle Temporary;
        GetWorldTimerManager().SetTimer(Temporary, this, &ADCharacterPlayer::ReloadMagazine, 0.2f);
    }
}

void ADCharacterPlayer::StopFire()
{
    if (!ActiveWeapon)
    {
        return;
    }

    bDesiredShoot = false;

    if (ActiveWeapon->IsState(EWeaponState::Shooting))
    {
        ActiveWeapon->SetWeaponState(EWeaponState::Idle);
        
        StateSwitchToRelax();

        ActiveWeapon->BP_OnStopFire();
    }
}

void ADCharacterPlayer::SwitchWeapon()
{
    HandleSwitchWeapon(INDEX_NONE);
}

void ADCharacterPlayer::HandleSwitchWeapon(int32 WeaponIndex)
{
    if (!CanEquip())
    {
        return;
    }
    
    if (bCombatStatus)
    {
        StopFire();
        StopAim();
        StopReloadMagazine();
    }

    ActiveWeapon->SetWeaponState(EWeaponState::Equipping);
    
    float Duration = PlayEquippingMontage();
    
    GetWorldTimerManager().SetTimer(Handle_Equip, FTimerDelegate::CreateUObject(
                                    this, &ADCharacterPlayer::SwitchFinished, WeaponIndex), Duration, false);
}

void ADCharacterPlayer::SwitchFinished(int32 WeaponIndex)
{
    //ActiveWeapon->SetActorHiddenInGame(true);
    if (IsLocallyControlled())
    {
        ActiveWeapon->SetWeaponEnable(false);
        ActiveWeapon->SetWeaponState(EWeaponState::Idle);
        //ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("root"));
    }

    if (GetLocalRole() == ROLE_Authority)
    {
        if (WeaponIndex == INDEX_NONE)
        {
            WeaponIndex = ActiveWeaponIndex == (WeaponInventory.Num() - 1) ? 0 : ActiveWeaponIndex + 1;
        }

        ActiveWeaponIndex = WeaponIndex;

        SwitchWeapon(WeaponIndex);
    }
}

void ADCharacterPlayer::SwitchWeaponToFirst()
{
    if (ActiveWeaponIndex != 0)
    {
        HandleSwitchWeapon(0);
    }
}

void ADCharacterPlayer::SwitchWeaponToSecond()
{
    if (ActiveWeaponIndex != 1)
    {
        HandleSwitchWeapon(1);
    }
}

void ADCharacterPlayer::ReloadMagazine()
{
    if (CanReload())
    {
        if (bDesiredShoot)
        {
            StopFire();
            // 换弹之后继续射击
            bDesiredShoot = true;
        }

        ActiveWeapon->SetWeaponState(EWeaponState::Reloading);

        TRIGGER_ABILITY(Condition_Reloading, this);

        float Duration = PlayReloadingMontage();
        GetWorldTimerManager().SetTimer(Handle_Reload, this, &ADCharacterPlayer::ReloadFinished, Duration);
        StateSwitchToCombat();
    }
}

void ADCharacterPlayer::StopReloadMagazine()
{
    if (ActiveWeapon && ActiveWeapon->IsState(EWeaponState::Reloading))
    {
        StateSwitchToRelax();
        ActiveWeapon->SetWeaponState(EWeaponState::Idle);
        GetWorldTimerManager().ClearTimer(Handle_Reload);
        StopReloadingMontage();
    }
}

void ADCharacterPlayer::ReloadFinished()
{
    UpdateMagazineUI();

    ActiveWeapon->OnReloadFinished();
    ActiveWeapon->SetWeaponState(EWeaponState::Idle);

    StateSwitchToRelax();

    if (bDesiredShoot)
    {
        StartFire();   
    }
}

void ADCharacterPlayer::EquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> WeaponClass)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        Index = FMath::Clamp(Index, 0, WeaponInventory.Num() - 1);
        ServerEquipWeapon(Index, Attrs, WeaponClass);
    }
    else
    {
        DoServerEquipWeapon(Index, Attrs, *WeaponClass);
    }
}

void ADCharacterPlayer::DoServerEquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, UClass* NewWeaponClass)
{
    FActorSpawnParameters SpawnParam;
    SpawnParam.Owner = this;
    SpawnParam.Instigator = this;
    AShootWeapon* NewWeapon = GetWorld()->SpawnActor<AShootWeapon>(NewWeaponClass, SpawnParam);
    NewWeapon->WeaponAttribute = Attrs;

    if (AShootWeapon* OldWeapon = WeaponInventory[Index])
    {
        OldWeapon->Destroy();
    }
        
    WeaponInventory[Index] = NewWeapon;

    if (Index == ActiveWeaponIndex)
    {
        FEquipmentAttributes* PrevWeaponAttr = nullptr;
        if (ActiveWeapon)
        {
            FEquipmentAttributes Attributes = MoveTemp(ActiveWeapon->WeaponAttribute);
            PrevWeaponAttr = &Attributes;
        }

        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, UDMProjectSettings::Get()->GetWepActiveSockName());
        NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
        SetActiveWeapon(NewWeapon);

        // 武器还未初始化时不刷新属性
        if (PrevWeaponAttr)
        {
            FastRefreshWeaponAttribute(*PrevWeaponAttr);
        }
    }
    else
    {
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, UDMProjectSettings::Get()->GetWepHolsterSockName());
        NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponHolsterSocketOffset);
    }
}

void ADCharacterPlayer::ServerEquipWeapon_Implementation(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> NewWeaponClass)
{
    EquipWeapon(Index, Attrs, NewWeaponClass);
}

void ADCharacterPlayer::EquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerEquipModule(ModuleClass, Attrs);
    }
    else
    {
        DoServerEquipModule(*ModuleClass, Attrs);
        RefreshAttributeBaseValue();
    }
}

void ADCharacterPlayer::DoServerEquipModule(UClass* ModuleClass, const FEquipmentAttributes& Attrs)
{
    UDModuleBase* Module = NewObject<UDModuleBase>(this, ModuleClass);
    Module->ModuleAttributes = Attrs;

    uint8 CategoryValue = static_cast<uint8>(Module->Category);
    EquippedModules[CategoryValue] = Module;
}

void ADCharacterPlayer::ServerEquipModule_Implementation(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs)
{
    EquipModule(ModuleClass, Attrs);
}

void ADCharacterPlayer::LearningTalents(int64 Talents)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        // todo 待完善
        TArray<FTalentInfo> TalentInfos;
        GSProject->GetLearnedTalents(ETalentCategory::Warrior, Talents, TalentInfos);

        LearnedTalents.SetNum(TalentInfos.Num());

        for (int32 Idx = 0; Idx < LearnedTalents.Num(); Idx++)
        {
            LearnedTalents[Idx] = TalentInfos[Idx].TalentClass;
        }
        
        RefreshAttributeBaseValue();
    }
    else
    {
        ServerLearningTalent(Talents);
    }
}

void ADCharacterPlayer::ServerLearningTalent_Implementation(int64 Talents)
{
    LearningTalents(Talents);
}

void ADCharacterPlayer::SwitchWeapon(int32 NewWeaponIndex)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    AShootWeapon* NewWeapon = WeaponInventory[NewWeaponIndex];

    FEquipmentAttributes PrevWeaponAttr = ActiveWeapon->WeaponAttribute;

    if (ActiveWeapon)
    {
        ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, UDMProjectSettings::Get()->GetWepHolsterSockName());
        ActiveWeapon->SetActorRelativeTransform(ActiveWeapon->WeaponHolsterSocketOffset);
    }

    NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, UDMProjectSettings::Get()->GetWepActiveSockName());
    NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
        
    SetActiveWeapon(NewWeapon);

    FastRefreshWeaponAttribute(PrevWeaponAttr);
}

void ADCharacterPlayer::AddInfiniteActors(const TArray<AActor*>& TargetActors)
{
    ScanComponent->AddInfiniteActors(TargetActors);
}

void ADCharacterPlayer::AddInfiniteActor(AActor* TargetActor)
{
    ScanComponent->AddInfiniteActor(TargetActor);
}

void ADCharacterPlayer::RemoveInfiniteActor(AActor* TargetActor)
{
    ScanComponent->RemoveInfiniteActor(TargetActor); 
}

void ADCharacterPlayer::ClearInfiniteActors()
{
    ScanComponent->ClearInfiniteActors();
}

void ADCharacterPlayer::SpawnDamageWidget(const FVector& Location, float DamageValue, bool bCritical, bool bIsHealthSteal)
{
    UClass* DamageComponentClass = GSProject->GetDamageComponentClass();

    if (DamageComponentClass)
    {
        UDamageWidgetComponent* DamageWidget = NewObject<UDamageWidgetComponent>(this, DamageComponentClass);
        float RandomZ = UKismetMathLibrary::RandomFloatInRange(-50.f, 50.f);
    
        DamageWidget->SetWorldLocation(Location + FVector(0, 0, RandomZ));
        DamageWidget->RegisterComponent();
        
        DamageWidget->ActiveComponent(DamageValue, bCritical, bIsHealthSteal);
    }
}

void ADCharacterPlayer::ShowHitEnemyTips(bool bEnemyDeath)
{
    if (PlayerHUD)
    {
        PlayerHUD->ShowHitEnemyMark(bEnemyDeath);
    }
}

void ADCharacterPlayer::RefreshAttributeBaseValue()
{
    FEquipmentAttributes GatherAttributes;

    GatherGearsAttributes(GatherAttributes);
    ApplyGearsAbilitiesToSelf(GatherAttributes.Perks);
    CharacterAttributes->UpdateAttributesBase(GatherAttributes);
}

void ADCharacterPlayer::InitializeAttributes()
{
    FEquipmentAttributes GatherAttributes;

    GatherGearsAttributes(GatherAttributes);
    ApplyGearsAbilitiesToSelf(GatherAttributes.Perks);
    CharacterAttributes->InitAttributes(GatherAttributes);
}

void ADCharacterPlayer::ApplyGearsAbilitiesToSelf(const TArray<int32>& GearPerks)
{
    AbilitySystem->ClearAllAbilities();

#if WITH_EDITORONLY_DATA

    for (TSubclassOf<UGameplayAbility> TestAbility : TestAbilities)
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(TestAbility));
    }

#endif
    
    for (UClass* TalentClass : LearnedTalents)
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(TalentClass));
    }

    for (int32 AbilityGuid : GearPerks)
    {
        UClass* PerkClass = GSProject->GetItemClassFromGuid(AbilityGuid);
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(PerkClass));
    }

    if (ActiveWeapon)
    {
        for (int32 AbilityGuid : ActiveWeapon->WeaponAttribute.Perks)
        {
            UClass* PerkClass = GSProject->GetItemClassFromGuid(AbilityGuid);
            CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(PerkClass)));
        }
    }

    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CUSTOMIZE_TAG(GE_Buff_All)));
    
    TRIGGER_ABILITY(Condition_Immediately, this);
}

void ADCharacterPlayer::GatherGearsAttributes(FEquipmentAttributes& GearsAttributes)
{
    if (ActiveWeapon)
    {
        GearsAttributes.MergeAndSkipPerks(ActiveWeapon->WeaponAttribute);
    }

    for (UDModuleBase* Module : EquippedModules)
    {
        GearsAttributes += Module->ModuleAttributes;
    }

    // Level Base Attrs
    FBaseAttributes BaseAttributes = GSProject->GetBaseAttributes(GetCharacterLevel());
    GearsAttributes += BaseAttributes;
}

void ADCharacterPlayer::FastRefreshWeaponAttribute(const FEquipmentAttributes& PrevWeaponAttrs)
{
    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CUSTOMIZE_TAG(GE_Buff_Weapon)));

    for (const FGameplayAbilitySpecHandle& Handle : CacheWeaponPerkHandles)
    {
        AbilitySystem->ClearAbility(Handle);
    }

    CacheWeaponPerkHandles.Reset(ActiveWeapon->WeaponAttribute.Perks.Num());
    
    for (int32 N = 0; N < ActiveWeapon->WeaponAttribute.Perks.Num(); N++)
    {
        UClass* AbilityClass = GSProject->GetItemClassFromGuid(ActiveWeapon->WeaponAttribute.Perks[N]);
        CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass)));
    }

    CharacterAttributes->IncrementAttributes(ActiveWeapon->WeaponAttribute - PrevWeaponAttrs);

    TRIGGER_ABILITY(Condition_Immediately, this);
}

void ADCharacterPlayer::SetPlayerHUDVisible(bool bVisible)
{
    if (PlayerHUD.IsValid())
    {
        PlayerHUD->SetVisibility(bVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
    }
}

void ADCharacterPlayer::RemovePlayerHUD()
{
    if (PlayerHUD.IsValid())
    {
        if (GEngine->GameViewport)
        {
            GEngine->GameViewport->RemoveViewportWidgetContent(PlayerHUD.ToSharedRef());
        }
        PlayerHUD.Reset();
    }
}

void ADCharacterPlayer::SetActiveWeapon(AShootWeapon* NewWeapon)
{
    ActiveWeapon = NewWeapon;

    if (IsLocalCharacter())
    {
        ActiveWeapon->SetWeaponEnable(true);

        UpdateWeaponHUD();
    }

    if (GetLocalRole() == ROLE_Authority)
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, ActiveWeapon, this);
    }
}

void ADCharacterPlayer::OnRep_ActiveWeapon()
{
    SetActiveWeapon(ActiveWeapon);
}

void ADCharacterPlayer::InitializePlayerGears()
{
    if (ADMPlayerController* PlayerController = GetPlayerController())
    {
        PlayerController->GetPlayerInformation(FPlayerInfoSignature::CreateUObject(this, &ADCharacterPlayer::OnInitializePlayerGears));
    }
}

void ADCharacterPlayer::OnInitializePlayerGears(bool bValidResult, const FTemporaryPlayerInfo& PlayerInfo)
{
    if (!bValidResult)
    {
        if (ADMPlayerController* PlayerController = GetPlayerController())
        {
            PlayerController->ClientReturnToMainMenuWithTextReason(LOCTEXT("QueryPlayer", "获取玩家数据失败"));
        }

        return;
    }
    
    CurrentCharacterMesh = PlayerInfo.Skin;
    
    if (GetNetMode() == NM_Standalone)
    {
        UpdateCharacterMesh();
    }

    for (const FTemporaryGearInfo& PW : PlayerInfo.Weapons)
    {
        DoServerEquipWeapon(PW.EquippedIdx, PW.Attributes, PW.GearClass);
    }

    for (const FTemporaryGearInfo& PM : PlayerInfo.Modules)
    {
        DoServerEquipModule(PM.GearClass, PM.Attributes);
    }

    // todo 天赋分支功能待完善
    TArray<FTalentInfo> Talents;
    GSProject->GetLearnedTalents(ETalentCategory::Warrior, PlayerInfo.LearnedTalents, Talents);
    
    for (const FTalentInfo& Talent : Talents)
    {
        LearnedTalents.Add(Talent.TalentClass);
    }

    Level = PlayerInfo.Level;
    
    InitializeAttributes();
}

void ADCharacterPlayer::OnPlayerPropertiesChanged(const FPlayerProperties& Properties)
{
    // 武器初始化之后才更新
    if (ActiveWeapon == nullptr)
    {
        return;
    }

    if (GetCharacterLevel() < Properties.Level)
    {
        ServerSetCharacterLevel(Properties.Level);
    }
}

void ADCharacterPlayer::ServerSetCharacterLevel_Implementation(int32 NewLevel)
{
    Level = NewLevel;

    RefreshAttributeBaseValue();
}

bool ADCharacterPlayer::CanReload() const
{
    return ActiveWeapon && ActiveWeapon->CanReload();
}

bool ADCharacterPlayer::CanEquip() const
{
    return ActiveWeapon && ActiveWeapon->GetWeaponState() < EWeaponState::Equipping;
}

bool ADCharacterPlayer::CanAim() const
{
    return ActiveWeapon && ActiveWeapon->GetWeaponState() <= EWeaponState::Reloading;
}

void ADCharacterPlayer::UpdateWeaponHUD() const
{
    if (PlayerHUD.IsValid() && ActiveWeapon)
    {
        PlayerHUD->SetCrosshairBrush(ActiveWeapon->GetDynamicCrosshairBrush());
    }
    
    UpdateMagazineUI();
}

void ADCharacterPlayer::UpdateMagazineUI() const
{
    if (PlayerHUD.IsValid() && ActiveWeapon)
    {
        PlayerHUD->SetMagazineInformation(ActiveWeapon->AmmoNum,
            ActiveWeapon->ReserveAmmo, ActiveWeapon->GetRemainAmmoPercent());
    }
}

void ADCharacterPlayer::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADCharacterPlayer::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADCharacterPlayer::MoveForward(float Value)
{
    if (Controller != nullptr)
    {
        if (Value != 0.0f)
        {
            // find out which way is forward
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);
            // get forward vector
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            AddMovementInput(Direction, Value);
        }
        else if (Gait == EMovementGait::Sprinting)
        {
            StopSprint();
        }
    }

    AxisInput.X = Value;
}

void ADCharacterPlayer::MoveRight(float Value)
{
    if (Controller != nullptr && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        //AddControllerYawInput(Value * GetWorld()->GetDeltaSeconds());

        if (Gait == EMovementGait::Sprinting)
        {
            Value *= 0.1f;
        }
        
        AddMovementInput(Direction, Value);
    }

    AxisInput.Y = Value;
}

bool ADCharacterPlayer::CanMove() const
{
    return MovementState == EMovementState::Grounded || MovementState == EMovementState::InAir;
}

AShootWeapon* ADCharacterPlayer::GetActiveWeapon() const
{
    return ActiveWeapon;
}

int32 ADCharacterPlayer::GetActiveWeaponIndex() const
{
    return ActiveWeaponIndex;
}


void ADCharacterPlayer::StartAim()
{
    if (!CanAim())
    {
        return;
    }

    StopSprint();

    bAimed = true;
    ToggleCrosshairVisible(true);

    StateSwitchToCombat();
    ActiveWeapon->SetWeaponAim(true);
    BP_OnToggleWeaponAim(true);
        
    SetDesiredGait(EMovementGait::Walking);
}

void ADCharacterPlayer::StopAim()
{
    if (!(ActiveWeapon && bAimed))
    {
        return;
    }

    bAimed = false;

    SetDesiredGait(DefaultGait);
    
    ToggleCrosshairVisible(false);

    ActiveWeapon->SetWeaponAim(false);
    BP_OnToggleWeaponAim(false);
    StateSwitchToRelax();
}

void ADCharacterPlayer::ServerSetDesiredGait_Implementation(EMovementGait NewGait)
{
    SetDesiredGait(NewGait);
}

void ADCharacterPlayer::SetDesiredGait(EMovementGait NewGait)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        ServerSetDesiredGait(NewGait);
    }

    Gait = NewGait;

    switch (Gait)
    {
        case EMovementGait::Walking: GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;       break;
        case EMovementGait::Running: GetCharacterMovement()->MaxWalkSpeed = RunSpeed;        break;
        case EMovementGait::Sprinting: GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;   break;
    }
}

void ADCharacterPlayer::ToggleSprint()
{
    if (Gait == EMovementGait::Sprinting)
    {
        StopSprint();
    }
    else
    {
        StartSprint();
    }
}

void ADCharacterPlayer::StartSprint()
{
    float ForwardCosAngle = FVector::DotProduct(GetActorRotation().Vector(), GetVelocity().GetUnsafeNormal2D());
    float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

    if (ForwardDeltaDegree < 45.f)
    {
        // 强制解除战斗状态
        CombatStatusCount = 0;
        StateSwitchToRelaxImmediately();
        
        StopFire();
        StopAim();
        StopReloadMagazine();

        SetDesiredGait(EMovementGait::Sprinting);
    }
}

void ADCharacterPlayer::StopSprint()
{
    if (Gait == EMovementGait::Sprinting)
    {
        SetDesiredGait(DefaultGait);
    }
}

void ADCharacterPlayer::MantleStart()
{
    if (!MantleLimit.IsArrive(GetWorld()->GetTimeSeconds()))
    {
        return;
    }

    if (MovementState == EMovementState::Mantling)
    {
        return;
    }

    HandleMantleStart();
}

void ADCharacterPlayer::HandleMantleStart()
{
    float ZOffset = 2.f;

    FVector CapsuleLocation = GetCapsuleComponent()->GetComponentLocation();
    float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    FVector Offset = CapsuleLocation - GetCapsuleComponent()->GetUpVector() * (CapsuleHalfHeight + ZOffset);
    FRotationMatrix ControlMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f));

    FVector InputVector = AxisInput.X * ControlMatrix.GetUnitAxis(EAxis::X) +
                            AxisInput.Y * ControlMatrix.GetUnitAxis(EAxis::Y);
    
    InputVector.Normalize();

    const FMantleTraceSettings& TraceSettings = UDMProjectSettings::Get()->GetMantleTraceSettings();

    FVector TraceStart = InputVector * -30.f + Offset + FVector(0, 0, (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2);
    FVector TraceEnd = TraceStart + InputVector * TraceSettings.ReachDistance;

    TArray<AActor*> EmptyArray;

    FHitResult HitResult;

    UKismetSystemLibrary::CapsuleTraceSingle(this, TraceStart, TraceEnd,
                                            TraceSettings.ForwardTraceRadius,
                                            (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2,
                                            Collision_TraceType_Climbable,
                                            false,
                                            EmptyArray,
                                            EDrawDebugTrace::None,
                                            HitResult,
                                            true);

    if (!HitResult.bBlockingHit || HitResult.bStartPenetrating || GetCharacterMovement()->IsWalkable(HitResult))
    {
        return;
    }

    FVector InitTraceNormal = HitResult.ImpactNormal;

    FVector WalkableTraceEnd = InitTraceNormal * -15.f + FVector(HitResult.ImpactPoint.X, HitResult.ImpactPoint.Y, Offset.Z);
    FVector WalkableTraceStart = WalkableTraceEnd + FVector(0, 0, TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius + 1.f);

    UKismetSystemLibrary::SphereTraceSingle(this, WalkableTraceStart, WalkableTraceEnd,
                                            TraceSettings.DownwardTraceRadius,
                                            Collision_TraceType_Climbable,
                                            false,
                                            EmptyArray,
                                            EDrawDebugTrace::None,
                                            HitResult,
                                            true);


    if (!HitResult.bBlockingHit || !GetCharacterMovement()->IsWalkable(HitResult))
    {
        return;
    }

    FVector DownTraceLocation = FVector(HitResult.Location.X, HitResult.Location.Y,
                                        HitResult.ImpactPoint.Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + ZOffset);

    float HalfHeight_WithoutHemisphere = GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(Collision_ObjectTrace_Player);

    UKismetSystemLibrary::SphereTraceSingleForObjects(this,
                                            DownTraceLocation + FVector(0,0,HalfHeight_WithoutHemisphere),
                                            DownTraceLocation - FVector(0,0,HalfHeight_WithoutHemisphere),
                                            GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
                                            ObjectTypes,
                                            false,
                                            EmptyArray,
                                            EDrawDebugTrace::None,
                                            HitResult,
                                            true);

    if (HitResult.bBlockingHit || HitResult.bStartPenetrating)
    {
        return;
    }

    // =================== start =====================

    float MantleHeight = (DownTraceLocation - GetActorLocation()).Z;
    
    EMantleType MantleType;

    if (MovementState == EMovementState::InAir)
    {
        MantleType = EMantleType::FallingCatch;
    }
    else
    {
        MantleType = MantleHeight > 125.f ? EMantleType::HighMantle : EMantleType::LowMantle;
    }

    //UKismetSystemLibrary::DrawDebugLine(this, InitTracePoint, InitTracePoint + InitTraceNormal * 100.f, FColor::Red, 10.f, 2);

    const FMantleInformation& MantleInfo = GSProject->GetMantleInfo(MantleType, OverlayState);

    FVector AnimatedOffset = InitTraceNormal * MantleInfo.StartingOffset.Y;

    // 动画开始的偏移位置 (相对于目标位置， 后续会平滑的Lerp这个偏移以修正角色的位置，【为了看起来像是扒在物体的边缘】)
    FVector AnimatedStartOffset = FVector(AnimatedOffset.X, AnimatedOffset.Y, -MantleInfo.StartingOffset.Z);

    
    float StartingPosition = FMath::GetMappedRangeValueClamped(
                                FVector2D(MantleInfo.LowHeight, MantleInfo.HighHeight),
                                FVector2D(MantleInfo.LowStartPosition, MantleInfo.HeightStartPosition), MantleHeight);
    
    float PlayRate = FMath::GetMappedRangeValueClamped(
                                FVector2D(MantleInfo.LowHeight, MantleInfo.HighHeight),
                                FVector2D(MantleInfo.LowPlayRate, MantleInfo.HeightPlayRate), MantleHeight);

    FMantleSpec MantleSpec;
    MantleSpec.MantleType = MantleType;
    MantleSpec.StartingPosition = StartingPosition;
    MantleSpec.PlayRate = PlayRate;
    MantleSpec.MantlingTarget = DownTraceLocation;
    MantleSpec.AnimatedStartOffset = AnimatedStartOffset;
    
    SetMantlingSpec(MantleSpec);
}


void ADCharacterPlayer::MantlingUpdate(float Alpha)
{
    FVector PCC = MantlingCorrectionCurve->GetVectorValue(MantlingSpec.StartingPosition + MantlingTimeline.GetPlaybackPosition());

    float PositionAlpha = PCC.X;
    float XYCorrectionAlpha = PCC.Y;
    float ZCorrectionAlpha = PCC.Z;

    FVector XYLerp = FMath::Lerp(TemporaryActualStartOffset,
                                 FVector(TemporaryAnimatedStartOffset.X,
                                         TemporaryAnimatedStartOffset.Y,
                                         TemporaryActualStartOffset.Z), XYCorrectionAlpha);

    // 修正高度, TemporaryAnimatedStartOffset.Z 是配置中的 -StartingOffset.Z
    float ZLerp = FMath::Lerp(TemporaryActualStartOffset.Z, TemporaryAnimatedStartOffset.Z, ZCorrectionAlpha);


    // 这个是要同时往反方向修正Actor的位置，不然的话一开始就直接跳到目标位置去了
    FVector BlendTarget = FMath::Lerp(TemporaryMantlingTarget + FVector(XYLerp.X, XYLerp.Y, ZLerp), TemporaryMantlingTarget, PositionAlpha);

    // (TemporaryMantlingTarget + TemporaryActualStartOffset) 这个就是当前Actor的位置
    FVector LerpedTargetLocation = FMath::Lerp(TemporaryMantlingTarget + TemporaryActualStartOffset, BlendTarget, Alpha);

    float YawCurveValue = GSProject->GetMantlingRotationCurve()->GetFloatValue(Alpha);

    FRotator LerpedFaceRotation = FMath::Lerp(GetActorRotation(), FaceRotation, YawCurveValue);

    UE_VLOG(this, LogDream, Log, TEXT("PCC: %s"), *PCC.ToString());
    UE_VLOG_LOCATION(this, LogDream, Log, GetActorLocation(), 30.f, FColor::Red, TEXT("FRootMotionSource_Mantling_CurrentLocation"));

    SetActorLocationAndRotation(LerpedTargetLocation, LerpedFaceRotation);
}

void ADCharacterPlayer::OnMantleFinished()
{
    if (IsLocallyControlled())
    {
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    }
}

void ADCharacterPlayer::PlayMantleAnim()
{
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        const FMantleInformation& MantleInfo = GSProject->GetMantleInfo(MantlingSpec.MantleType, OverlayState);
        AnimInstance->Montage_Play(MantleInfo.AnimMontage, MantlingSpec.PlayRate,
                                   EMontagePlayReturnType::Duration, MantlingSpec.StartingPosition);
    }
}

void ADCharacterPlayer::OnRep_MantlingSpec()
{
    const FMantleInformation& MantleInfo = GSProject->GetMantleInfo(MantlingSpec.MantleType, OverlayState);

    FVector CurrentLocation = GetActorLocation();

    MantlingCorrectionCurve = MantleInfo.PositionCurve;
    TemporaryMantlingTarget = MantlingSpec.MantlingTarget;
    TemporaryActualStartOffset =  CurrentLocation - MantlingSpec.MantlingTarget;
    TemporaryAnimatedStartOffset = MantlingSpec.AnimatedStartOffset;
    FaceRotation = (MantlingSpec.MantlingTarget - CurrentLocation).Rotation();
    FaceRotation.Pitch = 0;
    FaceRotation.Roll = 0;
    
    float MinTime;
    float MaxTime;
    MantleInfo.PositionCurve->GetTimeRange(MinTime, MaxTime);
    
    MantlingTimeline.SetTimelineLength(MaxTime - MantlingSpec.StartingPosition);
    MantlingTimeline.SetPlayRate(MantlingSpec.PlayRate);
    MantlingTimeline.PlayFromStart();

    if (IsLocallyControlled())
    {
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
    }
    
    if (GetNetMode() != NM_DedicatedServer)
    {
        //UE_LOG(LogDream, Error, TEXT("OnRep_MantlingSpec"));
        
        PlayMantleAnim();
    }
}

void ADCharacterPlayer::SetMantlingSpec(const FMantleSpec& NewMantleSpec)
{
    MantlingSpec = NewMantleSpec;

    if (GetLocalRole() == ROLE_Authority)
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, MantlingSpec, this);
    }
    else
    {
        ServerSetMantlingSpec(NewMantleSpec);
    }

    // SkipOwner 所以本地客户端不会触发, 这里手动调用一下
    OnRep_MantlingSpec();
}

void ADCharacterPlayer::ServerSetMantlingSpec_Implementation(const FMantleSpec& NewMantleSpec)
{
    SetMantlingSpec(NewMantleSpec);
}

/*void ADCharacterPlayer::ApplyMantleRootMotion(const FMantleTrace& MantleTrace)
{
    const FMantleInformation& MantleInfo = GSProject->GetMantleInfo(MantlingSpec.MantleType, OverlayState);
    
    /*MantlingRMS->Duration = Duration;
    MantlingRMS->AccumulateMode = ERootMotionAccumulateMode::Override;
    MantlingRMS->Settings.SetFlag(ERootMotionSourceSettingsFlags::DisablePartialEndTick);
    MantlingRMS->StartLocation = GetActorLocation();
    MantlingRMS->PathOffsetCurve = MantleInfo.ForceCorrectionCurve;
    MantlingRMS->TargetLocation = MantleTrace.MantlingTarget;
    MantlingRMS->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
    MantlingRMS->FinishVelocityParams.SetVelocity = FVector::ZeroVector;#1#

    FVector Location = GetActorLocation();
    
    MantlingRMS = MakeShared<FRootMotionSource_Mantling>();
    MantlingRMS->Duration = Duration;
    MantlingRMS->FaceRotation = (MantleTrace.MantlingTarget - Location).GetSafeNormal();
    MantlingRMS->AccumulateMode = ERootMotionAccumulateMode::Override;
    MantlingRMS->Settings.SetFlag(ERootMotionSourceSettingsFlags::DisablePartialEndTick);
    MantlingRMS->StartingPosition = MantlingSpec.StartingPosition;
    MantlingRMS->MantlingTarget = MantleTrace.MantlingTarget;
    MantlingRMS->MantlingActualStartOffset = Location - MantleTrace.MantlingTarget;
    MantlingRMS->MantlingAnimatedStartOffset = MantleTrace.AnimatedStartOffset;
    MantlingRMS->PositionCorrectionCurve = MantleInfo.PositionCurve;
    MantlingRMS->PathOffsetCurve = MantleInfo.PathOffsetCurve;
    MantlingRMS->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
    MantlingRMS->FinishVelocityParams.SetVelocity = FVector::ZeroVector;

    MantleRootMotionID = GetCharacterMovement()->ApplyRootMotionSource(MantlingRMS);
}

void ADCharacterPlayer::OnMantleRootMotionFinished()
{
    GetCharacterMovement()->RemoveRootMotionSourceByID(MantleRootMotionID);
}*/

void ADCharacterPlayer::Rolling()
{
    
}

void ADCharacterPlayer::ToggleOverlay()
{
    if (ToggleOverlayLimit.IsArrive(GetWorld()->GetTimeSeconds()))
    {
        OverlayState = OverlayState == EOverlayState::Default ? EOverlayState::Rifle : EOverlayState::Default;
        ServerSetOverlayState(OverlayState);
    }
}

void ADCharacterPlayer::ServerSetOverlayState_Implementation(EOverlayState NewOverlay)
{
    OverlayState = NewOverlay;
}

void ADCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
   
    DOREPLIFETIME(ADCharacterPlayer, CurrentCharacterMesh);
    
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, Gait, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, OverlayState, COND_SkipOwner);

    DOREPLIFETIME_WITH_PARAMS_FAST_PUSH_MODEL(ADCharacterPlayer, MantlingSpec, COND_SkipOwner);
    DOREPLIFETIME_WITH_PARAMS_FAST_PUSH_MODEL(ADCharacterPlayer, ActiveWeapon, COND_OwnerOnly);

    DOREPLIFETIME_CONDITION(ADCharacterPlayer, bCombatStatus, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, ActiveWeaponIndex, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, ControllerRotation, COND_SkipOwner);
}

void ADCharacterPlayer::Destroyed()
{
    Super::Destroyed();

    for (AShootWeapon* Weapon : WeaponInventory)
    {
        if (Weapon)
        {
            Weapon->Destroy();
        }
    }
}

void ADCharacterPlayer::OnDeath(const AActor* Causer)
{
    Super::OnDeath(Causer);
    // 服务器执行
    BP_OnServerDeath(Causer);

    SetReplicatingMovement(false);

    if (ADMPlayerController* PlayerController = GetPlayerController())
    {
        PlayerController->HandlePlayerCharacterDie();
    }

    //TearOff(); // 断开网络同步
}

void ADCharacterPlayer::HealthChanged(const FOnAttributeChangeData& AttrData)
{
    if (PlayerHUD.IsValid())
    {
        if (!FMath::IsNearlyEqual(AttrData.NewValue, CharacterAttributes->GetMaxHealth()))
        {
            if (AttrData.NewValue > AttrData.OldValue)
            {
                PlayerHUD->RecoveryHealth(GetHealthPercent());
            }
            else
            {
                PlayerHUD->ReduceHealth(GetHealthPercent());
            }
        }
    }

    if (IsDeath())
    {
        SetActorTickEnabled(false);
        
        // IsLocallyControlled 不用这个判断  
        if (PlayerHUD.IsValid())
        {
            GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            
            RemovePlayerHUD();
            StopAllActions();

            if (ADMPlayerController* PC = GetPlayerController())
            {
                DisableInput(PC);
            }

            TPCamera->bUsePawnControlRotation = false;
            TPCameraArm->bUsePawnControlRotation = false;
            TPCamera->FieldOfView = 90.f;
            TPCamera->SetRelativeRotation(FRotator(-70.f, 0, 0));
            TPCamera->SetRelativeLocation(FVector(0, 0, 400.f));
            TPCameraArm->SocketOffset = FVector::ZeroVector;

            if (ActiveWeapon)
            {
                ActiveWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
                ActiveWeapon->WeaponMesh->SetSimulatePhysics(true);
            }
        }
    }

    Super::HealthChanged(AttrData);
}

void ADCharacterPlayer::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
    Super::HandleDamage(DamageDone, Handle);
    
    TRIGGER_ABILITY(Condition_Injured, Handle.GetInstigator());

    RecoveryTimer.Reset(UDMAbilitiesStatics::GetHealthRecoveryTime(CharacterAttributes->GetRecovery()));
    
    ClientReceiveDamage(Handle.GetInstigator());
}

void ADCharacterPlayer::ClientReceiveDamage_Implementation(AActor* Causer)
{
    if (Causer && PlayerHUD.IsValid())
    {
        FRotator DeltaRotation = (Causer->GetActorLocation() - GetActorLocation()).Rotation() - GetControlRotation();
        PlayerHUD->ShowHurtMark(FRotator::ClampAxis(DeltaRotation.Yaw) / 360.f);
    }
}

void ADCharacterPlayer::ServerUpdateCharacterMesh_Implementation(UCharacterMesh* CharacterMesh)
{
    CurrentCharacterMesh = CharacterMesh;
}

void ADCharacterPlayer::OnRep_CharacterMesh()
{
    UpdateCharacterMesh();
}

void ADCharacterPlayer::UpdateCharacterMesh()
{
    if (CurrentCharacterMesh == nullptr)
    {
        return;
    }
    
    GetMesh()->SetSkeletalMesh(CurrentCharacterMesh->MasterMesh);
    GetMesh()->SetAnimClass(GSProject->GetMasterAnimClass());

    const TArray<USceneComponent*>& AttachChildren = GetMesh()->GetAttachChildren();
    for (int16 Idx = AttachChildren.Num() - 1; Idx > 0; Idx--)
    {
        if (USkeletalMeshComponent* SkMesh = Cast<USkeletalMeshComponent>(AttachChildren[Idx]))
        {
            if (SkMesh->GetOwner() == this)
            {
                SkMesh->DestroyComponent();
            }
        }
    }
        
    for (USkeletalMesh* SKMesh : CurrentCharacterMesh->SlaveMeshs)
    {
        USkeletalMeshComponent* SKComponent = NewObject<USkeletalMeshComponent>(this);
        SKComponent->RegisterComponent();
        SKComponent->SetSkeletalMesh(SKMesh);
        SKComponent->SetIsReplicated(false);
        SKComponent->SetAnimClass(GSProject->GetSlaveAnimClass());
        SKComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        SKComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SKComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void ADCharacterPlayer::SetCharacterMesh(UCharacterMesh* CharacterMesh)
{
    ServerUpdateCharacterMesh(CharacterMesh);
}

void ADCharacterPlayer::EnableInteractiveButton(float InteractiveTime, FText Desc, FDynamicOnInteractiveCompleted Event)
{
    if (PlayerHUD.IsValid())
    {
        PlayerHUD->ActivateInteractiveButton(InteractiveTime, Desc, FOnInteractiveCompleted::CreateLambda([Event]
        {
            Event.ExecuteIfBound();
        }));
    }
}

void ADCharacterPlayer::DisplayInteractiveButton(FText Desc)
{
    if (PlayerHUD.IsValid())
    {
        PlayerHUD->DisplayInteractiveButton(Desc);
    }
}

void ADCharacterPlayer::DisableInteractiveButton()
{
    if (PlayerHUD.IsValid())
    {
        PlayerHUD->DeactivateInteractiveButton();
    }
}

void ADCharacterPlayer::StopAllActions()
{
    StopFire();
    StopAim();
    StopSprint();
    StopReloadMagazine();
    SetStateToRelax();
}

void ADCharacterPlayer::RecoveryActiveWeaponAmmunition(int32 AmmoNum)
{
    if (ActiveWeapon)
    {
        ActiveWeapon->AmmoNum = FMath::Min(ActiveWeapon->AmmoNum + AmmoNum, ActiveWeapon->InitAmmo);
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
    if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(Spec.Def->UIData))
    {
        if (Spec.GetDuration() > FGameplayEffectConstants::INSTANT_APPLICATION)
        {
            if (FOnActiveGameplayEffectTimeChange* Delegate = AbilitySystem->OnGameplayEffectTimeChangeDelegate(Handle))
            {
                Delegate->AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectTimeChange);
            }
        }
        PlayerHUD->AddStateIcon(BuffUIData->BuffTag, BuffUIData->Icon, Spec.StackCount, Spec.GetDuration());
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration)
{
    //DREAM_LOG(Error, TEXT("NewDuration: %f"), NewDuration);
    if (const FActiveGameplayEffect* ActiveGE = AbilitySystem->GetActiveGameplayEffect(Handle))
    {
        if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(ActiveGE->Spec.Def->UIData))
        {
            PlayerHUD->RefreshStateIcon(BuffUIData->BuffTag, ActiveGE->Spec.StackCount, NewDuration);
        }
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect)
{
    if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(Effect.Spec.Def->UIData))
    {
        PlayerHUD->RemoveStateIcon(BuffUIData->BuffTag);
    }
}

void ADCharacterPlayer::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    EMovementMode MovementMode = GetCharacterMovement()->MovementMode.GetValue();

    /*UE_LOG(LogDream, Error, TEXT("OnMovementModeChanged: %s, %s"),
        *UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(MovementMode));*/

    switch (MovementMode)
    {
    case EMovementMode::MOVE_Walking:
    case EMovementMode::MOVE_NavWalking:
        MovementState = EMovementState::Grounded;
        break;

    case EMovementMode::MOVE_Falling:
        MovementState = EMovementState::InAir;
        break;
    default:
        MovementState = EMovementState::None;
        break;
    }
}

FMinimapDataIterator ADCharacterPlayer::GetMinimapDataIterator() const
{
    return ScanComponent->GetScannedItemsIterator();
}

void ADCharacterPlayer::HitEnemy(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget)
{
    const FGameplayEffectContext* EffectContext = DamageInfo.Handle.Get();

    if (EffectContext->GetScriptStruct() == FDreamGameplayEffectContext::StaticStruct())
    {
        const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContext);

        // 暴击的武器伤害
        if (DreamEffectContext->GetDamageCritical() && DreamEffectContext->GetDamageType() < EDDamageType::Other)
        {
            TRIGGER_ABILITY(Condition_Crit, HitTarget);
        }
    }
    
    if (DamageInfo.bKilled)
    {
        TRIGGER_ABILITY(Condition_KilledEnemy, HitTarget);
    }
    else
    {
        TRIGGER_ABILITY(Condition_HitEnemy, HitTarget);
    }

    ClientHitEnemy(DamageInfo, HitTarget);
}

void ADCharacterPlayer::ClientHitEnemy_Implementation(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget)
{
    const FGameplayEffectContext* EffectContext = DamageInfo.Handle.Get();
    
    if (EffectContext == nullptr)
    {
        return;
    }

    const FHitResult* HitResult = EffectContext->GetHitResult();

    TRIGGER_ABILITY(Condition_HitEnemy_Client, HitTarget);
    
    ShowHitEnemyTips(DamageInfo.bKilled);

    if (EffectContext->GetScriptStruct() == FDreamGameplayEffectContext::StaticStruct())
    {
        const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContext);

        if (HitResult)
        {
            SpawnDamageWidget(HitResult->ImpactPoint, DamageInfo.DamageAmount, DreamEffectContext->GetDamageCritical(), false);
        }
    }

    // 不好使
    /*if (HitResult && HitTarget)
    {
        if (!HitResult->BoneName.IsNone())
        {
            USkeletalMeshComponent* BodyMesh = HitTarget->GetMesh();
            
            BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            BodyMesh->SetAllBodiesSimulatePhysics(true);
            
            /*BodyMesh->SetAllBodiesBelowSimulatePhysics(HitResult->BoneName, true);
            BodyMesh->SetAllBodiesBelowPhysicsBlendWeight(HitResult->BoneName, .5f);#1#
            
            FVector Force = GetActorRotation().Vector() * 50000.f;
            BodyMesh->AddForceToAllBodiesBelow(Force, HitResult->BoneName, true);

            auto Func = [BodyMesh]
            {
                BodyMesh->SetAllBodiesSimulatePhysics(false);
                BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            };

            //FTimerHandle Temporary;
            GetWorldTimerManager().SetTimerForNextTick(Func);
        }
    }*/
}

void ADCharacterPlayer::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BP_OnRepPlayerState();
}

bool ADCharacterPlayer::PickUpMagazine(EAmmoType AmmoType)
{
    if (IsLocallyControlled())
    {
        bool bPickUp = false;
        if (ADMPlayerController* PC = GetPlayerController())
        {
            bPickUp = PC->AddWeaponAmmunition(AmmoType, UDMProjectSettings::Get()->GetPickupAmmunitionAmount());
            UpdateMagazineUI();
        }
        return bPickUp;
    }
    return true;
}

void ADCharacterPlayer::StateSwitchToCombat()
{
    GetWorldTimerManager().ClearTimer(Handle_CombatStatus);
    
    CombatStatusCount++;
    
    if (!bCombatStatus)
    {
        bCombatStatus = true;
        ServerSetCombatState(true);
    }
}


void ADCharacterPlayer::StateSwitchToRelax()
{
    if (bCombatStatus)
    {
        CombatStatusCount = FMath::Max(CombatStatusCount - 1, 0);
        if (CombatStatusCount == 0)
        {
            GetWorldTimerManager().SetTimer(Handle_CombatStatus, this,
                &ADCharacterPlayer::SetStateToRelax, UDMProjectSettings::Get()->GetIdleSwitchingTime());
        }
    }
}

void ADCharacterPlayer::StateSwitchToRelaxImmediately()
{
    if (bCombatStatus)
    {
        CombatStatusCount = FMath::Max(CombatStatusCount - 1, 0);
        if (CombatStatusCount == 0)
        {
            SetStateToRelax();
        }
    }
}

void ADCharacterPlayer::SetStateToRelax()
{
    bCombatStatus = false;
    ServerSetCombatState(false);
}

void ADCharacterPlayer::ServerSetCombatState_Implementation(bool bNewCombatState)
{
    bCombatStatus = bNewCombatState;
}

void ADCharacterPlayer::SetMouseInputScale(int32 Value)
{
    BaseTurnRate = FMath::Min(BaseTurnRate + Value * 3, MAX_MOUSE_SENSITIVITY);
    BaseLookUpRate = FMath::Min(BaseLookUpRate + Value * 3, MAX_MOUSE_SENSITIVITY);
}

bool ADCharacterPlayer::IsLocalCharacter() const
{
    return GetLocalRole() == ROLE_AutonomousProxy || GetNetMode() == NM_Standalone;
}

bool ADCharacterPlayer::IsNotServerCharacter() const
{
    if (GetNetMode() == NM_Standalone)
    {
        return true;
    }

    if (GetLocalRole() == ROLE_SimulatedProxy || GetLocalRole() == ROLE_AutonomousProxy)
    {
        return true;
    }

    return false;
}

FRotator ADCharacterPlayer::GetRepControllerRotation() const
{
    return IsLocalCharacter() ? GetControlRotation() : ControllerRotation.Rotation();
}

void ADCharacterPlayer::SetCameraFieldOfView(float NewFOV) const
{
    TPCamera->SetFieldOfView(NewFOV);
}

void ADCharacterPlayer::CameraAimTransformLerp(float Alpha) const
{
    TPCamera->SetRelativeRotation(FMath::LerpStable(DefaultCameraRotation, AimedCameraRotation, Alpha));
    TPCameraArm->SocketOffset = FMath::LerpStable(DefaultArmSocketOffset, AimedArmSocketOffset, Alpha);
}

bool ADCharacterPlayer::IsUnhealthy() const
{
    if (CharacterAttributes == nullptr)
    {
        return false;
    }

    // death
    if (CharacterAttributes->GetHealth() == 0)
    {
        return false;
    }
    
    return !FMath::IsNearlyEqual(CharacterAttributes->GetHealth(), CharacterAttributes->GetMaxHealth());
}

void ADCharacterPlayer::ToggleCrosshairVisible(bool bVisible)
{
    if (PlayerHUD.IsValid())
    {
        if (bVisible)
        {
            PlayerHUD->SetCrossHairVisibility(EVisibility::SelfHitTestInvisible);
        }
        else
        {
            PlayerHUD->SetCrossHairVisibility(EVisibility::Collapsed);
        }
    }
}

#undef LOCTEXT_NAMESPACE