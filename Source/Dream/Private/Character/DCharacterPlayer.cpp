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
#include "DreamAttributeSet.h"
#include "DreamGameplayType.h"
#include "MiniMapDataComponent.h"
#include "MiniMapDataInterface.h"
#include "PerkEffectSystemComponent.h"
#include "Components/DamageWidgetComponent.h"
#include "Props/Gear/GearBase.h"
#include "Props/Weapon/ShootWeapon.h"
#include "UI/PlayerHUD.h"
#include "GameplayEffectTypes.h"
#include "PlayerDataStoreModule.h"

static const FMontageSet InvalidMontage = FMontageSet();

// Sets default values
ADCharacterPlayer::ADCharacterPlayer()
    : BaseTurnRate(45.f)
    , BaseLookUpRate(45.f)
    , SprintSpeed(1000.f)
    , NormalSpeed(600.f)
    , OutOfCombatTime(3.f)
    , ScanInterval(1.f)
    , ActiveWeaponIndex(-1)
{
    PrimaryActorTick.bCanEverTick = false;

    PerkEffectSystem = CreateDefaultSubobject<UPerkEffectSystemComponent>(TEXT("PerkEffectSystem"));

    WalkAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WalkAudio"));
    WalkAudio->SetupAttachment(RootComponent);

    CharacterCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CharacterCapture"));
    CharacterCapture->SetAutoActivate(false);
    CharacterCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    CharacterCapture->SetupAttachment(RootComponent);

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

    GetCharacterMovement()->bOrientRotationToMovement = true;

    // 初始化武器插槽
    WeaponInventory.AddZeroed(3);
    LocalEquippedWeaponClass.AddZeroed(3);
}

void ADCharacterPlayer::PossessedBy(AController* InController)
{
    // 网络模式下  本地客户端不执行此函数  只有服务器会执行此函数
    Super::PossessedBy(InController);

    AbilitySystem->InitAbilityActorInfo(this, this);
}

void ADCharacterPlayer::InitializeUI()
{
    if (!PlayerHUD)
    {
        PlayerHUD = CreateWidget<UPlayerHUD>(GetWorld(), PlayerHUDClass);
        PlayerHUD->SetHealth(GetHealth(), 1.f);
        PlayerHUD->SetShield(GetShield(), 1.f);
        PlayerHUD->AddToViewport(10);
    }
}

void ADCharacterPlayer::BeginPlay()
{
    NormalSpeed = GetCharacterMovement()->MaxWalkSpeed;
    
    if (IsLocallyControlled())
    {
        GetWorldTimerManager().SetTimer(Handle_RadarScan, this, &ADCharacterPlayer::RadarScanTick, ScanInterval, true, 0.f);
        
        APlayerController* PC = GetPlayerController();

        PC->SetInputMode(FInputModeGameOnly());

        if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
        {
            PDS->OnGetWeaponComplete.AddUObject(this, &ADCharacterPlayer::OnInitPlayer);
            PDS->GetPlayerWeapons(EGetWeaponCondition::Equipped);
        }

        AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectAdded);
        AbilitySystem->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ADCharacterPlayer::OnActiveGameplayEffectRemoved);

        InitializeUI();
    }

    Super::BeginPlay();

    if (GetLocalRole() == ROLE_Authority)
    {
        int32 Idx = 0;
        for (FAbilitySlot Slot : DefaultAbilitySlot)
        {
            if (Slot.AbilityType == EAbilityType::Initiative)
            {
                AbilitySystem->GiveAbility(FGameplayAbilitySpec(Slot.AbilityClass, Slot.AbilityLevel, Idx));
                Idx++;
            }
            else
            {
                AbilitySystem->GiveAbility(FGameplayAbilitySpec(Slot.AbilityClass, Slot.AbilityLevel));
                //AbilitySystem->TryActivateAbilityByClass(Slot.AbilityClass);
            }
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

    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ADCharacterPlayer::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ADCharacterPlayer::StopCrouch);

    PlayerInputComponent->BindAction("OpenFire", IE_Pressed, this, &ADCharacterPlayer::StartFire);
    PlayerInputComponent->BindAction("OpenFire", IE_Released, this, &ADCharacterPlayer::StopFire);

    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ADCharacterPlayer::EquipWeapon);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADCharacterPlayer::ReloadMagazine);

    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ADCharacterPlayer::StartAim);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ADCharacterPlayer::StopAim);

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

    float FireInterval = ActiveWeapon->GetFireInterval();

    bool bCannotFire = GetWorld()->TimeSeconds <= (ActiveWeapon->GetLastFireTimeSeconds() + FireInterval);
    if (WeaponStatus > EWeaponStatus::Firing || (bCannotFire && ActiveWeapon->FireMode != EFireMode::Accumulation))
    {
        return;
    }

    ActiveWeapon->BP_OnStartFire();

    StopSprint();
    SetPawnStatus(EPawnStatus::Combat);
    WeaponStatus = EWeaponStatus::Firing;

    switch (ActiveWeapon->FireMode)
    {
        case EFireMode::SemiAutomatic:
            {
                HandleFire();
            }
            break;
        case EFireMode::FullyAutomatic:
            {
                GetWorldTimerManager().SetTimer(Handle_Shoot, this, &ADCharacterPlayer::HandleFire, FireInterval, true, 0.f);
            }
            break;
        case EFireMode::Accumulation:
            {
                GetWorldTimerManager().SetTimer(Handle_Shoot, this, &ADCharacterPlayer::HandleFire, FireInterval, false);
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

    if (ActiveWeapon->AmmoNum == 0)
    {
        ReloadMagazine();
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

    WeaponStatus = EWeaponStatus::Equipping;
        
    StopFire();
    StopAim();
    HandleStopReload();

    ServerEquipWeapon();

    ActiveWeapon->SetWeaponEnable(false);

    float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
    GetWorldTimerManager().SetTimer(Handle_Equip, this, &ADCharacterPlayer::EquipmentFinished, Duration);
}

void ADCharacterPlayer::MulticastEquipWeapon_Implementation()
{
    if (GetLocalRole() != ROLE_SimulatedProxy)
    {
        return;
    }

    PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
}

void ADCharacterPlayer::ServerEquipWeapon_Implementation()
{
    MulticastEquipWeapon();
    float Duration = PlayMontage(GetCurrentActionMontage()->EquipAnim, nullptr);
    GetWorldTimerManager().SetTimer(Handle_Equip, this, &ADCharacterPlayer::EquipmentFinished, Duration);
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

        SetPawnStatus(EPawnStatus::Combat);
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

    PerkEffectSystem->ActivationTimeFrame(ETimeFrame::Reloading);

    AttemptSetStatusToRelax();
}

void ADCharacterPlayer::ServerReloadMagazine_Implementation()
{
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

void ADCharacterPlayer::OnReq_ActiveWeapon()
{
    SetActiveWeapon(ActiveWeapon);
}

void ADCharacterPlayer::SetInventoryWeapon(int32 Index, /*int32 WeaponAttackPower,*/ TSubclassOf<AShootWeapon> NewWeaponClass)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerSetInventoryWeapon(Index, NewWeaponClass);
        return;
    }

    if (Index >= 0 && Index < WeaponInventory.Num())
    {
        FActorSpawnParameters SpawnParam;
        SpawnParam.Owner = this;
        SpawnParam.Instigator = this;
        AShootWeapon* NewWeapon = GetWorld()->SpawnActor<AShootWeapon>(NewWeaponClass, SpawnParam);
        //NewWeapon->WeaponAttribute.AttackPower = WeaponAttackPower;

        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, GetWeaponSlotNameFromIndex(Index));
        
        if (AShootWeapon* OldWeapon = WeaponInventory[Index])
        {
            OldWeapon->Destroy();
        }
        
        WeaponInventory[Index] = NewWeapon;

        ClientSetLocalEquippedWeaponClass(Index, NewWeaponClass);
        
        if (Index == ActiveWeaponIndex || (Index == 0 && ActiveWeaponIndex == -1))
        {
            NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
            
            ActiveWeaponIndex = Index;
            SetActiveWeapon(NewWeapon);
        }
        else
        {
            NewWeapon->SetActorRelativeTransform(Index == 1 ? NewWeapon->LeftSocketOffset : NewWeapon->RightSocketOffset);
        }
    }
    else
    {
        DREAM_NLOG(Error, TEXT("SetInventoryWeapon Fatal Error"));
    }
}

const TArray<TSubclassOf<AShootWeapon>>& ADCharacterPlayer::GetLocalEquippedWeaponClass() const
{
    return LocalEquippedWeaponClass;
}

void ADCharacterPlayer::EquippedGear(TSubclassOf<AGearBase> GearClass)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerEquippedGear(GearClass);
        return;
    }

    FActorSpawnParameters SpawnParam;
    SpawnParam.Owner = this;
    SpawnParam.Instigator = this;
    AGearBase* Gear = GetWorld()->SpawnActor<AGearBase>(GearClass, SpawnParam);
    Gear->Equipped(GetMesh());

    if (AGearBase* PrevGear = Gears.Add(Gear->Type, Gear))
    {
        PrevGear->Destroy();
    }

    RefreshAttributes();
}

void ADCharacterPlayer::ClearCombatStatus()
{
    StopFire();
    StopAim();
    HandleStopReload();
    CombatStatusCounter.Reset();
    GetWorldTimerManager().SetTimer(Handle_CombatToRelax, this, &ADCharacterPlayer::ModStatusToRelax, OutOfCombatTime);
}

void ADCharacterPlayer::ServerSetInventoryWeapon_Implementation(int32 Index, TSubclassOf<AShootWeapon> NewWeaponClass)
{
    SetInventoryWeapon(Index, NewWeaponClass);
}

void ADCharacterPlayer::ServerEquippedGear_Implementation(TSubclassOf<class AGearBase> GearClass)
{
    EquippedGear(GearClass);
}

void ADCharacterPlayer::SwitchWeapon(int32 NewWeaponIndex)
{
    if (NewWeaponIndex >= WeaponInventory.Num())
    {
        return;
    }

    FName PrevSlotName = WeaponInventory[NewWeaponIndex]->GetAttachParentSocketName();
    ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, PrevSlotName);
    ActiveWeapon->SetActorRelativeTransform(PrevSlotName == LeftShoulderSocketName ? ActiveWeapon->LeftSocketOffset : ActiveWeapon->RightSocketOffset);
    
    ActiveWeaponIndex = NewWeaponIndex;
    AShootWeapon* NewWeapon = WeaponInventory[ActiveWeaponIndex];
    NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
    NewWeapon->SetActorRelativeTransform(NewWeapon->WeaponSocketOffset);
    SetActiveWeapon(NewWeapon);
}

void ADCharacterPlayer::ActivateCharacterCapture()
{
    CharacterCapture->ShowOnlyActorComponents(this);
    ResetCharacterWeaponCapture();
    CharacterCapture->bCaptureEveryFrame = true;
}

void ADCharacterPlayer::DeactivateCharacterCapture()
{
    CharacterCapture->ClearShowOnlyComponents();
    CharacterCapture->bCaptureEveryFrame = false;
}

void ADCharacterPlayer::ResetCharacterWeaponCapture()
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

void ADCharacterPlayer::SetPlayerHUDVisible(bool bVisible)
{
    if (PlayerHUD)
    {
        PlayerHUD->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void ADCharacterPlayer::SetActiveWeapon(AShootWeapon* NewWeapon)
{
    if (!NewWeapon)
    {
        return;
    }

    BP_OnActiveWeaponChanged();
    ActiveWeapon = NewWeapon;

    if (GetLocalRole() == ROLE_Authority)
    {
        //NewWeapon->SetActorHiddenInGame(false);
        
        PerkEffectSystem->ApplyPerks(ActiveWeapon->WeaponAttribute.WeaponPerks);
        RefreshAttributes();
    }

    if (IsLocallyControlled())
    {
        ResetCharacterWeaponCapture();
        
        NewWeapon->SetWeaponEnable(true);
        PlayerHUD->SetCrossHairBrush(NewWeapon->GetDynamicCrosshairBrush());
        PlayerHUD->SetCrossHairVisibility(ESlateVisibility::Collapsed);
        PlayerHUD->SetMagazineBrush(NewWeapon->GetDynamicMagazineBrush());
        UpdateAmmoUI();
    }
}

void ADCharacterPlayer::OnInitPlayer(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage)
{
    FPlayerDataStoreModule::Get()->OnGetWeaponComplete.RemoveAll(this);
    
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
    return ActiveWeapon && WeaponStatus < EWeaponStatus::Equipping;
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
    if (Value == 0.f)
    {
        StopSprint();
    }
}

void ADCharacterPlayer::MoveRight(float Value)
{
    if ((Controller != NULL) && (Value != 0.0f))
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        // get right vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddControllerYawInput(Value * GetWorld()->GetDeltaSeconds());
        AddMovementInput(Direction, Value);
    }
}

AShootWeapon* ADCharacterPlayer::GetActiveWeapon() const
{
    return ActiveWeapon;
}

void ADCharacterPlayer::ClientSetLocalEquippedWeaponClass_Implementation(int32 Index, UClass* WeaponClass)
{
    LocalEquippedWeaponClass[Index] = WeaponClass;
}

void ADCharacterPlayer::StartAim()
{
    if (CanAim())
    {
        StopSprint();

        bAimed = true;
        ToggleCrossHairVisible(true);

        SetPawnStatus(EPawnStatus::Combat);
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


void ADCharacterPlayer::ServerStopAim_Implementation()
{
    AimedMoveSpeedChange(false);
}

void ADCharacterPlayer::AimedMoveSpeedChange(bool bNewAim)
{
    if (bNewAim)
    {
        GetCharacterMovement()->MaxWalkSpeed *= .5f;
    }
    else
    {
        float DefaultSpeed = Cast<ADCharacterPlayer>(GetClass()->ClassDefaultObject)->GetCharacterMovement()->
            MaxWalkSpeed;
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
    // get a cos(alpha) of forward vector vs velocity
    float ForwardCosAngle = FVector::DotProduct(GetActorRotation().Vector(), GetVelocity().GetUnsafeNormal2D());
    // now get the alpha and convert to degree
    float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

    if (ForwardDeltaDegree < 45.f && CanJump())
    {
        StopAim();
        StopFire();
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

void ADCharacterPlayer::StartCrouch()
{
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
    DOREPLIFETIME(ADCharacterPlayer, ReplicatedCtrlRotation);
}

void ADCharacterPlayer::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);
    ReplicatedCtrlRotation = GetControlRotation();
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
    if (GetLocalRole() == ROLE_Authority)
    {
        PerkEffectSystem->ActivationTimeFrame(ETimeFrame::Injured);
    }
    
    if (GetHealth() == 0.f && IsLocallyControlled())
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

        GetMesh()->SetAllBodiesSimulatePhysics(true);
        GetMesh()->SetAllBodiesPhysicsBlendWeight(1.f);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
    }

    UpdateHealthUI();

    Super::HealthChanged(AttrData);
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

    UKismetSystemLibrary::SphereTraceMultiForObjects(this, Location, Location + 1, RadarScanRadius,
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
    AttributeSet->SetAttackPower(ActiveWeapon->WeaponAttribute.AttackPower);
    
    for (TPair<EGearType, AGearBase*> Pair : Gears)
    {
        AttributeSet->SetDefensePower(Pair.Value->Defense);
        AttributeSet->SetCriticalDamage(Pair.Value->CriticalDamage);
        AttributeSet->SetCriticalRate(Pair.Value->CriticalRate);
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

void ADCharacterPlayer::HitEnemy(const FDamageTargetInfo& DamageInfo)
{
    if (DamageInfo.bKilled)
    {
        PerkEffectSystem->ActivationTimeFrame(ETimeFrame::KilledEnemy);
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

        const FHitResult* HitResult = EffectContext->GetHitResult();

        if (HitResult != nullptr)
        {
            SpawnDamageWidget(HitResult->ImpactPoint, DamageInfo.DamageAmount, DreamEffectContext->GetDamageCritical(), false);
            
            if (ADEnemyBase* Enemy = Cast<ADEnemyBase>(HitResult->GetActor()))
            {
                Enemy->ShowHealthUI();
            }
        }
        /*const TArray<FVector>& HitPoints = DreamEffectContext->GetHitPoints();
        if (HitPoints.Num() > 0)
        {
            for (FVector Point : HitPoints)
            {
                SpawnDamageWidget(Point, DamageInfo.DamageAmount, DreamEffectContext->GetDamageCritical(), false);
            }
        }
        else
        {
            if (HitResult != nullptr)
            {
                SpawnDamageWidget(HitResult->ImpactPoint, DamageInfo.DamageAmount, DreamEffectContext->GetDamageCritical(), false);
            }
        }*/
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

void ADCharacterPlayer::ServerSetPawnStatus_Implementation(EPawnStatus NewPawnStatus)
{
    SetPawnStatus(NewPawnStatus);
}

void ADCharacterPlayer::ModStatusToRelax()
{
    SetPawnStatus(EPawnStatus::Relax);
}

void ADCharacterPlayer::AttemptSetStatusToRelax()
{
    if (CombatStatusCounter.Decrement() == 0)
    {
        GetWorldTimerManager().SetTimer(Handle_CombatToRelax, this, &ADCharacterPlayer::ModStatusToRelax, OutOfCombatTime);
    }
}

FRotator ADCharacterPlayer::GetRemoteControllerRotation() const
{
    if (Controller && Controller->IsLocalPlayerController())
    {
        return GetControlRotation();
    }
    return ReplicatedCtrlRotation;
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

void ADCharacterPlayer::SetPawnStatus(EPawnStatus NewStatus)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerSetPawnStatus(NewStatus);
    }

    PawnStatus = NewStatus;

    if (NewStatus == EPawnStatus::Combat)
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
        bUseControllerRotationYaw = true;
        TPCameraArm->bEnableCameraLag = false;
        GetWorldTimerManager().SetTimer(Handle_CanTurn, [this] { bCanTurnPawn = true; }, .1f, false);

        if (IsLocallyControlled())
        {
            GetWorldTimerManager().ClearTimer(Handle_CombatToRelax);
            
            if (CombatStatusCounter.GetValue() < 0)
            {
                CombatStatusCounter.Reset();
            }
            CombatStatusCounter.Increment();
        }
    }
    else if (NewStatus == EPawnStatus::Relax)
    {
        GetWorldTimerManager().ClearTimer(Handle_CanTurn);
        bCanTurnPawn = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = false;
        TPCameraArm->bEnableCameraLag = true;
        TPCameraArm->CameraLagSpeed = 40.f;
    }
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
        
    }
}

bool ADCharacterPlayer::GetMiniMapTips(TArray<FMiniMapData>& Data)
{
    DoAddMiniMapTips(Data, InfiniteActors);
    DoAddMiniMapTips(Data, ActorWithinRadius);
    return Data.Num() > 0;
}

const FMontageSet* ADCharacterPlayer::GetCurrentActionMontage() const
{
    if (ActiveWeapon)
    {
        return WeaponMontage.Find(ActiveWeapon->AnimGroup);
    }
		
    return &InvalidMontage;
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
