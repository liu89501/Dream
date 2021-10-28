// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable All
#include "Character/DCharacterPlayer.h"
#include "AbilitySystemGlobals.h"
#include "DBaseAttributesAsset.h"
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
#include "DGameUserSettings.h"
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
#include "PlayerDataInterfaceStatic.h"
#include "GameplayAbilitySpec.h"
#include "DPlayerCameraManager.h"
#include "GameplayEffectExtension.h"

#define MAX_MOUSE_SENSITIVITY 135

// Sets default values
ADCharacterPlayer::ADCharacterPlayer()
    : SprintSpeed(1000.f)
    , AimMoveSpeed(200.f)
    , NormalSpeed(600.f)
    , OutOfCombatTime(3.f)
    , ScanInterval(1.f)
    , ActiveWeaponIndex(0)
{

    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;

    MinRotateLimit = -60.f;
    MaxRotateLimit = 60.f;
    
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

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

    GetCharacterMovement()->bOrientRotationToMovement = false;

    // 初始化武器插槽
    WeaponInventory.AddZeroed(2);
    //LocalEquippedWeaponClass.AddZeroed(2);

    ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPAsset(TEXT("/Game/Animation/AnimBP_CharacterCopy"));
    SlaveMeshAnimBPClass = AnimBPAsset.Class;
}

void ADCharacterPlayer::PossessedBy(AController* InController)
{
    // 网络模式下  本地客户端不执行此函数  只有服务器会执行此函数
    Super::PossessedBy(InController);
}

void ADCharacterPlayer::InitializeUI()
{
    if ((PlayerHUD == nullptr) && PlayerHUDClass)
    {
        PlayerHUD = CreateWidget<UPlayerHUD>(GetWorld(), PlayerHUDClass);
        PlayerHUD->SetHealth(1.f);
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

        if (!bIsPreviewCharacter)
        {
            if (APlayerController* PC = GetPlayerController())
            {
                PC->SetInputMode(FInputModeGameOnly());
            }
            
            InitializeUI();
            GetWorldTimerManager().SetTimer(Handle_RadarScan, this, &ADCharacterPlayer::RadarScanTick, ScanInterval, true, 0.f);

            AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectAdded);
            AbilitySystem->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectRemoved);
        }

        FPlayerDataInterface* PDS = FPlayerDataInterfaceStatic::Get();
        FGetPlayerInfoComplete Delegate;
        Delegate.BindUObject(this, &ADCharacterPlayer::OnInitPlayer);
        PDS->GetPlayerInfo(EGetEquipmentCondition::Equipped, Delegate);
        Handle_Exp = PDS->GetPlayerDataDelegate().OnExperienceChanged.AddUObject(this, &ADCharacterPlayer::OnPlayerExperienceChanged);
    }

    if (!bIsPreviewCharacter)
    {
        SetActorTickEnabled(GetLocalRole() == ROLE_Authority || IsLocallyControlled());
    }

    Super::BeginPlay();

    AbilitySystem->InitAbilityActorInfo(this, this);
}

void ADCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (IsLocallyControlled())
    {
        FPlayerDataInterfaceStatic::Get()->GetPlayerDataDelegate().OnExperienceChanged.Remove(Handle_Exp);
    }
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

    if (bIsPreviewCharacter)
    {
        return;
    }

    /*AbilitySystem->BindAbilityActivationToInputComponent(PlayerInputComponent,
        FGameplayAbilityInputBinds(TEXT("Confirm"), TEXT("Cancel"), TEXT("AbilityBind")));*/

    if (UDGameUserSettings* Settings = Cast<UDGameUserSettings>(GEngine->GetGameUserSettings()))
    {
        SetMouseInputScale(Settings->GetMouseSensitivity());
    }
    
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

    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ADCharacterPlayer::SwitchWeapon);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADCharacterPlayer::ReloadMagazine);

    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ADCharacterPlayer::StartAim);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ADCharacterPlayer::StopAim);
    //PlayerInputComponent->BindAction("SwitchCombat", IE_Released, this, &ADCharacterPlayer::SwitchCombat);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADCharacterPlayer::ToggleSprint);
    //PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ADCharacterPlayer::StopSprint);

    PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &ADCharacterPlayer::SwitchWeaponToFirst);
    PlayerInputComponent->BindAction("Weapon2", IE_Pressed, this, &ADCharacterPlayer::SwitchWeaponToSecond);
}

void ADCharacterPlayer::StartFire()
{
    bFireButtonDown = true;

    if (!ActiveWeapon)
    {
        return;
    }

    if (ActiveWeapon->AmmoNum == 0)
    {
        ReloadMagazine();
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
    if (WeaponStatus > EWeaponStatus::Firing || ActiveWeapon->AmmoNum == 0)
    {
        return;
    }

    //TriggerAbilityFromTag(ConditionTags::Condition_Firing, this);

    ActiveWeapon->HandleFire();
    UpdateAmmoUI();

    if (ActiveWeapon->FireCameraShake)
    {
        GetPlayerController()->ClientStartCameraShake(ActiveWeapon->FireCameraShake);
    }

    if (ActiveWeapon->FireMode == EFireMode::SemiAutomatic)
    {
        ActiveWeapon->BP_OnStopFire();
    }
    else if (ActiveWeapon->FireMode == EFireMode::Accumulation)
    {
        ActiveWeapon->BP_OnStopFire();
        GetWorldTimerManager().ClearTimer(Handle_Shoot);
    }
}

void ADCharacterPlayer::StopFire()
{
    bFireButtonDown = false;
    
    if (!ActiveWeapon)
    {
        return;
    }

    if (WeaponStatus == EWeaponStatus::Firing)
    {
        AttemptSetStatusToRelax(); 
        WeaponStatus = EWeaponStatus::Idle;
    }

    if (Handle_Shoot.IsValid())
    {
        ActiveWeapon->BP_OnStopFire();
        GetWorldTimerManager().ClearTimer(Handle_Shoot);
    }
}

void ADCharacterPlayer::SwitchWeapon()
{
    HandleSwitchWeapon(-1);
}

void ADCharacterPlayer::HandleSwitchWeapon(int32 WeaponIndex)
{
    if (!CanEquip())
    {
        return;
    }

    ServerSwitchWeapon();

    WeaponStatus = EWeaponStatus::Equipping;
    
    if (bCombatStatus)
    {
        StopFire();
        StopAim();
        HandleStopReload();

        ActiveWeapon->SetWeaponEnable(false);

        float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
        GetWorldTimerManager().SetTimer(Handle_Equip, FTimerDelegate::CreateUObject(this, &ADCharacterPlayer::SwitchFinished, -1), Duration, false);
    }
    else
    {
        SwitchFinished(WeaponIndex);
    }
}

void ADCharacterPlayer::MulticastSwitchWeapon_Implementation()
{
    if (GetLocalRole() != ROLE_SimulatedProxy)
    {
        return;
    }

    float MontageDuration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
}

void ADCharacterPlayer::ServerSwitchWeapon_Implementation()
{
    if (bCombatStatus)
    {
        MulticastSwitchWeapon();
        float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
        GetWorldTimerManager().SetTimer(Handle_Equip, FTimerDelegate::CreateUObject(this, &ADCharacterPlayer::SwitchFinished, -1), Duration, false);
    }
    else
    {
        SwitchFinished(INDEX_NONE);
    }
}

void ADCharacterPlayer::SwitchFinished(int32 WeaponIndex)
{
    //ActiveWeapon->SetActorHiddenInGame(true);
    if (IsLocallyControlled())
    {
        //ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("root"));
        WeaponStatus = EWeaponStatus::Idle;
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
    TriggerAbilityFromTag(CustomizeTags().Condition_Reloading, this);
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

void ADCharacterPlayer::OnRep_ActiveWeapon()
{
    SetActiveWeapon(ActiveWeapon);
}

void ADCharacterPlayer::EquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> WeaponClass)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        checkf(Index >= 0 && Index < WeaponInventory.Num(), TEXT("SetInventoryWeapon Fatal Error"));
        
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
        
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
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
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponHolsterSocketName);
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
    EquippedModules.Add(Module->Category, Module);
}

void ADCharacterPlayer::ServerEquipModule_Implementation(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs)
{
    EquipModule(ModuleClass, Attrs);
}

void ADCharacterPlayer::LearningTalents(const TArray<TSubclassOf<UDreamGameplayAbility>>& TalentClasses)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        LearnedTalents = TalentClasses;
        RefreshAttributeBaseValue();
    }
    else
    {
        ServerLearningTalent(TalentClasses);
    }
}

void ADCharacterPlayer::SwitchWeapon(int32 NewWeaponIndex)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        AShootWeapon* NewWeapon = WeaponInventory[NewWeaponIndex];

        FEquipmentAttributes PrevWeaponAttr = MoveTemp(ActiveWeapon->WeaponAttribute);

        if (ActiveWeapon)
        {
            ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponHolsterSocketName);
            ActiveWeapon->SetActorRelativeTransform(ActiveWeapon->WeaponHolsterSocketOffset);
        }

        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
        NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
        
        SetActiveWeapon(NewWeapon);

        FastRefreshWeaponAttribute(PrevWeaponAttr);
    }
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

void ADCharacterPlayer::AdditiveAttributes(const FEquipmentAttributes& Attributes)
{
    AbilitySystem->ApplyModToAttribute(AttributeSet->GetAttackPowerAttribute(), EGameplayModOp::Additive, Attributes.AttackPower);
    AbilitySystem->ApplyModToAttribute(AttributeSet->GetCriticalDamageAttribute(), EGameplayModOp::Additive, Attributes.CriticalDamage);
    AbilitySystem->ApplyModToAttribute(AttributeSet->GetCriticalRateAttribute(), EGameplayModOp::Additive, Attributes.CriticalRate);
    AbilitySystem->ApplyModToAttribute(AttributeSet->GetDefensePowerAttribute(), EGameplayModOp::Additive, Attributes.Defense);
    AbilitySystem->ApplyModToAttribute(AttributeSet->GetMaxHealthAttribute(), EGameplayModOp::Additive, Attributes.MaxHealth);
    AbilitySystem->ApplyModToAttribute(AttributeSet->GetHealthStealPercentageAttribute(), EGameplayModOp::Additive, Attributes.HealthSteal);
}

void ADCharacterPlayer::RefreshAttributeBaseValue()
{
    FEquipmentAttributes FinalValue;

    FinalValue += MoveTemp(ActiveWeapon->WeaponAttribute);

    for (TPair<EModuleCategory, UDModuleBase*> Module : EquippedModules)
    {
        FinalValue += Module.Value->ModuleAttributes;
    }

    FBaseAttributes BaseAttributes = BaseAttributesSettings->BaseAttributes;
    float BaseAttrDeltaScale = GetCharacterLevel() * BaseAttributesSettings->IncrementPerLevel;
    FinalValue += (BaseAttributes * BaseAttrDeltaScale);

    AttributeSet->SetAttackPower(FinalValue.AttackPower);
    AttributeSet->SetCriticalDamage(FinalValue.CriticalDamage);
    AttributeSet->SetCriticalRate(FinalValue.CriticalRate);
    AttributeSet->SetDefensePower(FinalValue.Defense);
    AttributeSet->SetMaxHealth(FinalValue.MaxHealth);
    AttributeSet->SetHealthStealPercentage(FinalValue.HealthSteal);

    FinalValue.Perks += LearnedTalents;

    AbilitySystem->ClearAllAbilities();
    
    for (UClass* AbilityClass : FinalValue.Perks)
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass));
    }
    
    for (UClass* AbilityClass : ActiveWeapon->WeaponAttribute.Perks)
    {
        CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass)));
    }

    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CustomizeTags().GE_Buff_All));
    TriggerAbilityFromTag(CustomizeTags().Condition_Immediately, this);
}

void ADCharacterPlayer::FastRefreshWeaponAttribute(const FEquipmentAttributes& PrevWeaponAttrs)
{
    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CustomizeTags().GE_Buff_Weapon));

    for (const FGameplayAbilitySpecHandle& Handle : CacheWeaponPerkHandles)
    {
        AbilitySystem->ClearAbility(Handle);
    }
    
    CacheWeaponPerkHandles.Reset(ActiveWeapon->WeaponAttribute.Perks.Num());
    
    for (int32 N = 0; N < ActiveWeapon->WeaponAttribute.Perks.Num(); N++)
    {
        UClass* AbilityClass = ActiveWeapon->WeaponAttribute.Perks[N];
        CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass)));
    }
    
    AdditiveAttributes(ActiveWeapon->WeaponAttribute - PrevWeaponAttrs);
    TriggerAbilityFromTag(CustomizeTags().Condition_Immediately, this);
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
    ActiveWeapon = NewWeapon;

    if (IsLocallyControlled())
    {
        ActiveWeapon->SetWeaponEnable(true);

        if (PlayerHUD)
        {
            PlayerHUD->SetCrossHairBrush(ActiveWeapon->GetDynamicCrosshairBrush());
            PlayerHUD->SetMagazineBrush(ActiveWeapon->GetDynamicMagazineBrush());
        }
        
        UpdateAmmoUI();
    }
}

void ADCharacterPlayer::OnInitPlayer(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage)
{
    if (!ErrorMessage.IsEmpty())
    {
        DREAM_LOG(Error, TEXT("InitPlayer Failure"));
        GetPlayerController()->ClientReturnToMainMenuWithTextReason(FText::FromString(ErrorMessage));
        return;
    }

    ServerInitializePlayer(PlayerInfo);
}

void ADCharacterPlayer::ServerInitializePlayer_Implementation(const FPlayerInfo& PlayerInfo)
{
    for (const FPlayerWeapon& PW : PlayerInfo.Weapons)
    {
        DoServerEquipWeapon(PW.Index, PW.Attributes, PW.WeaponClass);
    }

    for (const FPlayerModule& PM : PlayerInfo.Modules)
    {
        DoServerEquipModule(PM.ModuleClass, PM.Attributes);
    }

    LearnedTalents.Reset();
    for (const FTalentInfo& Talent : PlayerInfo.Talents)
    {
        LearnedTalents.Add(Talent.TalentClass.TryLoadClass<UDreamGameplayAbility>());
    }

    if (UCharacterMesh* CharacterMesh = Cast<UCharacterMesh>(PlayerInfo.CharacterMesh.TryLoad()))
    {
        if (CharacterMesh->IsSupportedForNetworking())
        {
            MulticastUpdateCharacterMesh(CharacterMesh);
        }
    }

    Level = PlayerInfo.Properties.Level;
    
    RefreshAttributeBaseValue();
}

void ADCharacterPlayer::OnPlayerExperienceChanged(int32 MaxExp, int32 CrtExp, int32 NewLevel)
{
    if (GetCharacterLevel() < NewLevel)
    {
        ServerSetCharacterLevel(NewLevel);
    }
}

void ADCharacterPlayer::ServerSetCharacterLevel_Implementation(int32 NewLevel)
{
    Level = NewLevel;

    RefreshAttributeBaseValue();
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

        //AddControllerYawInput(Value * GetWorld()->GetDeltaSeconds());
        AddMovementInput(Direction, Value);
    }

    MovingInput.X = Value;
}

void ADCharacterPlayer::ReplicationServerMoveDirection()
{
    if (!MovingInput.Equals(PrevMovingInput))
    {
        ServerUpdateMovingInput(MovingInput);
    }

    PrevMovingInput = MovingInput;
}

AShootWeapon* ADCharacterPlayer::GetActiveWeapon() const
{
    return ActiveWeapon;
}

void ADCharacterPlayer::ForceModifyStateToIdle()
{
    StopAim();
    StopFire();
    StopSprint();
}

int32 ADCharacterPlayer::GetActiveWeaponIndex() const
{
    return ActiveWeaponIndex;
}

void ADCharacterPlayer::ServerLearningTalent_Implementation(const TArray<TSubclassOf<UDreamGameplayAbility>>& TalentClasses)
{
    LearningTalents(TalentClasses);
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
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, ActiveWeaponIndex, ELifetimeCondition::COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, MovingInput, ELifetimeCondition::COND_SkipOwner);
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
    if (IsDeath())
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

    if (PlayerHUD)
    {
        PlayerHUD->SetHealth(GetHealthPercent());
    }

    Super::HealthChanged(AttrData);
}

void ADCharacterPlayer::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
    Super::HandleDamage(DamageDone, Handle);
    TriggerAbilityFromTag(CustomizeTags().Condition_Injured, Handle.GetInstigator());
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
            if (DataComponent && DataComponent->IsActive())
            {
                ActorWithinRadius.Add(Actor);
            }
        }
    }
}

void ADCharacterPlayer::ServerUpdateMovingInput_Implementation(const FVector2D& NewMovingInput)
{
    MovingInput = NewMovingInput;
}

void ADCharacterPlayer::ServerUpdateCharacterMesh_Implementation(UCharacterMesh* CharacterMesh)
{
    MulticastUpdateCharacterMesh(CharacterMesh);
}

void ADCharacterPlayer::MulticastUpdateCharacterMesh_Implementation(UCharacterMesh* CharacterMesh)
{
    if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_Standalone)
    {
        ProcessUpdateCharacterMesh(CharacterMesh);
    }
}

void ADCharacterPlayer::ProcessUpdateCharacterMesh(UCharacterMesh* CharacterMesh)
{
    GetMesh()->SetSkeletalMesh(CharacterMesh->MasterMesh);

    for (USkeletalMesh* SKMesh : CharacterMesh->SlaveMeshs)
    {
        USkeletalMeshComponent* SKComponent = NewObject<USkeletalMeshComponent>(this);
        SKComponent->RegisterComponent();
        SKComponent->SetSkeletalMesh(SKMesh);
        SKComponent->SetAnimClass(SlaveMeshAnimBPClass);
        SKComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        SKComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
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
        TriggerAbilityFromTag(CustomizeTags().Condition_KilledEnemy, HitTarget);
    }
    else
    {
        TriggerAbilityFromTag(CustomizeTags().Condition_HitEnemy, HitTarget);
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

            if (DamageInfo.bKilled)
            {
                FQuestActionHandle Handle(new FQuestAction_KilledTarget(HitResult->GetActor()->GetClass()));
                FPlayerDataInterfaceStatic::Get()->UpdateTaskState(Handle);
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

    if (!GetVelocity().IsNearlyZero() || bCombatStatus)
    {
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0, GetControlRotation().Yaw, 0), DeltaSeconds, 18.f));
    }

    if (IsLocallyControlled())
    {
        ReplicationServerMoveDirection();
    }
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

UPlayerHUD* ADCharacterPlayer::GetPlayerHUD() const
{
    return PlayerHUD;
}

void ADCharacterPlayer::SetMouseInputScale(int32 Value)
{
    BaseTurnRate = FMath::Min(BaseTurnRate + Value * 3, MAX_MOUSE_SENSITIVITY);
    BaseLookUpRate = FMath::Min(BaseLookUpRate + Value * 3, MAX_MOUSE_SENSITIVITY);
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
            FMiniMapData MapData;
            DataInterface->GetMiniMapDataComponent()->CalculationPosition(GetActorLocation(), GetControlRotation(), RadarScanRadius, MapData);
            Data.Add(MapData);
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
