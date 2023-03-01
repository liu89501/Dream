// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchmakingCallProxy.h"
#include "OnlineSubsystem.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "TimerManager.h"
#include "DreamGameMode.h"
#include "DreamLoadingScreen.h"
#include "GameMapsSettings.h"
#include "OnlineSubsystemSessionSettings.h"
#include "OnlineSubsystemUtils.h"

#define GAME_SERVER_KEY TEXT("GameServerAddress")

#define START_WAIT_TIME 30
#define TICK_INTERVAL 1
#define SearchTimeout 120


void FMatchmakingHandle::Clear()
{
	if (!World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(Handle_Ticker);
	World->GetTimerManager().ClearTimer(Handle_Search);
	
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();

	if (Handle_UpdateSessionComplete.IsValid())
	{
		SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(Handle_UpdateSessionComplete);
	}
	
	if (Handle_FindSessionsComplete.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);
	}
	
	if (Handle_JoinSessionComplete.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(Handle_JoinSessionComplete);
	}
	
	if (Handle_CustomUpdateComplete.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(Handle_CustomUpdateComplete);
	}
	
	if (Handle_CreateSessionComplete.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);
	}

	GDataInterface->RemoveOnLaunchServer(Handle_LaunchServer);
}

UMatchmakingCallProxy* UMatchmakingCallProxy::Matchmaking(
	APlayerController* PlayerController,
	const FLevelInformation& LevelInformation,
	FMatchmakingHandle& OutHandle
)
{
	UMatchmakingCallProxy* Proxy = NewObject<UMatchmakingCallProxy>(PlayerController);
	Proxy->PlayerController = PlayerController;
	Proxy->T_LevelInformation = LevelInformation;
	Proxy->WaitingStartTime = 0;

	Proxy->Handle = FMatchmakingHandle(PlayerController->GetWorld());
	OutHandle = Proxy->Handle;
	
	return Proxy;
}

void UMatchmakingCallProxy::Activate()
{
	SessionInt = Online::GetSessionInterface();

	MatchmakingSessionName = FName(NAME_PartySession);

	if (PlayerController && SessionInt.IsValid())
	{
		SearchSettings = MakeShared<FOnlineSessionSearch>();
		SearchSettings->bIsLanQuery = false;
		SearchSettings->MaxSearchResults = 1;
		SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SETTING_MAPNAME, T_LevelInformation.Map.GetAssetName(), EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SETTING_GAMEMODE, T_LevelInformation.GameModeClassAlias, EOnlineComparisonOp::Equals);

		LastSearchingTime = GetWorld()->GetTimeSeconds();
		
		FindLobby();
	}
	else
	{
		Handle.Clear();
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::WaitingTick()
{
	FNamedOnlineSession* Session = SessionInt->GetNamedSession(MatchmakingSessionName);

	if (Session->NumOpenPublicConnections == 0 || WaitingStartTime > START_WAIT_TIME)
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle.Handle_Ticker);
		
		FLaunchServerParam Param(T_LevelInformation.GameModeClassAlias, T_LevelInformation.Map.GetLongPackageName());

		Handle.Handle_LaunchServer = GDataInterface->AddOnLaunchServer(
			FOnLaunchServer::FDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnLaunchServer));
		
		GDataInterface->LaunchDedicatedServer(Param);
	}
	
	WaitingStartTime += TICK_INTERVAL;
}

void UMatchmakingCallProxy::FindLobby()
{
	if (SearchingSeconds > SearchTimeout)
	{
		CreateLobby();
	}
	else
	{
		Handle.Handle_FindSessionsComplete = SessionInt->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnSearchCompleted));
		
		SessionInt->FindSessions(0, SearchSettings.ToSharedRef());
	}

	float TimeSeconds = GetWorld()->GetTimeSeconds();
	SearchingSeconds += LastSearchingTime - TimeSeconds;
	LastSearchingTime = TimeSeconds;
}

void UMatchmakingCallProxy::OnSearchCompleted(bool bSuccessful)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle.Handle_FindSessionsComplete);

	if (bSuccessful && SearchSettings->SearchResults.Num() > 0)
	{
		Handle.Handle_JoinSessionComplete = SessionInt->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnJoinSessionCompleted));

		SessionInt->JoinSession(0, MatchmakingSessionName, SearchSettings->SearchResults[0]);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(Handle.Handle_Search, this, &UMatchmakingCallProxy::FindLobby, 5.f);
	}
}

void UMatchmakingCallProxy::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type CompleteResult)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle.Handle_JoinSessionComplete);
	
	if (CompleteResult == EOnJoinSessionCompleteResult::Success)
	{
		Handle.Handle_CustomUpdateComplete = SessionInt->AddOnSessionCustomDataChangedDelegate_Handle(
			FOnSessionCustomDataChangedDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnServerReadyComplete));
		
		UE_LOG(LogOnline, Verbose, TEXT("OnJoinSessionCompleted"));
		DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		Handle.Clear();
		OnFailure.Broadcast();
		UE_LOG(LogOnline, Error, TEXT("JoinSessions Fail SessionName: %s, Result: %d"), *SessionName.ToString(), CompleteResult);
	}
}

void UMatchmakingCallProxy::CreateLobby()
{
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.NumPublicConnections = T_LevelInformation.MaxPlayers;

	SessionSettings.Set(SETTING_MAPNAME, T_LevelInformation.Map.GetAssetName(), EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_GAMEMODE, T_LevelInformation.GameModeClassAlias, EOnlineDataAdvertisementType::ViaOnlineService);

	Handle.Handle_CreateSessionComplete = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnCreateSessionCompleted));

	SessionInt->CreateSession(0, MatchmakingSessionName, SessionSettings);
}

void UMatchmakingCallProxy::OnCreateSessionCompleted(FName SessionName, bool bSuccessful)
{
	SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle.Handle_CreateSessionComplete);

	if (bSuccessful)
	{
		GetWorld()->GetTimerManager().SetTimer(Handle.Handle_Ticker, this, &UMatchmakingCallProxy::WaitingTick, TICK_INTERVAL, true);
		
		UE_LOG(LogOnline, Verbose, TEXT("OnCreateSessionCompleted"));
		DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		Handle.Clear();
		OnFailure.Broadcast();

		UE_LOG(LogOnline, Error, TEXT("Create Session Fail SessionName: %s"), *SessionName.ToString());
	}
}

void UMatchmakingCallProxy::OnLaunchServer(const FLaunchServerResult& Result, bool bSuccessfully)
{
	GDataInterface->RemoveOnLaunchServer(Handle.Handle_LaunchServer);
	
	if (bSuccessfully)
	{
		UE_LOG(LogOnline, Verbose, TEXT("SearchServer Success, Addr: %s"), *Result.ServerAddress);

		FNamedOnlineSession* Session = SessionInt->GetNamedSession(MatchmakingSessionName);

		FOnlineSessionSettings NewSettings = Session->SessionSettings;
		NewSettings.Set(GAME_SERVER_KEY, Result.ServerAddress, EOnlineDataAdvertisementType::ViaOnlineService);

		Handle.Handle_UpdateSessionComplete = SessionInt->AddOnUpdateSessionCompleteDelegate_Handle(
            FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnUpdateSessionComplete));
	
		SessionInt->UpdateSession(MatchmakingSessionName, NewSettings);
	}
	else
	{
		Handle.Clear();
		OnFailure.Broadcast();

		UE_LOG(LogOnline, Error, TEXT("SearchServer Failure"));
	}
}

void UMatchmakingCallProxy::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInt->ClearOnUpdateSessionCompleteDelegate_Handle(Handle.Handle_UpdateSessionComplete);

	if (bWasSuccessful)
	{
		FNamedOnlineSession* Session = SessionInt->GetNamedSession(SessionName);

		FString DedicatedServerAddress;
		if (Session->SessionSettings.Get(GAME_SERVER_KEY, DedicatedServerAddress))
		{
			OnSuccess.Broadcast();
			
			SessionInt->StartSession(MatchmakingSessionName);
			
			TravelToServer(DedicatedServerAddress);
		}
		else
		{
			UE_LOG(LogOnline, Verbose, TEXT("DedicatedServerAddress InValid"));

			OnFailure.Broadcast();
		}
	}
	else
	{
		OnFailure.Broadcast();

		UE_LOG(LogOnline, Error, TEXT("UpdateSession Failure"));
	}

	Handle.Clear();
}

void UMatchmakingCallProxy::OnServerReadyComplete(FName SessionName, const FOnlineSessionSettings& Settings)
{
	SessionInt->ClearOnSessionCustomDataChangedDelegate_Handle(Handle.Handle_CustomUpdateComplete);
	
	FString DedicatedServerAddress;
	if (Settings.Get(GAME_SERVER_KEY, DedicatedServerAddress))
	{
		OnSuccess.Broadcast();

		SessionInt->StartSession(SessionName);
		TravelToServer(DedicatedServerAddress);
	}
	else
	{
		OnFailure.Broadcast();

		UE_LOG(LogOnline, Verbose, TEXT("ServerAddress Invalid"));
	}

	Handle.Clear();
}

void UMatchmakingCallProxy::TravelToServer(const FString& ServerAddress) const
{

	int32 ClientPlayerID = GDataInterface->GetClientPlayerID();

	FString URL = FString::Printf(TEXT("%s?PlayerId=%d"), *ServerAddress, ClientPlayerID);

	IDreamLoadingScreenModule::Get().StartInGameLoadingScreen();
	PlayerController->ClientTravel(URL, ETravelType::TRAVEL_Absolute);
}
