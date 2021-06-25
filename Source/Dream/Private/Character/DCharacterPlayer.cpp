// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable All
#include "Character/DCharacterPlayer.h"
#include "AbilitySystemGlobals.h"
#include "UnrealNetwork.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "JsonUtilities.h"
#include "DreamGameMode.h"
#include "Engine.h"
#include "DGameplayStatics.h"
#include "DEnemyBase.h"
#include "DGameplayEffectUIData.h"
#include "DGameplayTags.h"
#include "DreamGameplayAbility.h"
#include "DreamAttributeSet.h"
#include "DreamGameplayType.h"
#include "MiniMapDataComponent.h"
#include "MiniMapDataInterface.h"
#include "Components/DamageWidgetComponent.h"
#include "Props/Gear/GearBase.h"
#include "Props/Weapon/ShootWeapon.h"
#include "UI/PlayerHUD.h"
#include "GameplayEffectTypes.h"
#include "PlayerDataInterfaceModule.h"
#include "DModuleBase.h"
#include "DPlayerCameraManager.h"
#include "GameplayEffectExtension.h"

// Sets default values
ADCharacterPlayer::ADCharacterPlayer()
    : BaseTurnRate(45.f)
    , BaseLookUpRate(45.f)
    , SprintSpeed(1000.f)
    , AimMoveSpeed(200.f)
    , NormalSpeed(600.f)
    , OutOfCombatTime(3.f)
    , ScanInterval(1.f)
    , ActiveWeaponIndex(0)
{

    MinRotateLimit = -60.f;
    MaxRotateLimit = 60.f;
    
    PrimaryActorTick.bCanEverTick = true;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    CharacterCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CharacterCapture"));
    CharacterCapture->SetHiddenInGame(true);
    CharacterCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    CharacterCapture->SetupAttachment(RootComponent);

    CaptureLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("CaptureLight"));
    CaptureLight->LightingChannels.bChannel0 = false;
    CaptureLight->LightingChannels.bChannel1 = true;
    CaptureLight->LightingChannels.bChannel2 = false;
    CaptureLight->SetupAttachment(RootComponent);

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

    GetCapsuleComponent()->SetCollisionResponseToChannel(
        Collision_ObjectType_Projectile, ECollisionResponse::ECR_Ignore);

    GetCharacterMovement()->bOrientRotationToMovement = false;

    // 初始化武器插槽
    WeaponInventory.AddZeroed(2);
    LocalEquippedWeaponClass.AddZeroed(2);
    EquippedModules.AddZeroed(4);
}

void ADCharacterPlayer::PossessedBy(AController* InController)
{
    // 网络模式下  本地客户端不执行此函数  只有服务器会执行此函数
    Super::PossessedBy(InController);
}

void ADCharacterPlayer::InitializeUI()
{
    if (!PlayerHUD)
    {
        PlayerHUD = CreateWidget<UPlayerHUD>(GetWorld(), PlayerHUDClass);
        PlayerHUD->SetHealth(GetHealth(), 1.f);
        PlayerHUD->SetShield(GetShield(), 1.f);
        PlayerHUD->SetCrossHairVisibility(ESlateVisibility::Collapsed);
        PlayerHUD->AddToViewport(EWidgetOrder::Player);
    }
}

void ADCharacterPlayer::BeginPlay()
{
    NormalSpeed = GetCharacterMovement()->MaxWalkSpeed;

    if (IsLocallyControlled())
    {
        DefaultCameraRotation = TPCamera->GetRelativeRotation();
        DefaultArmSocketOffset = TPCameraArm->SocketOffset;
        
        GetWorldTimerManager().SetTimer(Handle_RadarScan, this, &ADCharacterPlayer::RadarScanTick, ScanInterval, true, 0.f);
        
        if (APlayerController* PC = GetPlayerController())
        {
            PC->SetInputMode(FInputModeGameOnly());
        }
        
        if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
        {
            PDS->OnGetWeaponComplete.AddUObject(this, &ADCharacterPlayer::OnInitPlayer);
            PDS->GetPlayerWeapons(EGetWeaponCondition::Equipped);
        }

        AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectAdded);
        AbilitySystem->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectRemoved);

        InitializeUI();
    }

    Super::BeginPlay();

    AbilitySystem->InitAbilityActorInfo(this, this);

    if (GetLocalRole() == ROLE_Authority)
    {
        for (TSubclassOf<UGameplayAbility> Ability : DefaultAbilitySlot)
        {
            AbilitySystem->GiveAbility(FGameplayAbilitySpec(Ability));
        }
        
#if WITH_EDITOR

        if (TestInitAttributes)
        {
            AttributeSet->InitFromMetaDataTable(TestInitAttributes);
        }
        
#endif
    }
}

void ADCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ADCharacterPlayer::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    // 网络模式下  本地客户端不执行此方法
}

float ADCharacterPlayer::PlayMontage(UAnimMontage* PawnAnim, UAnimMontage* WeaponAnim)
{
    float Duration = .1f;

    if (IsRunningDedicatedServer())
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

        if (WeaponAnim)
        {
            ActiveWeapon->WeaponMesh->AnimScriptInstance->Montage_Play(WeaponAnim);
        }
    }

    return Duration;
}

ADPlayerController* ADCharacterPlayer::GetPlayerController() const
{
    return Cast<ADPlayerController>(Controller);
}

// Called to bind functionality to input
void ADCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    /*AbilitySystem->BindAbilityActivationToInputComponent(PlayerInputComponent,
        FGameplayAbilityInputBinds(TEXT("Confirm"), TEXT("Cancel"), TEXT("AbilityBind")));*/
    
    PlayerInputComponent->BindAxis("MoveForward", this, &ADCharacterPlayer::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ADCharacterPlayer::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ADCharacterPlayer::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &ADCharacterPlayer::LookUpAtRate);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ADCharacterPlayer::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ADCharacterPlayer::StopJump);

    /*PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ADCharacterPlayer::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ADCharacterPlayer::StopCrouch);*/
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ADCharacterPlayer::SwitchCrouch);

    PlayerInputComponent->BindAction("OpenFire", IE_Pressed, this, &ADCharacterPlayer::StartFire);
    PlayerInputComponent->BindAction("OpenFire", IE_Released, this, &ADCharacterPlayer::StopFire);

    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ADCharacterPlayer::EquipWeapon);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADCharacterPlayer::ReloadMagazine);

    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ADCharacterPlayer::StartAim);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ADCharacterPlayer::StopAim);
    //PlayerInputComponent->BindAction("SwitchCombat", IE_Released, this, &ADCharacterPlayer::SwitchCombat);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADCharacterPlayer::ToggleSprint);
    //PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ADCharacterPlayer::StopSprint);
}

void ADCharacterPlayer::StartFire()
{
    bFireButtonDown = true;

    if (!ActiveWeapon)
    {
        return;
    }

    bool bCannotFire = GetWorld()->TimeSeconds <= (ActiveWeapon->GetLastFireTimeSeconds() + ActiveWeapon->GetFireInterval());
    if (WeaponStatus > EWeaponStatus::Firing || (bCannotFire && ActiveWeapon->FireMode != EFireMode::Accumulation))
    {
        return;
    }

    ActiveWeapon->BP_OnStartFire();

    bool PrevCombatStatus = bCombatStatus;

    StopSprint();
    SetCombatStatus(true);
    WeaponStatus = EWeaponStatus::Firing;

    if (!PrevCombatStatus)
    {
        GetWorldTimerManager().SetTimer(Handle_Shoot, this, &ADCharacterPlayer::ConfirmFire, 0.1f, false);
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
        {
            HandleFire();
        }
        break;
    case EFireMode::FullyAutomatic:
        {
            GetWorldTimerManager().SetTimer(Handle_Shoot, this,
                &ADCharacterPlayer::HandleFire, ActiveWeapon->GetFireInterval(), true, 0.f);
        }
        break;
    case EFireMode::Accumulation:
        {
            GetWorldTimerManager().SetTimer(Handle_Shoot, this,
                &ADCharacterPlayer::HandleFire, ActiveWeapon->GetFireInterval(), false);
        }
        break;
    }
}

void ADCharacterPlayer::HandleFire()
{
    if (WeaponStatus > EWeaponStatus::Firing)
    {
        return;
    }

    if (ActiveWeapon->AmmoNum == 0)
    {
        ReloadMagazine();
        return;
    }

    TriggerAbilityFromTag(ConditionTags::Condition_Firing, this);

    ActiveWeapon->HandleFire();
    UpdateAmmoUI();

    if (ActiveWeapon->FireCameraShake)
    {
        GetPlayerController()->ClientStartCameraShake(ActiveWeapon->FireCameraShake);
    }
}

void ADCharacterPlayer::StopFire()
{
    bFireButtonDown = false;
    
    if (!ActiveWeapon)
    {
        return;
    }

    ActiveWeapon->BP_OnStopFire();

    if (WeaponStatus == EWeaponStatus::Firing)
    {
        AttemptSetStatusToRelax(); 
        WeaponStatus = EWeaponStatus::Idle;
    }

    if (ActiveWeapon->FireMode != EFireMode::SemiAutomatic && Handle_Shoot.IsValid())
    {
        GetWorldTimerManager().ClearTimer(Handle_Shoot);
    }
}

void ADCharacterPlayer::EquipWeapon()
{
    if (!CanEquip())
    {
        return;
    }

    ServerEquipWeapon();

    WeaponStatus = EWeaponStatus::Equipping;
    
    if (bCombatStatus)
    {
        StopFire();
        StopAim();
        HandleStopReload();

        ActiveWeapon->SetWeaponEnable(false);

        float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
        GetWorldTimerManager().SetTimer(Handle_Equip, this, &ADCharacterPlayer::EquipmentFinished, Duration);
    }
    else
    {
        EquipmentFinished();
    }
}

void ADCharacterPlayer::MulticastEquipWeapon_Implementation()
{
    if (GetLocalRole() != ROLE_SimulatedProxy)
    {
        return;
    }

    float MontageDuration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
}

void ADCharacterPlayer::ServerEquipWeapon_Implementation()
{
    if (bCombatStatus)
    {
        MulticastEquipWeapon();
        float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
        GetWorldTimerManager().SetTimer(Handle_Equip, this, &ADCharacterPlayer::EquipmentFinished, Duration);
    }
    else
    {
        EquipmentFinished();
    }
}

void ADCharacterPlayer::EquipmentFinished()
{
    //ActiveWeapon->SetActorHiddenInGame(true);
    if (IsLocallyControlled())
    {
        //ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("root"));
        WeaponStatus = EWeaponStatus::Idle;
    }

    if (GetLocalRole() == ROLE_Authority)
    {
        SwitchWeapon(ActiveWeaponIndex == (WeaponInventory.Num() - 1) ? 0 : ActiveWeaponIndex + 1);
    }
}

void ADCharacterPlayer::ReloadMagazine()
{
    if (CanReload())
    {
        StopFire();

        SetCombatStatus(true);
        WeaponStatus = EWeaponStatus::Reloading;

        ServerReloadMagazine();

        float Duration = PlayMontage(GetCurrentActionMontage()->ReloadAnim, ActiveWeapon->ReloadAnim);
        GetWorldTimerManager().SetTimer(Handle_Reload, this, &ADCharacterPlayer::ReloadFinished, Duration);
    }
}

void ADCharacterPlayer::HandleStopReload()
{
    if (Handle_Reload.IsValid())
    {
        AttemptSetStatusToRelax();
        StopAnimMontage(GetCurrentActionMontage()->ReloadAnim);
        GetWorldTimerManager().ClearTimer(Handle_Reload);
    }
}

void ADCharacterPlayer::ReloadFinished()
{
    ADPlayerController* PC = GetPlayerController();
    check(PC);

    int32 ActualLoadingAmmo = ActiveWeapon->DAmmoNum - ActiveWeapon->AmmoNum;
    int32 TotalAmmo = PC->GetWeaponAmmunition(ActiveWeapon->AmmoType);

    int32 NewTotal = 0;

    // update remain magazine
    if (TotalAmmo <= ActualLoadingAmmo)
    {
        ActualLoadingAmmo = TotalAmmo;
    }
    else
    {
        NewTotal = TotalAmmo - ActualLoadingAmmo;
    }

    ActiveWeapon->AmmoNum += ActualLoadingAmmo;
    PC->SetWeaponAmmunition(ActiveWeapon->AmmoType, NewTotal);
    UpdateAmmoUI();
    WeaponStatus = EWeaponStatus::Idle;

    if (bFireButtonDown)
    {
        StartFire();
    }

    AttemptSetStatusToRelax();
}

void ADCharacterPlayer::ServerReloadMagazine_Implementation()
{
    TriggerAbilityFromTag(ConditionTags::Condition_Reloading, this);
    MulticastReloadMagazine();
}

void ADCharacterPlayer::MulticastReloadMagazine_Implementation()
{
    if (GetLocalRole() == ROLE_Authority || IsLocallyControlled())
    {
        return;
    }

    PlayMontage(GetCurrentActionMontage()->ReloadAnim, ActiveWeapon->ReloadAnim);
}

/*void ADCharacterPlayer::OnRep_CombatStatus()
{
    if (IsLocallyControlled())
    {
        return;
    }
    
    SetCombatStatus(bCombatStatus);
}*/

void ADCharacterPlayer::OnRep_ActiveWeapon()
{
    SetActiveWeapon(ActiveWeapon);
}

void ADCharacterPlayer::SetInventoryWeapon(int32 Index, TSubclassOf<AShootWeapon> NewWeaponClass)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        LocalEquippedWeaponClass[Index] = NewWeaponClass;
        ServerSetInventoryWeapon(Index, NewWeaponClass);
        return;
    }

    if (Index >= 0 && Index < WeaponInventory.Num())
    {
        FActorSpawnParameters SpawnParam;
        SpawnParam.Owner = this;
        SpawnParam.Instigator = this;
        AShootWeapon* NewWeapon = GetWorld()->SpawnActor<AShootWeapon>(NewWeaponClass, SpawnParam);

        if (AShootWeapon* OldWeapon = WeaponInventory[Index])
        {
            OldWeapon->Destroy();
        }
        
        WeaponInventory[Index] = NewWeapon;

        if (Index == ActiveWeaponIndex)
        {
            NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
            NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
            SetActiveWeapon(NewWeapon);
        }
        else
        {
            // NewWeapon->SetActorHiddenInGame(true);
            NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponHolsterSocketName);
            NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponHolsterSocketOffset);
        }
    }
    else
    {
        DREAM_NLOG(Error, TEXT("SetInventoryWeapon Fatal Error"));
    }
}

void ADCharacterPlayer::ServerSetInventoryWeapon_Implementation(int32 Index, TSubclassOf<AShootWeapon> NewWeaponClass)
{
    SetInventoryWeapon(Index, NewWeaponClass);
}

const TArray<TSubclassOf<AShootWeapon>>& ADCharacterPlayer::GetLocalEquippedWeaponClass() const
{
    return LocalEquippedWeaponClass;
}

void ADCharacterPlayer::EquippedModule(int32 Index, TSubclassOf<UDModuleBase> ModuleClass)
{
    UDModuleBase* Module = ModuleClass->GetDefaultObject<UDModuleBase>();
    
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerEquippedModule(Index, ModuleClass);
    }
    else
    {
        FEquipmentAttributes& PrevAttrs = FEquipmentAttributes::EmptyAttributes;
        
        if (TSubclassOf<UDModuleBase> PrevModule = EquippedModules[Index])
        {
            UDModuleBase* PrevModuleCDO = PrevModule->GetDefaultObject<UDModuleBase>();
            PrevAttrs = PrevModuleCDO->ModuleAttributes;
        }

        UpdateCharacterAttributes(PrevAttrs, Module->ModuleAttributes);
    }

    EquippedModules[Index] = ModuleClass;
}

TArray<TSubclassOf<UDModuleBase>> ADCharacterPlayer::GetModules() const
{
    return EquippedModules;
}

void ADCharacterPlayer::ServerEquippedModule_Implementation(int32 Index, TSubclassOf<UDModuleBase> ModuleClass)
{
    EquippedModule(Index, ModuleClass);
}

void ADCharacterPlayer::SwitchWeapon(int32 NewWeaponIndex)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }
    
    if (NewWeaponIndex >= WeaponInventory.Num())
    {
        return;
    }

    AShootWeapon* NewWeapon = WeaponInventory[NewWeaponIndex];

    ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponHolsterSocketName);
    ActiveWeapon->SetActorRelativeTransform(ActiveWeapon->WeaponHolsterSocketOffset);
    
    ActiveWeaponIndex = NewWeaponIndex;
    NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
    NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
    SetActiveWeapon(NewWeapon);
}

void ADCharacterPlayer::AddInfiniteActors(const TArray<AActor*>& TargetActors)
{
    InfiniteActors.Append(TargetActors);
}

void ADCharacterPlayer::AddInfiniteActor(AActor* TargetActor)
{
    InfiniteActors.Add(TargetActor);
}

void ADCharacterPlayer::RemoveInfiniteActor(AActor* TargetActor)
{
    InfiniteActors.Remove(TargetActor); 
}

void ADCharacterPlayer::ClearInfiniteActors()
{
    InfiniteActors.Reset();
}

void ADCharacterPlayer::ActivateCharacterCapture()
{
    SetMeshLightChannel(1);
    CharacterCapture->ShowOnlyActorComponents(this);
    ResetCharacterCapture();
    CharacterCapture->SetHiddenInGame(false);
}

void ADCharacterPlayer::DeactivateCharacterCapture()
{
    SetMeshLightChannel(0);
    CharacterCapture->ClearShowOnlyComponents();
    CharacterCapture->SetHiddenInGame(true);
}

void ADCharacterPlayer::ResetCharacterCapture()
{
    const TArray<USceneComponent*>& Components = GetMesh()->GetAttachChildren();

    for (USceneComponent* Component : Components)
    {
        if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
        {
            if (AActor* ComponentOwner = PrimitiveComponent->GetOwner())
            {
                CharacterCapture->ShowOnlyActorComponents(ComponentOwner, true);
            }
        }
    }
}

void ADCharacterPlayer::SpawnDamageWidget(const FVector& Location, float DamageValue, bool bCritical, bool bIsHealthSteal)
{
    UDamageWidgetComponent* DamageWidget = NewObject<UDamageWidgetComponent>(this, DamageWidgetClass);
    float RandomZ = UKismetMathLibrary::RandomFloatInRange(-50.f, 50.f);
    
    DamageWidget->SetWorldLocation(Location + FVector(0, 0, RandomZ));
    DamageWidget->RegisterComponent();
    DamageWidget->ActiveComponent(DamageValue, bCritical, bIsHealthSteal);
}

void ADCharacterPlayer::ShowHitEnemyTips(bool bEnemyDeath)
{
    if (PlayerHUD)
    {
        PlayerHUD->ShowHitEnemyTips(bEnemyDeath);
    }
}

float ADCharacterPlayer::GetCtrlYawDeltaCount() const
{
    return CtrlYawDeltaCount;
}

void ADCharacterPlayer::UpdateCharacterAttributes(const FEquipmentAttributes& OldAttrs, const FEquipmentAttributes& NewAttrs)
{
    AbilitySystem->ApplyModToAttribute(DreamAttrStatics().AttackPowerProperty,
        EGameplayModOp::Additive, NewAttrs.AttackPower - OldAttrs.AttackPower);
    
    AbilitySystem->ApplyModToAttribute(DreamAttrStatics().HealthStealPercentageProperty,
        EGameplayModOp::Additive, NewAttrs.HealthSteal - OldAttrs.HealthSteal);

    AbilitySystem->ApplyModToAttribute(DreamAttrStatics().DefensePowerProperty,
        EGameplayModOp::Additive, NewAttrs.Defense - OldAttrs.Defense);
    
    AbilitySystem->ApplyModToAttribute(DreamAttrStatics().CriticalRateProperty,
        EGameplayModOp::Additive, NewAttrs.CriticalRate - OldAttrs.CriticalRate);
    
    AbilitySystem->ApplyModToAttribute(DreamAttrStatics().CriticalRateProperty,
        EGameplayModOp::Additive, NewAttrs.CriticalDamage - OldAttrs.CriticalDamage);

    for (TSubclassOf<UDreamGameplayAbility> Ability : OldAttrs.Perks)
    {
        if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromClass(Ability))
        {
            AbilitySystem->ClearAbility(AbilitySpec->Handle);
        }
    }

    for (TSubclassOf<UDreamGameplayAbility> Ability : NewAttrs.Perks)
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(Ability));
    }

    TriggerAbilityFromTag(ConditionTags::Condition_Immediately, this);
}

void ADCharacterPlayer::SetPlayerHUDVisible(bool bVisible)
{
    if (PlayerHUD)
    {
        PlayerHUD->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

EMovingDirection ADCharacterPlayer::GetMovingDirection() const
{
    if (MovingInput.Y > 0.f)
    {
        if (MovingInput.X != 0.f)
        {
            return MovingInput.X > 0 ? EMovingDirection::FR : EMovingDirection::FL;
        }
    }
    else if (MovingInput.Y < 0.f)
    {
        if (MovingInput.X != 0.f)
        {
            return MovingInput.X > 0 ? EMovingDirection::BR : EMovingDirection::BL;
        }
    
        return EMovingDirection::B;
    }
    else
    {
        if (MovingInput.X != 0.f)
        {
            return MovingInput.X > 0 ? EMovingDirection::R : EMovingDirection::L;
        }
    }

    return EMovingDirection::F;
}

void ADCharacterPlayer::SetActiveWeapon(AShootWeapon* NewWeapon)
{
    bool bAuthority = GetLocalRole() == ROLE_Authority;
    bool bIsLocallyControlled = IsLocallyControlled();

    if (bAuthority)
    {
        UpdateCharacterAttributes(ActiveWeapon ? ActiveWeapon->WeaponAttribute : FEquipmentAttributes::EmptyAttributes, NewWeapon->WeaponAttribute);
    }

    ActiveWeapon = NewWeapon;

    /*if (NewWeapon == nullptr)
    {
        return;
    }*/

    //BP_OnActiveWeaponChanged();

    if (bIsLocallyControlled)
    {
        ActiveWeapon->SetWeaponEnable(true);
        PlayerHUD->SetCrossHairBrush(ActiveWeapon->GetDynamicCrosshairBrush());
        PlayerHUD->SetMagazineBrush(ActiveWeapon->GetDynamicMagazineBrush());
        UpdateAmmoUI();
    }

    /*if (bIsLocallyControlled || bAuthority)
    {
        
    }*/
}

void ADCharacterPlayer::OnInitPlayer(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage)
{
    FPlayerDataInterfaceModule::Get()->OnGetWeaponComplete.RemoveAll(this);
    
    if (!ErrorMessage.IsEmpty())
    {
        //BP_GetPlayerWeapons(Weapons);
        DREAM_LOG(Error, TEXT("GetWeapons Fail"));
        GetPlayerController()->ClientReturnToMainMenuWithTextReason(FText::FromString(ErrorMessage));
        return;
    }

    for (FPlayerWeapon Weapon : Weapons)
    {
        UClass* WeaponClass = LoadClass<AShootWeapon>(this, *Weapon.WeaponClass);
        SetInventoryWeapon(Weapon.EquipmentIndex, WeaponClass);
    }
}

bool ADCharacterPlayer::CanShoot() const
{
    return ActiveWeapon && WeaponStatus < EWeaponStatus::Reloading && ActiveWeapon->AmmoNum > 0;
}

bool ADCharacterPlayer::CanReload() const
{
    return ActiveWeapon && WeaponStatus < EWeaponStatus::Reloading && GetWeaponAmmunition() > 0 && ActiveWeapon->AmmoNum < ActiveWeapon->DAmmoNum;
}

bool ADCharacterPlayer::CanEquip() const
{
    return WeaponStatus < EWeaponStatus::Equipping;
}

bool ADCharacterPlayer::CanAim() const
{
    return ActiveWeapon && WeaponStatus <= EWeaponStatus::Reloading;
}

int32 ADCharacterPlayer::GetWeaponAmmunition() const
{
    if (ADPlayerController* PC = GetPlayerController())
    {
        if (ActiveWeapon)
        {
            return PC->GetWeaponAmmunition(ActiveWeapon->AmmoType);
        }
    }

    return 0;
}

void ADCharacterPlayer::UpdateAmmoUI()
{
    if (PlayerHUD && ActiveWeapon)
    {
        PlayerHUD->SetMagazineInfo(ActiveWeapon->AmmoNum, GetWeaponAmmunition(), ActiveWeapon->GetRemainAmmoPercent());
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
    if ((Controller != NULL) && (Value != 0.0f))
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        // get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
    if (Value == 0.f && bSprinted)
    {
        StopSprint();
    }

    MovingInput.Y = Value;
}

void ADCharacterPlayer::MoveRight(float Value)
{
    if ((Controller != NULL) && (Value != 0.0f) && !bSprinted)
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        // get right vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddControllerYawInput(Value * GetWorld()->GetDeltaSeconds());
        AddMovementInput(Direction, Value);
    }

    MovingInput.X = Value;
}

AShootWeapon* ADCharacterPlayer::GetActiveWeapon() const
{
    return ActiveWeapon;
}

void ADCharacterPlayer::StartAim()
{
    if (CanAim())
    {
        StopSprint();

        bAimed = true;
        ToggleCrossHairVisible(true);

        SetCombatStatus(true);
        ActiveWeapon->SetWeaponAim(true);
        BP_OnToggleWeaponAim(true);

        AimedMoveSpeedChange(true);

        ServerStartAim();
    }
}


void ADCharacterPlayer::ServerStartAim_Implementation()
{
    AimedMoveSpeedChange(true);
}

void ADCharacterPlayer::StopAim()
{
    if (!ActiveWeapon)
    {
        return;
    }
    
    if (bAimed)
    {
        ServerStopAim();

        bAimed = false;
        ToggleCrossHairVisible(false);

        ActiveWeapon->SetWeaponAim(false);
        BP_OnToggleWeaponAim(false);
        AimedMoveSpeedChange(false);
        
        AttemptSetStatusToRelax();
    }
}

void ADCharacterPlayer::SwitchCombat()
{
    
}

void ADCharacterPlayer::ServerStopAim_Implementation()
{
    AimedMoveSpeedChange(false);
}

void ADCharacterPlayer::AimedMoveSpeedChange(bool bNewAim)
{
    if (bNewAim)
    {
        GetCharacterMovement()->MaxWalkSpeed = AimMoveSpeed;
    }
    else
    {
        float DefaultSpeed = Cast<ADCharacterPlayer>(GetClass()->ClassDefaultObject)->GetCharacterMovement()-> MaxWalkSpeed;
        GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
    }
}

void ADCharacterPlayer::ToggleSprint()
{
    if (bSprinted)
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
        SetCombatStatus(false);
        WeaponStatus = EWeaponStatus::Idle;
        
        StopFire();
        StopAim();
        StopCrouch();
        HandleStopReload();

        bSprinted = true;
        SprintMoveSpeedChange(bSprinted);
        ServerStartSprint();
    }
}

void ADCharacterPlayer::StopSprint()
{
    if (bSprinted)
    {
        bSprinted = false;
        SprintMoveSpeedChange(bSprinted);
        ServerStopSprint();
    }
}

void ADCharacterPlayer::SprintMoveSpeedChange(bool bNewSprint)
{
    if (bNewSprint)
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
    }
}

void ADCharacterPlayer::ServerStartSprint_Implementation()
{
    bSprinted = true;
    SprintMoveSpeedChange(bSprinted);
}

void ADCharacterPlayer::ServerStopSprint_Implementation()
{
    bSprinted = false;
    SprintMoveSpeedChange(bSprinted);
}

void ADCharacterPlayer::StartJump()
{
    Jump();
}

void ADCharacterPlayer::StopJump()
{
    StopJumping();
}

void ADCharacterPlayer::SwitchCrouch()
{
    if (bIsCrouched)
    {
        StopCrouch();
    }
    else
    {
        StartCrouch();
    }
}

void ADCharacterPlayer::StartCrouch()
{
    StopSprint();
    Crouch();
}

void ADCharacterPlayer::StopCrouch()
{
    UnCrouch();
}

void ADCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADCharacterPlayer, ActiveWeapon);
    DOREPLIFETIME(ADCharacterPlayer, bCombatStatus);
    DOREPLIFETIME(ADCharacterPlayer, bSprinted);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, RemoteCtrlRotation, ELifetimeCondition::COND_SkipOwner);
}

void ADCharacterPlayer::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);
    RemoteCtrlRotation = GetControlRotation();
}

void ADCharacterPlayer::Destroyed()
{
    Super::Destroyed();

    if (Handle_RadarScan.IsValid())
    {
        GetWorldTimerManager().ClearTimer(Handle_RadarScan);
    }

    if (WeaponInventory.Num() > 0)
    {
        for (AShootWeapon* Weapon : WeaponInventory)
        {
            if (Weapon && !Weapon->IsPendingKill())
            {
                Weapon->Destroy();
            }
        }
    }
}

void ADCharacterPlayer::OnDeath(const AActor* Causer)
{
    Super::OnDeath(Causer);
    // 服务器执行
    BP_OnServerDeath(Causer);

    ADreamGameMode* GameMode = GetWorld()->GetAuthGameMode<ADreamGameMode>();
    float PlayerResurrectionTime = GameMode->GetPlayerResurrectionTime();
    ClientResurrection(PlayerResurrectionTime);

    FTimerHandle Handle_Temp;
    GetWorldTimerManager().SetTimer(
        Handle_Temp,
        [this]
        {
            FTransform CurrTransform = GetActorTransform();
            ADPlayerController* PlayerCtrl = GetPlayerController();

            Destroy();

            PlayerCtrl->ProcessRebornCharacter(CurrTransform);
        },
        PlayerResurrectionTime,
        false
    );
}

void ADCharacterPlayer::HealthChanged(const FOnAttributeChangeData& AttrData)
{
    if (GetHealth() == 0.f)
    {
        if (IsLocallyControlled())
        {
            StopFire();
            StopAim();
            GetWorldTimerManager().ClearTimer(Handle_Reload);
            GetWorldTimerManager().ClearTimer(Handle_Equip);

            DisableInput(GetPlayerController());

            TPCamera->bUsePawnControlRotation = false;
            TPCameraArm->bUsePawnControlRotation = false;
            TPCamera->FieldOfView = 120.f;
            TPCamera->SetRelativeRotation(FRotator(-60.f, 0, 0));
            TPCamera->SetRelativeLocation(FVector(0, 0, 240.f));
            TPCameraArm->SocketOffset = FVector::ZeroVector;

            PlayerHUD->RemoveFromParent();
        }

        if (ActiveWeapon)
        {
            ActiveWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
            ActiveWeapon->WeaponMesh->SetSimulatePhysics(true);
        }
    }

    UpdateHealthUI();

    Super::HealthChanged(AttrData);
}

void ADCharacterPlayer::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
    Super::HandleDamage(DamageDone, Handle);
    TriggerAbilityFromTag(ConditionTags::Condition_Injured, Handle.GetInstigator());
}

void ADCharacterPlayer::UpdateHealthUI()
{
    if (PlayerHUD)
    {
        PlayerHUD->SetHealth(GetHealth(), GetHealthPercent());
        PlayerHUD->SetShield(GetShield(), GetShieldPercent());
    }
}

void ADCharacterPlayer::RadarScanTick()
{
    TArray<FHitResult> OutHits;
    FVector Location = GetActorLocation();

    UKismetSystemLibrary::SphereTraceMultiForObjects(this, Location, Location, RadarScanRadius,
        ScanObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, OutHits, true);

    ActorWithinRadius.Reset();

    for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(OutHits); It; ++It)
    {
        AActor* Actor = (*It).GetActor();
        
        if (IMiniMapDataInterface* MiniMapDataInterface = Cast<IMiniMapDataInterface>(Actor))
        {
            UMiniMapDataComponent* DataComponent = MiniMapDataInterface->GetMiniMapDataComponent();
            if (DataComponent == nullptr)
            {
                continue;
            }

            if (!DataComponent->IsActive())
            {
                continue;
            }

            ActorWithinRadius.Add(Actor);
        }
    }
}

void ADCharacterPlayer::RefreshAttributes()
{
    //AttributeSet->SetAttackPower(ActiveWeapon->WeaponAttribute.AttackPower);
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
        PlayerHUD->AddBuffIcon(BuffUIData->BuffTag, BuffUIData->Icon, Spec.StackCount, Spec.GetDuration());
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectTimeChange(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration)
{
    //DREAM_LOG(Error, TEXT("NewDuration: %f"), NewDuration);
    if (const FActiveGameplayEffect* ActiveGE = AbilitySystem->GetActiveGameplayEffect(Handle))
    {
        if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(ActiveGE->Spec.Def->UIData))
        {
            PlayerHUD->RefreshBuffIcon(BuffUIData->BuffTag, ActiveGE->Spec.StackCount, NewDuration);
        }
    }
}

void ADCharacterPlayer::OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& Effect)
{
    if (UDGameplayEffectUIData_Buff* BuffUIData = Cast<UDGameplayEffectUIData_Buff>(Effect.Spec.Def->UIData))
    {
        PlayerHUD->RemoveBuffIcon(BuffUIData->BuffTag);
    }
}

void ADCharacterPlayer::HitEnemy(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget)
{
    if (DamageInfo.bKilled)
    {
        TriggerAbilityFromTag(ConditionTags::Condition_KilledEnemy, HitTarget);
    }
    else
    {
        TriggerAbilityFromTag(ConditionTags::Condition_HitEnemy, HitTarget);
    }

    ClientHitEnemy(DamageInfo);
}

void ADCharacterPlayer::ClientHitEnemy_Implementation(const FDamageTargetInfo& DamageInfo)
{
    const FGameplayEffectContext* EffectContext = DamageInfo.Handle.Get();
    
    if (EffectContext == nullptr)
    {
        return;
    }

    ShowHitEnemyTips(DamageInfo.bKilled);
        
    if (EffectContext->GetScriptStruct() == FDreamGameplayEffectContext::StaticStruct())
    {
        FDreamGameplayEffectContext* DreamEffectContext = ((FDreamGameplayEffectContext*)EffectContext);

        if (const FHitResult* HitResult = DreamEffectContext->GetHitResult())
        {
            SpawnDamageWidget(HitResult->ImpactPoint, DamageInfo.DamageAmount, DreamEffectContext->GetDamageCritical(), false);
            
            if (ADEnemyBase* Enemy = Cast<ADEnemyBase>(HitResult->GetActor()))
            {
                Enemy->ShowHealthUI();
            }
        }
    }
}

int32 ADCharacterPlayer::GetPickUpMagazineNumber(EAmmoType AmmoType) const
{
    int Number;

    switch (AmmoType)
    {
    case EAmmoType::Level1:
        Number = UKismetMathLibrary::RandomIntegerInRange(100, 300);
        break;
    case EAmmoType::Level2:
        Number = UKismetMathLibrary::RandomIntegerInRange(40, 80);
        break;
    case EAmmoType::Level3:
        Number = UKismetMathLibrary::RandomIntegerInRange(5, 20);
        break;
    default:
        Number = 1;
        break;
    }

    return Number;
}

void ADCharacterPlayer::SetMeshLightChannel(int32 LightChannel)
{
    GetMesh()->SetLightingChannels(LightChannel==0, LightChannel==1, LightChannel==2);
    const TArray<USceneComponent*>& Components = GetMesh()->GetAttachChildren();
    for (USceneComponent* Component : Components)
    {
        DREAM_NLOG(Error, TEXT("Component: %s"), *Component->GetName());
        if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
        {
            PrimitiveComponent->SetLightingChannels(LightChannel==0, LightChannel==1, LightChannel==2);
        }
    }
}

void ADCharacterPlayer::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BP_OnRepPlayerState();
}

void ADCharacterPlayer::ServerSetCombatStatus_Implementation(bool bNewCombatStatus)
{
    SetCombatStatus(bNewCombatStatus);
}

void ADCharacterPlayer::ModStatusToRelax()
{
    SetCombatStatus(false);
}

void ADCharacterPlayer::AttemptSetStatusToRelax()
{
    if (bCombatStatus)
    {
        if (CombatStatusCounter.Decrement() == 0)
        {
            GetWorldTimerManager().SetTimer(Handle_CombatStatus, this, &ADCharacterPlayer::ModStatusToRelax, OutOfCombatTime);
        }
    }
}

void ADCharacterPlayer::ClearCombatStatusCounter()
{
    StopFire();
    StopAim();
    HandleStopReload();
    CombatStatusCounter.Reset();
    GetWorldTimerManager().ClearTimer(Handle_CombatStatus);
    ModStatusToRelax();
}

void ADCharacterPlayer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    /*if (FMath::IsNearlyZero(GetVelocity().Size()))
    {
        FRotator Delta = (CtrlRotation - ActorRotation);
        Delta.Normalize();
                
        CtrlYawDeltaCount += Delta.Yaw - PrevCtrlYaw;
        PrevCtrlYaw = Delta.Yaw;
                
        if (!UKismetMathLibrary::InRange_FloatFloat(Delta.Yaw, MinRotateLimit, MaxRotateLimit))
        {
            float TargetYaw = CtrlRotation.Yaw + (Delta.Yaw > 0 ? MinRotateLimit : MaxRotateLimit);
            SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, TargetYaw, 0), DeltaSeconds, 20.f));
        }
    }
    else
    {
        SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, CtrlRotation.Yaw, 0), DeltaSeconds, 18.f));
    }*/

    FRotator ActorRotation = GetActorRotation();
    FRotator CtrlRotation = GetRemoteControllerRotation();

    SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, CtrlRotation.Yaw, 0), DeltaSeconds, 18.f));
}

FRotator ADCharacterPlayer::GetRemoteControllerRotation() const
{
    return IsLocallyControlled() ? GetControlRotation() : RemoteCtrlRotation;
}

bool ADCharacterPlayer::PickUpMagazine(EAmmoType AmmoType)
{
    if (IsLocallyControlled())
    {
        int32 RandomNumber = GetPickUpMagazineNumber(AmmoType);
        bool bPickUp = false;
        if (ADPlayerController* PC = GetPlayerController())
        {
            bPickUp = PC->AddWeaponAmmunition(AmmoType, RandomNumber);
            UpdateAmmoUI();
        }
        return bPickUp;
    }
    return true;
}

void ADCharacterPlayer::SetCombatStatus(bool bNewCombatStatus)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        ServerSetCombatStatus(bNewCombatStatus);
    }
    
    if (bNewCombatStatus)
    {
        if (IsLocallyControlled())
        {
            if (CombatStatusCounter.GetValue() < 0)
            {
                CombatStatusCounter.Reset();
            }
            
            GetWorldTimerManager().ClearTimer(Handle_CombatStatus);
            CombatStatusCounter.Increment();
        }
    }

    bCombatStatus = bNewCombatStatus;
}

void ADCharacterPlayer::DoAddMiniMapTips(TArray<FMiniMapData>& Data, const TArray<AActor*>& ScanActors)
{
    for (AActor* ScanActor : ScanActors)
    {
        if (!ScanActor->IsValidLowLevelFast())
        {
            continue;
        }
        if (ScanActor->IsPendingKillPending())
        {
            continue;
        }
        
        if (IMiniMapDataInterface* DataInterface = Cast<IMiniMapDataInterface>(ScanActor))
        {
            Data.Add(DataInterface->GetMiniMapDataComponent()->CalculationPosition(GetActorLocation(), GetControlRotation(), RadarScanRadius));
        }
        else if (UMiniMapDataComponent* MiniMapDataComponent = ScanActor->FindComponentByClass<UMiniMapDataComponent>())
        {
            Data.Add(MiniMapDataComponent->CalculationPosition(GetActorLocation(), GetControlRotation(), RadarScanRadius));
        }
    }
}

bool ADCharacterPlayer::GetMiniMapTips(TArray<FMiniMapData>& Data)
{
    DoAddMiniMapTips(Data, InfiniteActors);
    DoAddMiniMapTips(Data, ActorWithinRadius);
    return Data.Num() > 0;
}

const FCharacterMontage* ADCharacterPlayer::GetCurrentActionMontage() const
{
    if (ActiveWeapon)
    {
        return &ActiveWeapon->CharacterAnim;
    }

    return nullptr;
}

void ADCharacterPlayer::SetCameraFieldOfView(float NewFOV)
{
    TPCamera->SetFieldOfView(NewFOV);
}

void ADCharacterPlayer::CameraAimTransformLerp(float Alpha)
{
    TPCamera->SetRelativeRotation(FMath::LerpStable(DefaultCameraRotation, AimedCameraRotation, Alpha));
    TPCameraArm->SocketOffset = FMath::LerpStable(DefaultArmSocketOffset, AimedArmSocketOffset, Alpha);
}

void ADCharacterPlayer::TriggerAbilityFromTag(const FGameplayTag& Tag, AActor* Target)
{
    FGameplayEventData Payload;
    Payload.Instigator = this;
    Payload.Target = Target;
    AbilitySystem->HandleGameplayEvent(Tag, &Payload);
}

void ADCharacterPlayer::ToggleCrossHairVisible(bool bVisible)
{
    if (PlayerHUD)
    {
        if (bVisible)
        {
            PlayerHUD->SetCrossHairVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            PlayerHUD->SetCrossHairVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ADCharacterPlayer::ClientResurrection_Implementation(int32 ResurrectionTime)
{
    BP_OnStartResurrection(ResurrectionTime);
}
