// Fill out your copyright notice in the Description page of Project Settings.

#include "DMPlayerController.h"
#include "CallReceiver.h"
#include "DDropReward.h"
#include "JsonUtilities.h"
#include "DGameplayStatics.h"
#include "DPlayerCameraManager.h"
#include "DMProjectSettings.h"
#include "DreamGameMode.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "DreamWidgetStatics.h"
#include "PlayerDataInterfaceBase.h"
#include "Character/DCharacterPlayer.h"
#include "PlayerDataInterfaceStatic.h"
#include "UI/SGuide.h"

#define LOCTEXT_NAMESPACE "Player.DMController"

void FPlayerDesc::Initialize(const FPlayerInfo& PlayerInfo)
{
    Weapons.SetNumZeroed(PlayerInfo.Weapons.Num());
    Modules.SetNumZeroed(PlayerInfo.Modules.Num());

    for (const FPlayerWeapon& Weapon : PlayerInfo.Weapons)
    {
        if (!Weapon.bEquipped)
        {
            continue;
        }
        
        FGearDesc& Gear = Weapons[Weapon.Index];

        Gear.Attributes = Weapon.Attributes;
        Gear.EquippedIdx = Weapon.Index;
        Gear.GearLevel = Weapon.WeaponLevel;
        Gear.GearQuality = ItemUtils::GetItemQuality(Weapon.ItemGuid);
        Gear.GearClass = GSProject->GetItemClassFromGuid(Weapon.ItemGuid);
    }
    
    for (const FPlayerModule& Module : PlayerInfo.Modules)
    {
        if (!Module.bEquipped)
        {
            continue;
        }

        uint8 CategoryValue = static_cast<uint8>(Module.Category);

        FGearDesc& Gear = Modules[CategoryValue];

        Gear.Attributes = Module.Attributes;
        Gear.EquippedIdx = CategoryValue;
        Gear.GearLevel = Module.ModuleLevel;
        Gear.GearQuality = ItemUtils::GetItemQuality(Module.ItemGuid);
        Gear.GearClass = GSProject->GetItemClassFromGuid(Module.ItemGuid);
    }

    Skin = Cast<UCharacterMesh>(PlayerInfo.CharacterMesh.TryLoad());

    Level = PlayerInfo.CharacterLevel;
    Talents = PlayerInfo.LearnedTalents;
}

ADMPlayerController::ADMPlayerController()
    : Ammunition_L1(1.f)
    , Ammunition_L2(1.f)
    , Ammunition_L3(1.f)
    , bInitializedCachedPlayerDesc(false)
{
    PlayerCameraManagerClass = ADPlayerCameraManager::StaticClass();
}

void ADMPlayerController::GetPlayerInformation(FPlayerInfoSignature Delegate)
{
    if (bInitializedCachedPlayerDesc)
    {
        Delegate.Execute(true, CachedPlayerDesc);
    }
    else if (PlayerState)
    {
        FQueryPlayerParam Param;
        Param.PlayerId = PlayerState->GetPlayerId();
        Param.Condition = Query_Cond_Used;

        Handle_QueryPlayerData = GDataInterface->AddOnGetPlayerInfo(
            FOnGetPlayerInfo::FDelegate::CreateUObject(this, &ADMPlayerController::OnGetPlayerInfo, Delegate));
        
        GDataInterface->ServerGetPlayerInfo(Param);
    }
    else
    {
        Delegate.Execute(false, CachedPlayerDesc);
    }
}

FPlayerDesc& ADMPlayerController::GetCachedPlayerDesc()
{
    return CachedPlayerDesc;
}

void ADMPlayerController::OnGetPlayerInfo(const FPlayerInfo& PlayerInfo, bool bQueryResult, FPlayerInfoSignature Delegate)
{
    GDataInterface->RemoveOnGetPlayerInfo(Handle_QueryPlayerData);
    
    if (bQueryResult)
    {
        bInitializedCachedPlayerDesc = true;
        CachedPlayerDesc.Initialize(PlayerInfo);
        Delegate.Execute(true, CachedPlayerDesc);
    }
    else
    {
        Delegate.Execute(false, CachedPlayerDesc);
    }
}

void ADMPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        Handle_UpdateTask = GDataInterface->AddOnUpdateTaskCond(FOnUpdatedTasks::FDelegate::CreateUObject(this, &ADMPlayerController::OnUpdatedTasks));
        Handle_ReceiveRewards = GDataInterface->AddOnReceiveRewards(FOnReceiveRewards::FDelegate::CreateUObject(this, &ADMPlayerController::OnReceiveRewardMessage));

#if !WITH_EDITOR
        if (GetNetMode() == NM_Client)
        {
#endif

        
        GetLocalPlayer()->ViewportClient->AddViewportWidgetContent(
                SNew(SGuide)
                .PlayerOwner(this)
                .Elements_UObject(this, &ADMPlayerController::GetGuideElements)
            );

#if !WITH_EDITOR
        }
#endif
        
    }
}

void ADMPlayerController::BeginInactiveState()
{
    // 不执行 APlayerController 的逻辑
}

void ADMPlayerController::NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest)
{
    Super::NotifyLoadedWorld(WorldPackageName, bFinalDest);
    // 无缝旅行加载通知
    DREAM_NLOG(Verbose, TEXT("NotifyLoadedWorld"));
}

void ADMPlayerController::ExitTeam()
{
    ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("TEST")));
}

void ADMPlayerController::PopupRewards(const FItemListHandle& ItemList)
{
    OnReceiveRewardMessage(ItemList);
}

void ADMPlayerController::AddGuideActor(AActor* Actor)
{
    if (Actor == nullptr)
    {
        return;
    }
    
    if (Actor->GetNativeInterfaceAddress(IIconInterface::UClassType::StaticClass()) != nullptr)
    {
        GuideActors.AddUnique(Actor);
    }
    else if (Actor->GetClass()->ImplementsInterface(IIconInterface::UClassType::StaticClass()))
    {
        GuideActors.AddUnique(Actor);
    }
}

void ADMPlayerController::RemoveGuideActor(AActor* Actor)
{
    GuideActors.Remove(Actor);
}

void ADMPlayerController::CallServerActor(FGameplayTag CallTag, AActor* TargetActor)
{
    if (TargetActor)
    {
        ServerCallServerActor(CallTag, TargetActor);
    }
}

void ADMPlayerController::OnRespawnPlayer()
{
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();

    if (GameMode == nullptr)
    {
        return;
    }

    if (GetPawn() != nullptr)
    {
        GetPawn()->Destroy();
    }

    if (GameMode->PlayerCanRestart(this))
    {
        GameMode->RestartPlayer(this);
    }
}

void ADMPlayerController::FailedToSpawnPawn()
{
    Super::FailedToSpawnPawn();

    GetWorldTimerManager().SetTimerForNextTick(this, &ADMPlayerController::OnRespawnPlayer);
}

void ADMPlayerController::HandlePlayerCharacterDie()
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (ADreamGameMode* GameMode = GetWorld()->GetAuthGameMode<ADreamGameMode>())
    {
        GameMode->OnPlayerCharacterDie(this);

        if (GameMode->PlayerCanRestart(this))
        {
            GetWorldTimerManager().SetTimer(Handle_RespawnPlayer, this, &ADMPlayerController::OnRespawnPlayer, GetMinRespawnDelay());
        }
    }
}

void ADMPlayerController::CancelRespawnCharacter()
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }
    
    GetWorldTimerManager().ClearTimer(Handle_RespawnPlayer);
}

void ADMPlayerController::ServerCallServerActor_Implementation(const FGameplayTag& Tag, AActor* TargetActor)
{
    if (TargetActor == nullptr)
    {
        UE_LOG(LogDream, Warning, TEXT("TargetActor Invalid"));
        return;
    }
    
    if (TargetActor->GetClass()->ImplementsInterface(ICallReceiver::UClassType::StaticClass()))
    {
        ICallReceiver::Execute_ReceiveCall(TargetActor, Tag);
    }
    else if (ICallReceiver* CallReceiver = Cast<ICallReceiver>(TargetActor))
    {
        CallReceiver->ReceiveCall(Tag);
    }
}

void ADMPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (IsLocalController())
    {
        GDataInterface->RemoveOnUpdateTaskCond(Handle_UpdateTask);
        GDataInterface->RemoveOnReceiveRewards(Handle_ReceiveRewards);
    }
}

void ADMPlayerController::ClientChangeConnectedServer_Implementation(uint32 Address, uint32 Port)
{
    FOnReconnectServer Delegate;

    Delegate.BindLambda([this](bool bSuccessfully)
    {
        if (!bSuccessfully)
        {
            FString ErrorMsg = TEXT("改变后端服务器连接失败");
            UE_LOG(LogDream, Error, TEXT("ChangeServer %s"), *ErrorMsg);
            ClientReturnToMainMenuWithTextReason(FText::FromString(ErrorMsg));
        }
    });
    
    GDataInterface->ReconnectToSpecifiedServer(Address, Port, Delegate);
}

void ADMPlayerController::OnUpdatedTasks(const TArray<FTaskInProgressInfo>& UpdatedTasks)
{
    BP_OnTrackingTaskUpdated(UpdatedTasks);
}

void ADMPlayerController::ClientWasKicked_Implementation(const FText& KickReason)
{
    UDreamWidgetStatics::PopupDialog(EDialogType::WARNING, KickReason, 2.f);
}

void ADMPlayerController::ServerSendMessage_Implementation(const FTalkMessage& Message)
{
    if (!IsLocalController() && GetLocalRole() != ROLE_Authority)
    {
        MulticastMessage(Message);
    }
}

void ADMPlayerController::MulticastMessage_Implementation(const FTalkMessage& Message)
{
    OnReceiveTalk(Message);
}

void ADMPlayerController::SendTalkMessage(const FTalkMessage& Message)
{
    ServerSendMessage(Message);
}

void ADMPlayerController::OnReceiveRewardMessage(const FItemListHandle& ItemList)
{
    for (const TSharedPtr<FItem>& ItemData : ItemList)
    {
        int32 ItemGuid = ItemData->GetItemGuid();
        const FItemDef& ItemDef = GSProject->GetItemDefinition(ItemGuid);

        EItemType::Type ItemType = ItemUtils::GetItemType(ItemGuid);

        ERewardNotifyMode NotifyMode = ERewardNotifyMode::Primary;

        switch (ItemType)
        {
        case EItemType::Material:
        case EItemType::Experience:

            NotifyMode = ERewardNotifyMode::Secondary;
            break;

        default:;
        }

        OnPopupRewards(ItemDef.ItemBaseInfo, NotifyMode, ItemData->GetItemNum());
    }
}

TArray<FGuideElement> ADMPlayerController::GetGuideElements() const
{
    TArray<FGuideElement> Elements;

    if (GuideActors.Num() > 0)
    {
        for (AActor* GuideActor : GuideActors)
        {
            if (!GuideActor || GuideActor->IsPendingKillPending())
            {
                continue;
            }

            IIconInterface* IconInterface = Cast<IIconInterface>(GuideActor);
            if (IconInterface == nullptr)
            {
                continue;
            }

            UIconComponent* IconComponent = IconInterface->GetIconComponent();
            if (!IconComponent->IsActive())
            {
                continue;
            }

            FGuideElement Element;
            Element.Location = GuideActor->GetActorLocation();
            Element.Brush = &IconComponent->Icon;

            Elements.Add(Element);
        }
    }
     
    return Elements;
}


void ADMPlayerController::ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason)
{
    UDreamWidgetStatics::PopupDialog(EDialogType::INFO, ReturnReason);
    UDGameplayStatics::ReturnToHomeWorld(this);
}

bool ADMPlayerController::AddWeaponAmmunition(EAmmoType AmmoType, float RecoveryAmount)
{
    float& Ammunition = GetWeaponAmmunition(AmmoType);
    
    if (FMath::IsNearlyEqual(Ammunition, 1.f))
    {
        return false;
    }
    
    Ammunition = FMath::Clamp(Ammunition + RecoveryAmount, 0.f, 1.f);

    OnAmmunitionChange.Broadcast(AmmoType, Ammunition);
    
    return true;
}

void ADMPlayerController::SetWeaponAmmunition(EAmmoType AmmoType, float NewAmmunition)
{
    float& Ammunition = GetWeaponAmmunition(AmmoType);
    Ammunition = FMath::Clamp(NewAmmunition, 0.f, 1.f);

    OnAmmunitionChange.Broadcast(AmmoType, Ammunition);
}

float& ADMPlayerController::GetWeaponAmmunition(EAmmoType AmmoType)
{
    return AmmoType == EAmmoType::Level1
               ? Ammunition_L1
               : AmmoType == EAmmoType::Level2
               ? Ammunition_L2
               : Ammunition_L3;
}

void ADMPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


#undef LOCTEXT_NAMESPACE
