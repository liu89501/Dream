// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DPlayerController.h"
#include "OnlineSessionInterface.h"
#include "SocketSubsystem.h"
#include "UdpSocketBuilder.h"
#include "JsonUtilities.h"
#include "Async.h"
#include "DGameplayStatics.h"
#include "DPlayerCameraManager.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "DreamGameInstance.h"
#include "DreamGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSessionSettings.h"
#include "Character/DCharacterPlayer.h"
#include "PlayerDataInterfaceStatic.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

#define LOCTEXT_NAMESPACE "Player.Controller"

typedef TSharedRef<TArray<uint8>, ESPMode::ThreadSafe> RawData;

bool FRewardMessage::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    uint8 Sign = 0;
    
    if (Ar.IsSaving())
    {
        if (RewardPropsClass)
        {
            Sign |= 1; 
        }

        if (RewardNum > 1)
        {
            Sign |= 1 << 1;
        }
    }

    Ar.SerializeBits(&Sign, 2);

    if (Sign & 1)
    {
        Ar << RewardPropsClass;
    }

    if (Sign & 1 << 1)
    {
        Ar << RewardNum;
    }

    bOutSuccess = true;
    return true;
}

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
        if (ISocketSubsystem* SocketSystem = ISocketSubsystem::Get())
        {
            ClientSocket = SocketSystem->CreateSocket(TEXT("SteamClientSocket"), TEXT("LocalSteamClientSocket"));
            FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(1000);
            UdpReceiver = new FUdpSocketReceiver(ClientSocket, ThreadWaitTime, TEXT("ClientReceiver"));
            UdpReceiver->OnDataReceived().BindUObject(this, &ADPlayerController::RecvData);
            UdpReceiver->Start();
        }
    }
}

void ADPlayerController::NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest)
{
    Super::NotifyLoadedWorld(WorldPackageName, bFinalDest);
    // 无缝旅行加载通知
    DREAM_NLOG(Verbose, TEXT("NotifyLoadedWorld"));
}

void ADPlayerController::Travel(const FString& Address)
{
    ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void ADPlayerController::AgreeTeamUpApply(const FPlayerAccountInfo& PlayerInfo)
{
    
}

void ADPlayerController::ExitTeam()
{
    UE_LOG(LogDream, Error, TEXT("%s"), *GetWorld()->GetMapName());
    ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("TEST")));
}

void ADPlayerController::ProcessRebornCharacter(const FTransform& SpawnTransform)
{
    FActorSpawnParameters Params;
    Params.Owner = this;
    ADreamGameMode* AuthGameMode = GetWorld()->GetAuthGameMode<ADreamGameMode>();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    ACharacter* RiseCharacter = GetWorld()->SpawnActor<ACharacter>(AuthGameMode->DefaultPawnClass, SpawnTransform, Params);
    Possess(RiseCharacter);
}

void ADPlayerController::OnGetPlayerInfoComplete(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage)
{
    //OnlinePlayerInfo = PlayerInfo;
    
    if (!ErrorMessage.IsEmpty())
    {
        ReturnToMainMenuWithTextReason(LOCTEXT("PlayerInfo_Invalid", "玩家数据获取失败"));
    }
}

void ADPlayerController::ReturnToMainMenuWithTextReason(const FText& ErrorMessage)
{
    ClientReturnToMainMenuWithTextReason(ErrorMessage);
}

void ADPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    delete UdpReceiver;
    if (ClientSocket)
    {
        ISocketSubsystem::Get(STEAM_SUBSYSTEM)->DestroySocket(ClientSocket);
    }
}

void ADPlayerController::TestLoginServer()
{
    if (FPlayerDataInterface* PDS = FPlayerDataInterfaceStatic::Get())
    {
        PDS->Login(FCommonCompleteNotify());
    }
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
                        DREAM_NLOG(Log, TEXT("Result, SessionId: %s, SessionInfo: %s"),
                                   *Result.GetSessionIdStr(), *Result.Session.SessionInfo->ToDebugString());
                    }
                }
            }
        );

        SessionInt->AddOnFindSessionsCompleteDelegate_Handle(Delegate);
        SessionInt->FindSessions(0, Search);
    }
}

void ADPlayerController::RecvData(const TSharedPtr<FArrayReader, ESPMode::ThreadSafe>& ReaderPtr,
                                  const FIPv4Endpoint& Endpoint)
{
    AsyncTask(ENamedThreads::GameThread, [this, ReaderPtr]
    {
        uint8 MessageType = (*ReaderPtr)[0];
        uint8* MessagePtr = ReaderPtr->GetData();
        FUTF8ToTCHAR Data(reinterpret_cast<const ANSICHAR*>(++MessagePtr), ReaderPtr->Num() - 1);
        HandleRawMessage(TEnumAsByte<EMessageType::Type>(MessageType), FString(Data.Length(), Data.Get()));
    });
}

void ADPlayerController::HandleRawMessage(EMessageType::Type Type, const FString& Message)
{
    //DREAM_NLOG(Verbose, TEXT("HandleRawMessage: %s, MessageType: %d"), *Message, Type);

    switch (Type)
    {
    case EMessageType::Kill_Message:
        {
            FKillMessage KillMessage;
            FJsonObjectConverter::JsonObjectStringToUStruct(Message, &KillMessage, 0, 0);
            BP_OnReceiveKillMessage(KillMessage);
            break;
        }
    case EMessageType::Talk_Message:
        {
            FTalkMessage TalkMessage;
            FJsonObjectConverter::JsonObjectStringToUStruct(Message, &TalkMessage, 0, 0);
            BP_OnReceiveTalkMessage(TalkMessage);
            break;
        }
    case EMessageType::TeamApply_Message:
        {
            FPlayerAccountInfo PlayerInfo;
            FJsonObjectConverter::JsonObjectStringToUStruct(Message, &PlayerInfo, 0, 0);
            // Todo 组队逻辑 待处理
            break;
        }
    default:
        break;
    }
}

void ADPlayerController::SendMulticastMessage(TEnumAsByte<EMessageType::Type> Type, const FString& Message)
{
    for (TActorIterator<ADCharacterPlayer> PlayerPawns(GetWorld()); PlayerPawns; ++PlayerPawns)
    {
        APlayerState* PawnPlayerState = PlayerPawns->GetPlayerState();

        if (!PawnPlayerState || *PlayerPawns == GetPawn())
        {
            continue;
        }

        FUniqueNetIdRepl UniqueId = PawnPlayerState->GetUniqueId();

        if (UniqueId.IsValid())
        {
            SendMessageToPlayer(UniqueId, Type, Message);
        }
    }
}

void ADPlayerController::SendMessageToPlayer(const FUniqueNetIdRepl& UserNetId, TEnumAsByte<EMessageType::Type> Type, const FString& Message)
{
    FTCHARToUTF8 Utf8Msg(*Message);
    TArray<uint8> Data;
    Data.Add(Type);
    Data.Append(reinterpret_cast<const uint8*>(Utf8Msg.Get()), Utf8Msg.Length());

    TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(STEAM_SUBSYSTEM)->CreateInternetAddr();

    bool bValid;
    Addr->SetIp(*UserNetId->ToString(), bValid);

    if (bValid)
    {
        int32 SendBytes;
        if (!ClientSocket->SendTo(Data.GetData(), Data.Num(), SendBytes, *Addr))
        {
            DREAM_NLOG(Error, TEXT("ClientSocket SendTo Error"));
        }
    }
    else
    {
        DREAM_NLOG(Error, TEXT("SetIp Error"));
    }
}

void ADPlayerController::ServerMulticastMessage_Implementation(EMessageType::Type Type, const FString& Message)
{
    NetMulticastMessage(Type, Message);
}

void ADPlayerController::ClientReceiveMessage_Implementation(EMessageType::Type Type, const FString& Message)
{
    HandleRawMessage(Type, Message);
}

void ADPlayerController::NetMulticastMessage_Implementation(EMessageType::Type Type, const FString& Message)
{
    HandleRawMessage(Type, Message);
}

void ADPlayerController::SendTeamUpApply(APlayerState* TargetPlayerState)
{
    if (TargetPlayerState)
    {
        FUniqueNetIdRepl TargetUniqueId = TargetPlayerState->GetUniqueId();

        FUniqueNetIdRepl MyUniqueId = PlayerState->GetUniqueId();

        if (TargetUniqueId.IsValid() && MyUniqueId.IsValid())
        {
            FPlayerAccountInfo Info;
            Info.AccountID = MyUniqueId->ToString();
            Info.PlayerName = PlayerState->GetPlayerName();

            FString JsonMessage;
            FJsonObjectConverter::UStructToJsonObjectString(Info, JsonMessage, 0, 0, 0, nullptr, false);
            SendMessageToPlayer(TargetUniqueId, EMessageType::TeamApply_Message, JsonMessage);
        }
    }
}

void ADPlayerController::SendKillMessage(const FKillMessage& Message)
{
    FString JsonMessage;
    FJsonObjectConverter::UStructToJsonObjectString(Message, JsonMessage, 0, 0, 0, nullptr, false);
    SendMulticastMessage(EMessageType::Kill_Message, JsonMessage);
}

void ADPlayerController::SendTalkMessage(const FTalkMessage& Message)
{
    FString JsonMessage;
    FJsonObjectConverter::UStructToJsonObjectString(Message, JsonMessage, 0, 0, 0, nullptr, false);
    //SendMulticastMessage(EMessageType::Talk_Message, JsonMessage);
    ServerMulticastMessage(EMessageType::Talk_Message, JsonMessage);
}

void ADPlayerController::ExitGame()
{
    FPlatformMisc::RequestExit(false);
}

void ADPlayerController::BeginGame()
{
    FPlayerDataInterfaceStatic::Get()->Login(FCommonCompleteNotify());
}

void ADPlayerController::HandleTeamApply()
{
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

void ADPlayerController::ClientReceiveRewardMessage_Implementation(const TArray<FRewardMessage>& RewardMessages)
{
    for (const FRewardMessage& RewardMessage : RewardMessages)
    {
        if (RewardMessage.RewardPropsClass)
        {
            if (IPropsInterface* PropsInterface = Cast<IPropsInterface>(RewardMessage.RewardPropsClass->GetDefaultObject()))
            {
                BP_ReceiveRewardMessage(PropsInterface->GetPropsInfo(), PropsInterface->GetRewardNotifyMode(), RewardMessage.RewardNum);
            }
        }
    }
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

void ADPlayerController::ServerAddWeaponAmmunition_Implementation(EAmmoType AmmoType, int32 AmmunitionAmount)
{
    int32 DAmmunition = GetDefaultAmmunition(AmmoType);
    int32& Ammunition = GetWeaponAmmunition(AmmoType);
    Ammunition = FMath::Min(Ammunition + AmmunitionAmount, DAmmunition);
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
