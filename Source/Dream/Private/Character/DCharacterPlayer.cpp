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
#include "DreamGameMode.h"
#include "Engine.h"
#include "DGameplayStatics.h"
#include "DEnemyBase.h"
#include "DGameplayEffectUIData.h"
#include "DGameplayTags.h"
#include "DGameUserSettings.h"
#include "DModuleBase.h"
#include "DreamGameplayAbility.h"
#include "DreamAttributeSet.h"
#include "DreamGameplayType.h"
#include "Components/DamageWidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "PlayerDataInterfaceStatic.h"
#include "DPlayerController.h"
#include "DProjectSettings.h"
#include "DreamWidgetStatics.h"
#include "MinimapScanComponent.h"
#include "UI/SPlayerHUD.h"

#define MAX_MOUSE_SENSITIVITY 135

#define TRIGGER_ABILITY(Tag, Target_) \
    { \
        FGameplayEventData Payload; \
        Payload.Instigator = this; \
        Payload.Target = Target_; \
        AbilitySystem->HandleGameplayEvent(CUSTOMIZE_TAG(Tag), &Payload); \
    }

UDProjectSettings* ADCharacterPlayer::CDOProjectSettings = nullptr;

// Sets default values
ADCharacterPlayer::ADCharacterPlayer()
    : SprintSpeed(1000.f)
    , AimMoveSpeed(200.f)
    , NormalSpeed(600.f)
    , CombatStatusCount(0)
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

    ScanComponent = CreateDefaultSubobject<UMinimapScanComponent>(TEXT("ScanComponent"));

    WeaponInventory.SetNumZeroed(2);
    EquippedModules.SetNumZeroed(static_cast<uint8>(EModuleCategory::Max));
    
    CDOProjectSettings = UDProjectSettings::GetProjectSettings();
}

void ADCharacterPlayer::PossessedBy(AController* InController)
{
    // 网络模式下  本地客户端不执行此函数  只有服务器会执行此函数
    Super::PossessedBy(InController);
}

void ADCharacterPlayer::BeginPlay()
{
    NormalSpeed = GetCharacterMovement()->MaxWalkSpeed;

    if (IsLocallyControlled())
    {
        if (APlayerController* PC = GetPlayerController())
        {
            PC->SetInputMode(FInputModeGameOnly());
        }
        
        DefaultCameraRotation = TPCamera->GetRelativeRotation();
        DefaultArmSocketOffset = TPCameraArm->SocketOffset;

        PlayerHUD = SNew(SPlayerHUD)
                .InOwnerPlayer(this)
                .MinimapDataIterator_UObject(this, &ADCharacterPlayer::GetMinimapDataIterator);
            
        GEngine->GameViewport->AddViewportWidgetContent(PlayerHUD.ToSharedRef(), EWidgetOrder::Player);
            
        AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectAdded);
        AbilitySystem->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectRemoved);

        FPlayerDataInterface* PDS = FPDIStatic::Get();
        Handle_PlayerInfo = PDS->AddOnGetPlayerInfo(FOnGetPlayerInfo::FDelegate::CreateUObject(this, &ADCharacterPlayer::OnInitPlayer));
        Handle_Properties = PDS->GetPlayerDataDelegate().OnPropertiesChange.AddUObject(this, &ADCharacterPlayer::OnPlayerPropertiesChanged);
        
        PDS->GetPlayerInfo(EQueryCondition::Weapon | EQueryCondition::Weapon_Equipped |
            EQueryCondition::Module | EQueryCondition::Module_Equipped | EQueryCondition::Skin);
    }

    Super::BeginPlay();

    // 服务器和本地角色才需要tick
    SetActorTickEnabled(GetLocalRole() == ROLE_Authority || IsLocallyControlled());

    AbilitySystem->InitAbilityActorInfo(this, this);
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

void ADCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (IsLocallyControlled())
    {
        FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.Remove(Handle_Properties);
    }
}

void ADCharacterPlayer::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    // 这里无法判断isLocalController 可能是此时本地的Role还没有设置好
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
    if (!ActiveWeapon)
    {
        return;
    }

    if (ActiveWeapon->FireMode != EFireMode::Accumulation)
    {
        if (GetWorld()->GetTimeSeconds() <= (ActiveWeapon->GetLastFireTimeSeconds() + ActiveWeapon->GetFireInterval()))
        {
            return;
        }
    }

    bFiring = true;

    if (WeaponStatus > EWeaponStatus::Firing)
    {
        return;
    }

    ActiveWeapon->BP_OnStartFire();

    bool PrevCombatStatus = bCombatStatus;

    StopSprint();
    StateSwitchToCombat();
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

    if (ActiveWeapon->AmmoNum == 0)
    {
        ReloadMagazine();
    }
}

void ADCharacterPlayer::StopFire()
{
    if (!ActiveWeapon)
    {
        return;
    }
    
    if (bFiring)
    {
        StateSwitchToRelax();

        if (WeaponStatus == EWeaponStatus::Firing)
        {
            WeaponStatus = EWeaponStatus::Idle;

            ActiveWeapon->BP_OnStopFire();
        }

        if (Handle_Shoot.IsValid())
        {
            GetWorldTimerManager().ClearTimer(Handle_Shoot);
        }
    }

    bFiring = false;
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

    if (GetLocalRole() != ROLE_Authority)
    {
        ServerSwitchWeapon();
    }

    WeaponStatus = EWeaponStatus::Equipping;

    if (bCombatStatus)
    {
        StopFire();
        StopAim();
        StopReloadMagazine();
    }

    ActiveWeapon->SetWeaponEnable(false);
    float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
    GetWorldTimerManager().SetTimer(Handle_Equip, FTimerDelegate::CreateUObject(this, &ADCharacterPlayer::SwitchFinished, WeaponIndex), Duration, false);
}

void ADCharacterPlayer::MulticastSwitchWeapon_Implementation()
{
    if (GetLocalRole() != ROLE_SimulatedProxy)
    {
        return;
    }

    PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
}

void ADCharacterPlayer::ServerSwitchWeapon_Implementation()
{
    MulticastSwitchWeapon();
    float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
    GetWorldTimerManager().SetTimer(Handle_Equip, FTimerDelegate::CreateUObject(this, &ADCharacterPlayer::SwitchFinished, -1), Duration, false);
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
        
        WeaponStatus = EWeaponStatus::Reloading;

        StateSwitchToCombat();

        ServerReloadMagazine();

        float Duration = PlayMontage(GetCurrentActionMontage()->ReloadAnim, ActiveWeapon->ReloadAnim);
        GetWorldTimerManager().SetTimer(Handle_Reload, this, &ADCharacterPlayer::ReloadFinished, Duration);
    }
}

void ADCharacterPlayer::StopReloadMagazine()
{
    if (WeaponStatus == EWeaponStatus::Reloading)
    {
        WeaponStatus = EWeaponStatus::Idle;
        GetWorldTimerManager().ClearTimer(Handle_Reload);

        StateSwitchToRelax();
        StopAnimMontage(GetCurrentActionMontage()->ReloadAnim);
    }
}

void ADCharacterPlayer::ReloadFinished()
{
    WeaponStatus = EWeaponStatus::Idle;
    
    ActiveWeapon->ReloadingWeapon();
    
    UpdateAmmoUI();

    StateSwitchToRelax();

    if (bFiring)
    {
        StartFire();   
    }
}

void ADCharacterPlayer::ServerReloadMagazine_Implementation()
{
    TRIGGER_ABILITY(Condition_Reloading, this);
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
        
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CDOProjectSettings->GetWepActiveSockName());
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
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CDOProjectSettings->GetWepHolsterSockName());
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
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    AShootWeapon* NewWeapon = WeaponInventory[NewWeaponIndex];

    FEquipmentAttributes PrevWeaponAttr = ActiveWeapon->WeaponAttribute;

    if (ActiveWeapon)
    {
        ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CDOProjectSettings->GetWepHolsterSockName());
        ActiveWeapon->SetActorRelativeTransform(ActiveWeapon->WeaponHolsterSocketOffset);
    }

    NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CDOProjectSettings->GetWepActiveSockName());
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
    UClass* DamageComponentClass = CDOProjectSettings->GetDamageComponentClass();

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

float ADCharacterPlayer::GetCtrlYawDeltaCount() const
{
    return CtrlYawDeltaCount;
}

void ADCharacterPlayer::RefreshAttributeBaseValue()
{
    FEquipmentAttributes FinalValue;

    FinalValue.CombineSkipPerks(ActiveWeapon->WeaponAttribute);

    for (UDModuleBase* Module : EquippedModules)
    {
        FinalValue += Module->ModuleAttributes;
    }
    
    float BaseAttrDeltaScale = GetCharacterLevel() * BaseAttributesSettings->IncrementPerLevel;
    FinalValue += BaseAttributesSettings->BaseAttributes * BaseAttrDeltaScale;

    AttributeSet->UpdateAttributesBase(FinalValue);

    AbilitySystem->ClearAllAbilities();
    
    for (UClass* AbilityClass : LearnedTalents)
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass));
    }

    for (int32 AbilityGuid : FinalValue.Perks)
    {
        UClass* PerkClass = CDOProjectSettings->GetItemClassFromGuid(AbilityGuid);
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(PerkClass));
    }
    
    for (int32 AbilityGuid : ActiveWeapon->WeaponAttribute.Perks)
    {
        UClass* PerkClass = CDOProjectSettings->GetItemClassFromGuid(AbilityGuid);
        CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(PerkClass)));
    }

    AbilitySystem->RemoveActiveEffectsWithTags(FGameplayTagContainer(CUSTOMIZE_TAG(GE_Buff_All)));
    
    TRIGGER_ABILITY(Condition_Immediately, this);
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
        UClass* AbilityClass = CDOProjectSettings->GetItemClassFromGuid(ActiveWeapon->WeaponAttribute.Perks[N]);
        CacheWeaponPerkHandles.Add(AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbilityClass)));
    }

    AttributeSet->IncrementAttributes(ActiveWeapon->WeaponAttribute - PrevWeaponAttrs);

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
        GEngine->GameViewport->RemoveViewportWidgetContent(PlayerHUD.ToSharedRef());
        PlayerHUD.Reset();
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

        if (PlayerHUD.IsValid())
        {
            PlayerHUD->SetCrosshairBrush(ActiveWeapon->GetDynamicCrosshairBrush());
        }
        
        UpdateAmmoUI();
    }
}

void ADCharacterPlayer::OnInitPlayer(const FPlayerInfo& PlayerInfo, bool bSuccess)
{
    FPDIStatic::Get()->RemoveOnGetPlayerInfo(Handle_PlayerInfo);
    
    if (!bSuccess)
    {
        DREAM_NLOG(Error, TEXT("InitPlayer Failure"));
        UDreamWidgetStatics::PopupDialog(this, EDialogType::ERROR, FText::FromString(TEXT("初始化玩家数据失败")));
        UDGameplayStatics::ReturnToHomeWorld(this);
        return;
    }

    ServerInitializePlayer(PlayerInfo);
}

void ADCharacterPlayer::ServerInitializePlayer_Implementation(const FPlayerInfo& PlayerInfo)
{
    UCharacterMesh* CharacterMesh = Cast<UCharacterMesh>(PlayerInfo.CharacterMesh.TryLoad());
    checkf(CharacterMesh, TEXT("CharacterMesh Invalid, %s"), *PlayerInfo.CharacterMesh.ToString());

    CurrentCharacterMesh = CharacterMesh;
    
    if (!IsRunningDedicatedServer())
    {
        UpdateCharacterMesh();
    }

    for (const FPlayerWeapon& PW : PlayerInfo.Weapons)
    {
        UClass* Class = CDOProjectSettings->GetItemClassFromGuid(PW.ItemGuid);
        DoServerEquipWeapon(PW.Index, PW.Attributes, Class);
    }

    for (const FPlayerModule& PM : PlayerInfo.Modules)
    {
        UClass* Class = CDOProjectSettings->GetItemClassFromGuid(PM.ItemGuid);
        DoServerEquipModule(Class, PM.Attributes);
    }

    // todo 天赋分支功能待完善
    TArray<FTalentInfo> Talents = CDOProjectSettings->GetLearnedTalents(ETalentCategory::Warrior, PlayerInfo.LearnedTalents);
    
    LearnedTalents.Reset();
    for (const FTalentInfo& Talent : Talents)
    {
        LearnedTalents.Add(Talent.TalentClass);
    }

    Level = PlayerInfo.CharacterLevel;
    
    RefreshAttributeBaseValue();
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

bool ADCharacterPlayer::CanShoot() const
{
    return ActiveWeapon && WeaponStatus < EWeaponStatus::Reloading && ActiveWeapon->AmmoNum > 0;
}

bool ADCharacterPlayer::CanReload() const
{
    return ActiveWeapon && WeaponStatus < EWeaponStatus::Reloading &&
        GetPlayerController()->GetWeaponAmmunition(ActiveWeapon->AmmoType) > 0 && !ActiveWeapon->IsFullAmmo();
}

bool ADCharacterPlayer::CanEquip() const
{
    return ActiveWeapon && WeaponStatus < EWeaponStatus::Equipping;
}

bool ADCharacterPlayer::CanAim() const
{
    return ActiveWeapon && WeaponStatus <= EWeaponStatus::Reloading;
}

void ADCharacterPlayer::UpdateAmmoUI() const
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
    if ((Controller != nullptr) && (Value != 0.0f))
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
    if ((Controller != nullptr) && (Value != 0.0f) && !bSprinted)
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

    MovingInput.Assign(PrevMovingInput);
}

AShootWeapon* ADCharacterPlayer::GetActiveWeapon() const
{
    return ActiveWeapon;
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
        ToggleCrosshairVisible(true);

        StateSwitchToCombat();
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
        ToggleCrosshairVisible(false);

        ActiveWeapon->SetWeaponAim(false);
        BP_OnToggleWeaponAim(false);
        AimedMoveSpeedChange(false);
        
        StateSwitchToRelax();
    }
}


void ADCharacterPlayer::ServerStopAim_Implementation()
{
    AimedMoveSpeedChange(false);
}

void ADCharacterPlayer::AimedMoveSpeedChange(bool bNewAim) const
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
        // 强制解除战斗状态
        CombatStatusCount = 0;
        StateSwitchToRelaxImmediately();
        
        WeaponStatus = EWeaponStatus::Idle;
        
        StopFire();
        StopAim();
        StopReloadMagazine();

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

void ADCharacterPlayer::SprintMoveSpeedChange(bool bNewSprint) const
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
    DOREPLIFETIME(ADCharacterPlayer, bSprinted);
    DOREPLIFETIME(ADCharacterPlayer, CurrentCharacterMesh);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, bCombatStatus, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, ActiveWeaponIndex, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ADCharacterPlayer, MovingInput, COND_SkipOwner);
}

void ADCharacterPlayer::Destroyed()
{
    Super::Destroyed();

    for (AShootWeapon* Weapon : WeaponInventory)
    {
        if (Weapon && !Weapon->IsPendingKill())
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
    //TearOff(); 断开网络同步

    ADreamGameMode* DGameMode = GetWorld()->GetAuthGameMode<ADreamGameMode>();
    DGameMode->ReSpawnCharacter(this);
}

void ADCharacterPlayer::HealthChanged(const FOnAttributeChangeData& AttrData)
{
    if (PlayerHUD.IsValid())
    {
        PlayerHUD->SetHealthPercent(GetHealthPercent());
    }
    
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

            if (GEngine->GameViewport)
            {
                GEngine->GameViewport->RemoveViewportWidgetContent(PlayerHUD.ToSharedRef());
            }
        }

        if (ActiveWeapon)
        {
            ActiveWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
            ActiveWeapon->WeaponMesh->SetSimulatePhysics(true);
        }
    }

    Super::HealthChanged(AttrData);
}

void ADCharacterPlayer::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
    Super::HandleDamage(DamageDone, Handle);
    TRIGGER_ABILITY(Condition_Injured, Handle.GetInstigator());
    
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

void ADCharacterPlayer::ServerUpdateMovingInput_Implementation(const FMoveInput& NewMovingInput)
{
    MovingInput = NewMovingInput;
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
    GetMesh()->SetSkeletalMesh(CurrentCharacterMesh->MasterMesh);

    for (auto Iterator = GetMesh()->GetAttachChildren().CreateConstIterator(); Iterator; ++Iterator)
    {
        USceneComponent* const Child = *Iterator;

        if (Child && Child->GetOwner() == this)
        {
            Child->DestroyComponent();
        }
    }
        
    for (USkeletalMesh* SKMesh : CurrentCharacterMesh->SlaveMeshs)
    {
        USkeletalMeshComponent* SKComponent = NewObject<USkeletalMeshComponent>(this);
        SKComponent->RegisterComponent();
        SKComponent->SetSkeletalMesh(SKMesh);
        SKComponent->SetIsReplicated(false);
        SKComponent->SetAnimClass(CDOProjectSettings->GetSlaveAnimClass());
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
        PlayerHUD->ActivateInteractiveButton(InteractiveTime, Desc, FOnInteractiveCompleted::CreateLambda([Event, this]
        {
            Event.ExecuteIfBound(this);
        }));
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
        if (ADPlayerController* PC = GetPlayerController())
        {
            bPickUp = PC->AddWeaponAmmunition(AmmoType, CDOProjectSettings->GetPickupAmmunitionAmount());
            UpdateAmmoUI();
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
                &ADCharacterPlayer::SetStateToRelax, CDOProjectSettings->GetIdleSwitchingTime());
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

const FCharacterMontage* ADCharacterPlayer::GetCurrentActionMontage() const
{
    if (ActiveWeapon)
    {
        return &ActiveWeapon->CharacterAnim;
    }

    return nullptr;
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
