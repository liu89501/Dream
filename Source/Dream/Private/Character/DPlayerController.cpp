// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DPlayerController.h"
#include "JsonUtilities.h"
#include "DGameplayStatics.h"
#include "DPlayerCameraManager.h"
#include "DProjectSettings.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "DreamGameInstance.h"
#include "DreamGameMode.h"
#include "DreamWidgetStatics.h"
#include "OnlineSubsystem.h"
#include "PlayerDataInterfaceBase.h"
#include "Character/DCharacterPlayer.h"
#include "PlayerDataInterfaceStatic.h"
#include "PlayerServerDataInterface.h"
#include "UI/SGuide.h"

#define LOCTEXT_NAMESPACE "Player.Controller"

ADPlayerController::ADPlayerController()
    : LevelOneAmmunition(800)
      , LevelTwoAmmunition(1000)
      , LevelThreeAmmunition(60)
{
    PlayerCameraManagerClass = ADPlayerCameraManager::StaticClass();
}

void ADPlayerController::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void ADPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        FPlayerDataInterface* Interface = FPDIStatic::Get();
        Handle_UpdateTask = Interface->AddOnUpdateTaskCond(FOnUpdatedTasks::FDelegate::CreateUObject(this, &ADPlayerController::OnUpdatedTasks));
        Handle_ReceiveRewards = Interface->AddOnReceiveRewards(FOnReceiveRewards::FDelegate::CreateUObject(this, &ADPlayerController::OnReceiveRewardMessage));

#if !WITH_EDITOR
        if (GetNetMode() == NM_Client)
        {
#endif

        
        GetLocalPlayer()->ViewportClient->AddViewportWidgetContent(
                SNew(SGuide)
                .PlayerOwner(this)
                .Elements_UObject(this, &ADPlayerController::GetGuideElements)
            );

#if !WITH_EDITOR
        }
#endif
        
    }
}

void ADPlayerController::NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest)
{
    Super::NotifyLoadedWorld(WorldPackageName, bFinalDest);
    // 无缝旅行加载通知
    DREAM_NLOG(Verbose, TEXT("NotifyLoadedWorld"));
}

void ADPlayerController::ExitTeam()
{
    UE_LOG(LogDream, Error, TEXT("%s"), *GetWorld()->GetMapName());
    ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("TEST")));
}

void ADPlayerController::PopupRewards(const FItemListHandle& ItemList)
{
    OnReceiveRewardMessage(ItemList);
}

void ADPlayerController::AddGuideActor(AActor* Actor)
{
    if (Actor && Actor->GetNativeInterfaceAddress(IIconInterface::UClassType::StaticClass()) != nullptr)
    {
        GuideActors.AddUnique(Actor);
    }
}

void ADPlayerController::RemoveGuideActor(AActor* Actor)
{
    GuideActors.Remove(Actor);
}

void ADPlayerController::BroadcastGameModeClientDelegate(FGameplayTag DelegateTag)
{
    ServerBroadcastGMClientDelegate(DelegateTag);
}

void ADPlayerController::ServerBroadcastGMClientDelegate_Implementation(const FGameplayTag& DelegateTag)
{
    if (ADreamGameMode* DreamGameMode = Cast<ADreamGameMode>(GetWorld()->GetAuthGameMode()))
    {
        DreamGameMode->BroadcastClientRPCDelegate(DelegateTag);
    }
}

void ADPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    FPDIStatic::Get()->RemoveOnUpdateTaskCond(Handle_UpdateTask);
    FPDIStatic::Get()->RemoveOnReceiveRewards(Handle_ReceiveRewards);
}

#if WITH_EDITORONLY_DATA

void ADPlayerController::TestLoginServer()
{
    FPDIStatic::Get()->Login();
}

void ADPlayerController::TestCreateSession()
{
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
    {
        FOnlineSessionSettings Settings;
        Settings.bUsesPresence = true;
        Settings.bAllowJoinInProgress = true;
        Settings.bShouldAdvertise = true;
        Settings.bAllowInvites = true;
        Settings.bAllowJoinViaPresence = true;

        FString MapName(TEXT("TEST_MAP"));
        Settings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
        Settings.NumPublicConnections = 4;
        OSS->GetSessionInterface()->CreateSession(0, GameSessionName, Settings);
    }
}

void ADPlayerController::TestFindSession()
{
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
    {
        TSharedRef<FOnlineSessionSearch> Search = MakeShareable(new FOnlineSessionSearch);
        Search->MaxSearchResults = 2;
        Search->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

        FString MapName(TEXT("TEST_MAP"));
        Search->QuerySettings.Set(SETTING_MAPNAME, MapName, EOnlineComparisonOp::Equals);

        IOnlineSessionPtr SessionInt = OSS->GetSessionInterface();

        FOnFindSessionsCompleteDelegate Delegate = FOnFindSessionsCompleteDelegate::CreateLambda(
            [Search]
        (bool bSuccess)
            {
                if (Search->SearchResults.Num() > 0)
                {
                    for (FOnlineSessionSearchResult Result : Search->SearchResults)
                    {
                        DumpSession(&Result.Session);
                    }
                }
                else
                {
                    DREAM_NLOG(Verbose, TEXT("Not Func Session"));
                }
            }
        );

        SessionInt->AddOnFindSessionsCompleteDelegate_Handle(Delegate);
        SessionInt->FindSessions(0, Search);
    }
}

void ADPlayerController::TestSocket() const
{
    FPlayerDataInterfaceBase* InterfaceBase = static_cast<FPlayerDataInterfaceBase*>(FPDIStatic::Get());
    InterfaceBase->GetSender()->Send(PDIBuildParam<TService_PlayerInfo>(EGetEquipmentCondition::Equipped));
}

#endif

void ADPlayerController::ClientChangeConnectedServer_Implementation(uint32 Address, uint32 Port)
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
    
    FPDIStatic::Get()->ReconnectToSpecifiedServer(Address, Port, Delegate);
}

void ADPlayerController::OnUpdatedTasks(const TArray<FTaskInProgressInfo>& UpdatedTasks)
{
    BP_OnTrackingTaskUpdated(UpdatedTasks);
}

void ADPlayerController::ClientWasKicked_Implementation(const FText& KickReason)
{
    UDreamWidgetStatics::PopupDialog(this, EDialogType::WARNING, KickReason, 2.f);
    UDGameplayStatics::ReturnToHomeWorld(this);
}

void ADPlayerController::ServerSendMessage_Implementation(const FTalkMessage& Message)
{
    if (!IsLocalController() && GetLocalRole() != ROLE_Authority)
    {
        MulticastMessage(Message);
    }
}

void ADPlayerController::MulticastMessage_Implementation(const FTalkMessage& Message)
{
    BP_OnReceiveTalkMessage(Message);
}

void ADPlayerController::SendTalkMessage(const FTalkMessage& Message)
{
    ServerSendMessage(Message);
}

int32& ADPlayerController::GetWeaponAmmunition(EAmmoType AmmoType)
{
    return AmmoType == EAmmoType::Level1
               ? LevelOneAmmunition
               : AmmoType == EAmmoType::Level2
               ? LevelTwoAmmunition
               : LevelThreeAmmunition;
}

int32 ADPlayerController::GetWeaponAmmunition(EAmmoType AmmoType) const
{
    return AmmoType == EAmmoType::Level1
               ? LevelOneAmmunition
               : AmmoType == EAmmoType::Level2
               ? LevelTwoAmmunition
               : LevelThreeAmmunition;
}

int32 ADPlayerController::GetDefaultAmmunition(EAmmoType AmmoType) const
{
    const ADPlayerController* CDO = GetDefault<ADPlayerController>();
    return AmmoType == EAmmoType::Level1
               ? CDO->LevelOneAmmunition
               : AmmoType == EAmmoType::Level2
               ? CDO->LevelTwoAmmunition
               : CDO->LevelThreeAmmunition;
}

void ADPlayerController::OnReceiveRewardMessage(const FItemListHandle& ItemList)
{
    for (UItemData* ItemData : ItemList.Items)
    {
        UClass* ItemClass = UDProjectSettings::GetProjectSettings()->GetItemClassFromGuid(ItemData->ItemGuid);

        if (ItemClass == nullptr)
        {
            continue;
        }

        if (IPropsInterface* PropsInterface = Cast<IPropsInterface>(ItemClass->GetDefaultObject()))
        {
            BP_ReceiveRewardMessage(PropsInterface->GetPropsInfo(), PropsInterface->GetRewardNotifyMode(), ItemData->GetItemNum());
        }
    }
}

TArray<FGuideElement> ADPlayerController::GetGuideElements() const
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

void ADPlayerController::ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason)
{
    Super::ClientReturnToMainMenuWithTextReason_Implementation(ReturnReason);
    
    UDreamWidgetStatics::PopupDialog(this, EDialogType::ERROR, ReturnReason, 3.f);
}

bool ADPlayerController::AddWeaponAmmunition(EAmmoType AmmoType, int32 AmmunitionAmount)
{
    const ADPlayerController* CDO = GetDefault<ADPlayerController>();

    bool bSuccess = false;

    switch (AmmoType)
    {
    case EAmmoType::Level1:
        bSuccess = LevelOneAmmunition < CDO->LevelOneAmmunition;
        break;
    case EAmmoType::Level2:
        bSuccess = LevelTwoAmmunition < CDO->LevelTwoAmmunition;
        break;
    case EAmmoType::Level3:
        bSuccess = LevelThreeAmmunition < CDO->LevelThreeAmmunition;
        break;
    default:
        break;
    }

    if (bSuccess)
    {
        int32 DAmmunition = GetDefaultAmmunition(AmmoType);
        int32& Ammunition = GetWeaponAmmunition(AmmoType);
        Ammunition = FMath::Min(Ammunition + AmmunitionAmount, DAmmunition);
    }

    return bSuccess;
}

void ADPlayerController::SetWeaponAmmunition(EAmmoType AmmoType, int32 NewValue)
{
    int32& Ammunition = GetWeaponAmmunition(AmmoType);
    Ammunition = NewValue;
}

void ADPlayerController::DecrementAmmunition(EAmmoType AmmoType, int32 Amount)
{
    int32& Ammunition = GetWeaponAmmunition(AmmoType);
    Ammunition -= Amount;
}

void ADPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


#undef LOCTEXT_NAMESPACE
