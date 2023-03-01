// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/DCharacterPlayer.h"
#include "AbilitySystemGlobals.h"
#include "DBaseAttributesAsset.h"
#include "UnrealNetwork.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
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
#include "DmProjectSettings.h"
#include "DMRollingAsset.h"
#include "DMUpgradeAddition.h"
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

#if ENABLE_DRAW_DEBUG

TAutoConsoleVariable<int32> DebugDMCharacterCVar::CVar(
    TEXT("p.dm.characterDebug"),
    0,
    TEXT("Whether to draw debug information.\n")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Cheat);

#endif


FCameraViewLerp FCameraViewLerp::LerpTo(const FCameraViewLerp& Target, float Alpha) const
{
    FCameraViewLerp Temp;
    Temp.ArmLength = FMath::Lerp(ArmLength, Target.ArmLength, Alpha);
    Temp.CameraRotation = FMath::Lerp(CameraRotation, Target.CameraRotation, Alpha);
    Temp.ArmSocketOffset = FMath::Lerp(ArmSocketOffset, Target.ArmSocketOffset, Alpha);
    return Temp;
}

// Sets default values
ADCharacterPlayer::ADCharacterPlayer()
    : SprintSpeed(1000.f)
    , CombatStatusCount(0)
    , DefaultGait(EMovementGait::Running)
    , MovementState(EMovementState::Grounded)
    , MovementAction(EMovementAction::None)
    , OverlayState(EOverlayState::Rifle)
    , bDisableGroundedRotation(false)
    , ActiveWeaponIndex(0)
    , HealthUpdateFrequency(0.1f)
    , RollingLimit(2.f)
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

    WeaponInventory.SetNumZeroed(MAX_WeaponSlot);
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
    // 这里无法判断isLocalController 可能是此时本地的Role还没有设置好

    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    Gait = DefaultGait;

    if (GetLocalRole() != ROLE_Authority)
    {
        AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectAdded);
        AbilitySystem->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectRemoved);
    }

    Timeline_Mantling.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
    
    FOnTimelineFloatStatic OnMantlingUpdate;
    OnMantlingUpdate.BindUObject(this, &ADCharacterPlayer::MantlingUpdate);
    Timeline_Mantling.AddInterpFloat(GSProject->GetMantlePositionCurve(), OnMantlingUpdate);
    
    FOnTimelineEventStatic OnFinished;
    OnFinished.BindUObject(this, &ADCharacterPlayer::OnMantleFinished);
    Timeline_Mantling.SetTimelineFinishedFunc(OnFinished);
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

    TargetRotation = GetActorRotation();
    LastVelocityRotation = TargetRotation;
    LastMovementInputRotation = TargetRotation;

    // [Local]
    if (IsLocalCharacter())
    {
        if (RollingCameraCurve )
        {
            FOnTimelineFloatStatic OnRollingViewLerp;
            OnRollingViewLerp.BindUObject(this, &ADCharacterPlayer::OnRollingViewLerp);
            Timeline_Rolling.AddInterpFloat(RollingCameraCurve, OnRollingViewLerp);
        }
        
        InitializePlayerHUD();

        if (ADMPlayerController* PlayerController = GetPlayerController())
        {
            PlayerController->SetInputMode(FInputModeGameOnly());
        }

        DefaultView.ArmLength = TPCameraArm->TargetArmLength;
        DefaultView.CameraRotation = TPCamera->GetRelativeRotation();
        DefaultView.ArmSocketOffset = TPCameraArm->SocketOffset;

        Handle_Properties = GDataInterface->GetPlayerDataDelegate().OnPropertiesChange.AddUObject(this, &ADCharacterPlayer::OnPlayerPropertiesChanged);
    }

    AbilitySystem->InitAbilityActorInfo(this, this);
}

void ADCharacterPlayer::InitializePlayerHUD()
{
    if (!PlayerHUD.IsValid())
    {
        PlayerHUD = SNew(SPlayerHUD)
                .InOwnerPlayer(this)
                .MinimapDataIterator_UObject(this, &ADCharacterPlayer::GetMinimapDataIterator);
            
        GEngine->GameViewport->AddViewportWidgetContent(PlayerHUD.ToSharedRef(), EWidgetOrder::Player);
    }
}

SPlayerHUD* ADCharacterPlayer::GetOrCreatePlayerHUD()
{
    if (PlayerHUD.IsValid())
    {
        return PlayerHUD.Get();
    }

    InitializePlayerHUD();
    
    return PlayerHUD.Get();
}

void ADCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (Handle_Properties.IsValid())
    {
        GDataInterface->GetPlayerDataDelegate().OnPropertiesChange.Remove(Handle_Properties);
    }
}

void ADCharacterPlayer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateEssentialValues(DeltaSeconds);
    UpdateGroundedRotation(DeltaSeconds);
    
    Timeline_Mantling.TickTimeline(DeltaSeconds);
    
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
    else if (GetLocalRole() == ROLE_AutonomousProxy)
    {
        Timeline_Rolling.TickTimeline(DeltaSeconds);
    }
}

void ADCharacterPlayer::SmoothUpdateRotation(float DeltaSeconds, float RotRate, float RotRateConst, const FRotator& Target)
{
    // [Server/Client]
    TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaSeconds, RotRateConst);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0, TargetRotation.Yaw, 0), DeltaSeconds, RotRate));
}

void ADCharacterPlayer::LimitRotation(float DeltaSeconds, float YawMin, float YawMax, float InterpSpeed)
{
    FRotator Delta = (AimingRotation - GetActorRotation()).GetNormalized();

    if (!UKismetMathLibrary::InRange_FloatFloat(Delta.Yaw, YawMin, YawMax))
    {
        SmoothUpdateRotation(DeltaSeconds, InterpSpeed, 0.f, FRotator(0, AimingRotation.Yaw + (Delta.Yaw > 0.f ? YawMin : YawMax), 0));
    }
}

void ADCharacterPlayer::UpdateEssentialValues(float DeltaSeconds)
{
    AimingRotation = FMath::RInterpTo(AimingRotation, GetReplicatedControlRotation(), DeltaSeconds, 30.f);
    
    FVector CurrentVelocity = GetVelocity();

    Speed = CurrentVelocity.Size2D();
    bIsMoving = Speed > 1.f;

    if (bIsMoving)
    {
        LastVelocityRotation = CurrentVelocity.Rotation();
    }

    Acceleration = (CurrentVelocity - PrevVelocity) / DeltaSeconds;
    PrevVelocity = CurrentVelocity;

    FVector CurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
    MovementInputAmount = CurrentAcceleration.Size() / GetCharacterMovement()->GetMaxAcceleration();
    bHasMovementInput = MovementInputAmount > 0.f;

    if (bHasMovementInput)
    {
        LastMovementInputRotation = CurrentAcceleration.Rotation();
    }

    AimYawRate = FMath::Abs((AimingRotation.Yaw - PrevAimYaw) / DeltaSeconds);
    PrevAimYaw = AimingRotation.Yaw;
    
}

void ADCharacterPlayer::UpdateGroundedRotation(float DeltaSeconds)
{
    if (MovementState != EMovementState::Grounded || MovementAction != EMovementAction::None || bDisableGroundedRotation)
    {
        return;
    }

    if ((bIsMoving && bHasMovementInput || Speed > 150.f) && !HasAnyRootMotion())
    {
        if (bCombatStatus)
        {
            SmoothUpdateRotation(DeltaSeconds, 20.f, 1000.f, AimingRotation);
        }
        else
        {
            float YawValue = Gait == EMovementGait::Sprinting ? LastVelocityRotation.Yaw : AimingRotation.Yaw;
            
            float CurveValue = GSProject->GetRotationRateCurve()->GetFloatValue(GetMappedSpeed());
            float RotationRate = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 300.f), FVector2D(1.f, 3.f), AimYawRate) * CurveValue;
            SmoothUpdateRotation(DeltaSeconds, RotationRate, 1000.0f, FRotator(0,YawValue,0));
        }
    }
    else
    {
        SmoothUpdateRotation(DeltaSeconds, 18.f, 800.f, FRotator(0,AimingRotation.Yaw,0));
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

FVector ADCharacterPlayer::GetInputVector() const
{
    FRotationMatrix ControlMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f));
    return (AxisInput.X * ControlMatrix.GetUnitAxis(EAxis::X) +
                            AxisInput.Y * ControlMatrix.GetUnitAxis(EAxis::Y)).GetSafeNormal();
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

    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ADCharacterPlayer::ToggleWeapon);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADCharacterPlayer::ReloadMagazine);

    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ADCharacterPlayer::StartAim);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ADCharacterPlayer::StopAim);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADCharacterPlayer::ToggleSprint);
    
    PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ADCharacterPlayer::Rolling);
    
    PlayerInputComponent->BindAction("ToggleOverlay", IE_Pressed, this, &ADCharacterPlayer::ToggleOverlay);

    PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &ADCharacterPlayer::ToggleWeaponToFirst);
    PlayerInputComponent->BindAction("Weapon2", IE_Pressed, this, &ADCharacterPlayer::ToggleWeaponToSecond);
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

void ADCharacterPlayer::ToggleWeapon()
{
    HandleToggleWeapon(INDEX_NONE);
}

void ADCharacterPlayer::HandleToggleWeapon(int32 WeaponIndex)
{
    if (!CanEquip())
    {
        return;
    }
    
    StopFire();
    StopAim();
    StopReloadMagazine();

    ActiveWeapon->SetWeaponState(EWeaponState::Equipping);
    
    float Duration = PlayEquippingMontage();
    if (Duration > 0)
    {
        GetWorldTimerManager().SetTimer(Handle_Equip, FTimerDelegate::CreateUObject(
                                    this, &ADCharacterPlayer::ToggleWeaponFinished, WeaponIndex), Duration, false);
    }
    else
    {
        ToggleWeaponFinished(WeaponIndex);
    }
}

void ADCharacterPlayer::ToggleWeaponFinished(int32 WeaponIndex)
{
    ActiveWeapon->SetWeaponEnable(false);
    ActiveWeapon->SetWeaponState(EWeaponState::Idle);
    ServerToggleWeapon(WeaponIndex);
}

void ADCharacterPlayer::ServerToggleWeapon_Implementation(int32 NewWeaponIndex)
{
    if (NewWeaponIndex == INDEX_NONE)
    {
        NewWeaponIndex = (ActiveWeaponIndex + 1) % MAX_WeaponSlot;
    }

    ActiveWeaponIndex = NewWeaponIndex;
    MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, ActiveWeaponIndex, this);


    AShootWeapon* NewWeapon = WeaponInventory[NewWeaponIndex];

    // SetActiveWeapon 之前的武器属性
    const FEquipmentAttributes* PrevWeaponAttr = &ActiveWeapon->GetAttributes();

    if (ActiveWeapon)
    {
        ActiveWeapon->AttachToCharacter(false, GetMesh());
    }

    NewWeapon->AttachToCharacter(true, GetMesh());
        
    SetActiveWeapon(NewWeapon);

    FastRefreshWeaponAttribute(*PrevWeaponAttr);
}

void ADCharacterPlayer::ToggleWeaponToFirst()
{
    if (ActiveWeaponIndex != 0)
    {
        ActiveWeaponIndex = 0;
        HandleToggleWeapon(0);
    }
}

void ADCharacterPlayer::ToggleWeaponToSecond()
{
    if (ActiveWeaponIndex != 1)
    {
        ActiveWeaponIndex = 1;
        HandleToggleWeapon(1);
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
    ActiveWeapon->OnReloadFinished();
    
    ActiveWeapon->SetWeaponState(EWeaponState::Idle);

    UpdateMagazineUI();

    StateSwitchToRelax();

    if (bDesiredShoot)
    {
        StartFire();   
    }
}

void ADCharacterPlayer::EquipWeapon(const FGearDesc& GearDesc)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerEquipWeapon(GearDesc);
    }
    else
    {
        DoServerEquipWeapon(GearDesc);
    }
}

void ADCharacterPlayer::ServerEquipWeapon_Implementation(const FGearDesc& GearDesc)
{
    DoServerEquipWeapon(GearDesc);
}

void ADCharacterPlayer::DoServerEquipWeapon(const FGearDesc& GearDesc)
{
    if (!WeaponInventory.IsValidIndex(GearDesc.EquippedIdx))
    {
        UE_LOG(LogDream, Error, TEXT("EquippedIdx Invalid"));
        return;
    }

    if (ADMPlayerController* PlayerCtrl = GetPlayerController())
    {
        FPlayerDesc& PlayerDesc = PlayerCtrl->GetCachedPlayerDesc();
        if (PlayerDesc.Weapons.IsValidIndex(GearDesc.EquippedIdx))
        {
            PlayerDesc.Weapons[GearDesc.EquippedIdx] = GearDesc;
        }
    }

    FActorSpawnParameters SpawnParam;
    SpawnParam.Owner = this;
    SpawnParam.Instigator = this;
    AShootWeapon* NewWeapon = GetWorld()->SpawnActor<AShootWeapon>(GearDesc.GearClass, SpawnParam);

    float UpgradeAdditionPercentage = 0.f;
    if (GearDesc.GearLevel > 0)
    {
       UpgradeAdditionPercentage = GSProject->GetUpgradeAddition()->GetAdditionStrength(GearDesc.GearQuality, GearDesc.GearLevel);
    }
    
    NewWeapon->InitializeWeaponAttributes(GearDesc.Attributes, UpgradeAdditionPercentage);

    if (AShootWeapon* OldWeapon = WeaponInventory[GearDesc.EquippedIdx])
    {
        OldWeapon->Destroy();
    }
        
    WeaponInventory[GearDesc.EquippedIdx] = NewWeapon;

    if (GearDesc.EquippedIdx == ActiveWeaponIndex)
    {
        // 更改之前的武器属性
        const FEquipmentAttributes* PrevWeaponAttributes = ActiveWeapon ? &ActiveWeapon->GetAttributes() : nullptr;

        NewWeapon->AttachToCharacter(true, GetMesh());
        SetActiveWeapon(NewWeapon);

        // 武器还未初始化时不刷新属性
        if (PrevWeaponAttributes)
        {
            FastRefreshWeaponAttribute(*PrevWeaponAttributes);
        }
    }
    else
    {
        NewWeapon->AttachToCharacter(false, GetMesh());
    }
}

void ADCharacterPlayer::EquipModule(const FGearDesc& GearDesc)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerEquipModule(GearDesc);
    }
    else
    {
        DoServerEquipModule(GearDesc);
        RefreshAttributeBaseValue();
    }
}

void ADCharacterPlayer::DoServerEquipModule(const FGearDesc& GearDesc)
{
    UDModuleBase* Module = NewObject<UDModuleBase>(this, GearDesc.GearClass);
    Module->ModuleAttributes = GearDesc.Attributes;

    uint8 CategoryValue = static_cast<uint8>(Module->Category);
    EquippedModules[CategoryValue] = Module;

    if (ADMPlayerController* PlayerCtrl = GetPlayerController())
    {
        FPlayerDesc& PlayerDesc = PlayerCtrl->GetCachedPlayerDesc();
        if (PlayerDesc.Modules.IsValidIndex(CategoryValue))
        {
            PlayerDesc.Modules[CategoryValue] = GearDesc;
        }
    }
}

void ADCharacterPlayer::ServerEquipModule_Implementation(const FGearDesc& GearDesc)
{
    DoServerEquipModule(GearDesc);
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

        if (ADMPlayerController* PlayerCtrl = GetPlayerController())
        {
            PlayerCtrl->GetCachedPlayerDesc().Talents = Talents;
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
    if (UClass* DamageComponentClass = GSProject->GetDamageWidgetClass())
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
    CharacterAttributes->UpdateAttributesBase(GatherAttributes);
    
    ApplyGearsAbilitiesToSelf(GatherAttributes.Perks);
}

void ADCharacterPlayer::ApplyGearsAbilitiesToSelf(const TArray<int32>& GearPerks)
{
    AbilitySystem->ClearAllAbilities();

    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CUSTOMIZE_TAG(GE_Buff_All)));

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
        for (int32 AbilityGuid : ActiveWeapon->GetAttributes().Perks)
        {
            UClass* PerkClass = GSProject->GetItemClassFromGuid(AbilityGuid);
            CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(PerkClass)));
        }
    }
    
    TRIGGER_ABILITY(Condition_Immediately, this);
}

void ADCharacterPlayer::GatherGearsAttributes(FEquipmentAttributes& GearsAttributes)
{
    if (ActiveWeapon)
    {
        GearsAttributes.MergeAndSkipPerks(ActiveWeapon->GetAttributes());
    }

    for (UDModuleBase* Module : EquippedModules)
    {
        GearsAttributes += Module->ModuleAttributes;
    }

    // Level Base Attrs
    FBaseAttributes BaseAttributes = GSProject->GetBaseAttributes(Level);
    GearsAttributes += BaseAttributes;
}

void ADCharacterPlayer::FastRefreshWeaponAttribute(const FEquipmentAttributes& PrevWeaponAttrs)
{
    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CUSTOMIZE_TAG(GE_Buff_Weapon)));

    for (const FGameplayAbilitySpecHandle& Handle : CacheWeaponPerkHandles)
    {
        AbilitySystem->ClearAbility(Handle);
    }

    const FEquipmentAttributes& Attributes = ActiveWeapon->GetAttributes();
    CacheWeaponPerkHandles.Reset(Attributes.Perks.Num());
    
    for (int32 N = 0; N < Attributes.Perks.Num(); N++)
    {
        UClass* AbilityClass = GSProject->GetItemClassFromGuid(Attributes.Perks[N]);
        CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass)));
    }

    FEquipmentAttributes DiffAttributes = Attributes - PrevWeaponAttrs;

    if (DiffAttributes.IsValidNumericalValue())
    {
        CharacterAttributes->IncrementAttributes(DiffAttributes);
    }

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

    if (IsLocallyControlled())
    {
        if (ActiveWeapon)
        {
            ActiveWeapon->SetWeaponEnable(true);
            
            GetOrCreatePlayerHUD()->SetCrosshairBrush(ActiveWeapon->GetDynamicCrosshairBrush());

            UpdateMagazineUI();
        }
    }
    else if (GetLocalRole() == ROLE_Authority) // 忽略单机模式
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

void ADCharacterPlayer::OnInitializePlayerGears(bool bValidResult, const FPlayerDesc& PlayerInfo)
{
    if (!bValidResult)
    {
        if (ADMPlayerController* PlayerController = GetPlayerController())
        {
            PlayerController->ClientReturnToMainMenuWithTextReason(LOCTEXT("QueryPlayer", "获取玩家数据失败"));
        }

        return;
    }
    
    SetCharacterMesh(PlayerInfo.Skin);

    for (const FGearDesc& PW : PlayerInfo.Weapons)
    {
        DoServerEquipWeapon(PW);
    }

    for (const FGearDesc& PM : PlayerInfo.Modules)
    {
        DoServerEquipModule(PM);
    }

    // todo 天赋分支功能待完善
    TArray<FTalentInfo> Talents;
    GSProject->GetLearnedTalents(ETalentCategory::Warrior, PlayerInfo.Talents, Talents);
    
    for (const FTalentInfo& Talent : Talents)
    {
        LearnedTalents.Add(Talent.TalentClass);
    }

    Level = PlayerInfo.Level;
    
    FEquipmentAttributes GatherAttributes;
    GatherGearsAttributes(GatherAttributes);
    CharacterAttributes->InitAttributes(GatherAttributes);
    ApplyGearsAbilitiesToSelf(GatherAttributes.Perks);
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

    if (ADMPlayerController* PlayerCtrl = GetPlayerController())
    {
        PlayerCtrl->GetCachedPlayerDesc().Level = NewLevel;
    }

    RefreshAttributeBaseValue();
}

bool ADCharacterPlayer::CanReload() const
{
    return ActiveWeapon && ActiveWeapon->CanReload() && MovementAction == EMovementAction::None;
}

bool ADCharacterPlayer::CanEquip() const
{
    return ActiveWeapon && ActiveWeapon->GetWeaponState() < EWeaponState::Equipping && MovementAction == EMovementAction::None;
}

bool ADCharacterPlayer::CanAim() const
{
    return ActiveWeapon && ActiveWeapon->GetWeaponState() <= EWeaponState::Reloading && MovementAction == EMovementAction::None;
}

void ADCharacterPlayer::UpdateMagazineUI() const
{
    if (PlayerHUD.IsValid())
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
    if (Controller != nullptr && CanMove())
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
    if (Controller != nullptr && Value != 0.0f && CanMove())
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
    return MovementState == EMovementState::Grounded &&
            MovementAction == EMovementAction::None &&
            !HasAnyRootMotion();
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

    TPCameraArm->bEnableCameraLag = false;

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

    TPCameraArm->bEnableCameraLag = true;

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
    Gait = NewGait;
    
    if (GetLocalRole() == ROLE_Authority)
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, Gait, this);
    }
    else
    {
        ServerSetDesiredGait(NewGait);
    }

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

    FVector InputVector = GetInputVector();

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

    // 防止攀爬时移动
    MovementState = EMovementState::Mantling;

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
    FVector PCC = MantlingCorrectionCurve->GetVectorValue(MantlingSpec.StartingPosition + Timeline_Mantling.GetPlaybackPosition());

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

#if ENABLE_DRAW_DEBUG

    if (DebugDMCharacterCVar::CVar.GetValueOnAnyThread() == 1)
    {
        FLinearColor DebugLineColor = GetLocalRole() == ROLE_Authority ? FLinearColor::Red :
                                    GetLocalRole() == ROLE_AutonomousProxy ? FLinearColor::Blue : FLinearColor::Green;
    
        UKismetSystemLibrary::DrawDebugLine(this, GetActorLocation(), GetActorLocation() + LerpedFaceRotation.Vector() * 100.f, DebugLineColor, 2.f, 1.f);

        UE_VLOG(this, LogDream, Log, TEXT("PCC: %s"), *PCC.ToString());
        UE_VLOG_LOCATION(this, LogDream, Log, GetActorLocation(), 30.f, FColor::Red, TEXT("FRootMotionSource_Mantling_CurrentLocation"));
    }

#endif

    if (GetLocalRole() == ROLE_Authority)
    {
        SetActorRotation(LerpedFaceRotation);
    }
    else
    {
        SetActorLocationAndRotation(LerpedTargetLocation, LerpedFaceRotation);
    }

    TargetRotation = LerpedFaceRotation;
}

void ADCharacterPlayer::OnMantleFinished()
{
    MovementAction = EMovementAction::None;
    
    if (GetLocalRole() > ROLE_SimulatedProxy)
    {
        if (ActiveWeapon && MantlingSpec.MantleType == EMantleType::HighMantle)
        {
            ActiveWeapon->SetActorHiddenInGame(false);
        }

        UCharacterMovementComponent* MyCharacterMovement = GetCharacterMovement();
            
        if (GetLocalRole() == ROLE_Authority)
        {
            MyCharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = false;
            MyCharacterMovement->bServerAcceptClientAuthoritativePosition = false;
        }

        MyCharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
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
    MantlingCorrectionCurve->GetTimeRange(MinTime, MaxTime);
    
    Timeline_Mantling.SetTimelineLength(MaxTime - MantlingSpec.StartingPosition);
    Timeline_Mantling.SetPlayRate(MantlingSpec.PlayRate);
    Timeline_Mantling.PlayFromStart();

    MovementAction = MantlingSpec.MantleType == EMantleType::LowMantle ? EMovementAction::LowMantle : EMovementAction::HighMantle;

    if (GetLocalRole() != ROLE_Authority)
    {
        PlayMantleAnim();
    }
}

void ADCharacterPlayer::SetMantlingSpec(const FMantleSpec& NewMantleSpec)
{
    // [server/client] call
    
    MantlingSpec = NewMantleSpec;

    UCharacterMovementComponent* MyCharacterMovement = GetCharacterMovement();

    MyCharacterMovement->SetMovementMode(EMovementMode::MOVE_None);

    if (ActiveWeapon && MantlingSpec.MantleType == EMantleType::HighMantle)
    {
        ActiveWeapon->SetActorHiddenInGame(true);
    }

    if (GetLocalRole() == ROLE_Authority)
    {
        MyCharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = true;
        MyCharacterMovement->bServerAcceptClientAuthoritativePosition = true;
        
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

void ADCharacterPlayer::Rolling()
{
    if (RollingLimit.IsArrive(GetWorld()->GetTimeSeconds()))
    {
        StopFire();

        Timeline_Rolling.PlayFromStart();
        
        ERollingDirection RollingDirection;

        if (AxisInput.IsNearlyZero(KINDA_SMALL_NUMBER))
        {
            RollingDirection = ERollingDirection::F;
        }
        else if (FMath::IsNearlyZero(AxisInput.X, KINDA_SMALL_NUMBER))
        {
            RollingDirection = AxisInput.Y > 0 ? ERollingDirection::R : ERollingDirection::L;
        }
        else if (AxisInput.X < 0)
        {
            RollingDirection = AxisInput.Y > KINDA_SMALL_NUMBER ? ERollingDirection::BR :
                               AxisInput.Y == 0 ? ERollingDirection::B : ERollingDirection::BL;
        }
        else
        {
            RollingDirection = AxisInput.Y > KINDA_SMALL_NUMBER ? ERollingDirection::FR :
                               AxisInput.Y == 0 ? ERollingDirection::F : ERollingDirection::FL;
        }

        HandleRolling(RollingDirection);
    }
}

void ADCharacterPlayer::OnRollingFinished()
{
    bDisableGroundedRotation = false;

    if (IsLocallyControlled())
    {
        Timeline_Rolling.Reverse();
    }
}

void ADCharacterPlayer::Multicast_Rolling_Implementation(ERollingDirection NewRollingDirection)
{
    if (!IsLocallyControlled())
    {
        PlayRollingAnimation(NewRollingDirection);
    }
}

void ADCharacterPlayer::OnRollingViewLerp(float Value)
{
    SetCameraView(DefaultView.LerpTo(View_Rolling, Value));
}

void ADCharacterPlayer::PlayRollingAnimation(ERollingDirection NewRollingDirection)
{
    if (UAnimMontage* RollingMontage = GSProject->GetRollingAsset()->GetRollingMontage(NewRollingDirection))
    {
        bDisableGroundedRotation = true;
    
        PlayAnimMontage(RollingMontage);

        float Duration = RollingMontage->SequenceLength / RollingMontage->RateScale;

        GetWorldTimerManager().SetTimer(Handle_Rolling, this, &ADCharacterPlayer::OnRollingFinished, Duration);
    }
}

void ADCharacterPlayer::HandleRolling(ERollingDirection NewRollingDirection)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        Multicast_Rolling(NewRollingDirection);
    }
    else
    {
        Server_Rolling(NewRollingDirection);
    }

    PlayRollingAnimation(NewRollingDirection);
}

void ADCharacterPlayer::Server_Rolling_Implementation(ERollingDirection NewRollingDirection)
{
    HandleRolling(NewRollingDirection);
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
    MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, OverlayState, this);
}

void ADCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams RepParams;
    RepParams.bIsPushBased = true;
   
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, CurrentCharacterMesh, RepParams);

    RepParams.Condition = COND_SkipOwner;
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, MantlingSpec, RepParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, Gait, RepParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, OverlayState, RepParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, bCombatStatus, RepParams);
    
    RepParams.Condition = COND_OwnerOnly;
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, ActiveWeapon, RepParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ADCharacterPlayer, ActiveWeaponIndex, RepParams);
    
    
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
        if (MovementState == EMovementState::Mantling)
        {
            Timeline_Mantling.Stop();
        }
        
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
    SetCharacterMesh(CharacterMesh);
}

void ADCharacterPlayer::SetCharacterMesh(UCharacterMesh* CharacterMesh)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        CurrentCharacterMesh = CharacterMesh;
        MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, CurrentCharacterMesh, this);

        if (ADMPlayerController* PlayerCtrl = GetPlayerController())
        {
            PlayerCtrl->GetCachedPlayerDesc().Skin = CharacterMesh;
        }
    }
    else if (GetLocalRole() == ROLE_AutonomousProxy)
    {
        ServerUpdateCharacterMesh(CharacterMesh);
    }
   
    UpdateCharacterMesh();
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

    // 服务器只需要更新主SKMesh就行
    GetMesh()->SetSkeletalMesh(CurrentCharacterMesh->MasterMesh);
    GetMesh()->SetAnimClass(GSProject->GetMasterAnimClass());

    if (GetNetMode() != NM_DedicatedServer)
    {
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
        ActiveWeapon->AmmoNum = FMath::Min(ActiveWeapon->AmmoNum + AmmoNum, ActiveWeapon->GetInitAmmoNum());
        UpdateMagazineUI();
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
    if (IsLocallyControlled())
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

            // 此时 PlayerHUD 有可能还没创建
            GetOrCreatePlayerHUD()->AddStateIcon(BuffUIData->BuffTag, BuffUIData->Icon, Spec.StackCount, Spec.GetDuration());
        }
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration)
{
    //DREAM_LOG(Error, TEXT("NewDuration: %f"), NewDuration);
    if (const FActiveGameplayEffect* ActiveGE = AbilitySystem->GetActiveGameplayEffect(Handle))
    {
        if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(ActiveGE->Spec.Def->UIData))
        {
            if (PlayerHUD.IsValid())
            {
                PlayerHUD->RefreshStateIcon(BuffUIData->BuffTag, ActiveGE->Spec.StackCount, NewDuration);
            }
        }
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect)
{
    if (IsLocallyControlled())
    {
        if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(Effect.Spec.Def->UIData))
        {
            if (PlayerHUD.IsValid())
            {
                PlayerHUD->RemoveStateIcon(BuffUIData->BuffTag);
            }
        }
    }
}

void ADCharacterPlayer::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    EMovementMode MovementMode = GetCharacterMovement()->MovementMode.GetValue();

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
    MARK_PROPERTY_DIRTY_FROM_NAME(ADCharacterPlayer, bCombatStatus, this);
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

void ADCharacterPlayer::UpdateOverlayDetailID(int32 NewDetailID)
{
    OverlayDetailID = NewDetailID;
}

FRotator ADCharacterPlayer::GetReplicatedControlRotation() const
{
    return IsLocalCharacter() ? GetControlRotation() : ControllerRotation.Rotation();
}

void ADCharacterPlayer::GetCameraLocationAndRotation(FVector& CameraLocation, FRotator& CameraRotation)
{
    CameraLocation = TPCamera->GetComponentLocation();
    CameraRotation = TPCamera->GetComponentRotation();
}

void ADCharacterPlayer::SetCameraFieldOfView(float NewFOV) const
{
    TPCamera->SetFieldOfView(NewFOV);
}

void ADCharacterPlayer::CameraAimTransformLerp(float Alpha)
{
    SetCameraView(DefaultView.LerpTo(View_Aiming, Alpha));
}

void ADCharacterPlayer::SetCameraView(const FCameraViewLerp& NewView)
{
    TPCamera->SetRelativeRotation(NewView.CameraRotation);
    TPCameraArm->SocketOffset = NewView.ArmSocketOffset;
    TPCameraArm->TargetArmLength = NewView.ArmLength;
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