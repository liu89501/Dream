// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchmakingCallProxy.h"
#include "OnlineSubsystem.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "TimerManager.h"
#include "DreamGameInstance.h"
#include "DreamGameMode.h"
#include "GameMapsSettings.h"
#include "OnlineSubsystemSessionSettings.h"
#include "OnlineSubsystemUtils.h"

#define SESSION_WAIT_TIME 30
#define SESSION_WAIT_TICK_INTERVAL 1
#define FIND_SESSION_RETRY_COUNT 10
#define FIND_SESSION_RETRY_INTERVAL 1


UMatchmakingCallProxy* UMatchmakingCallProxy::Matchmaking(
	APlayerController* PlayerController,
	const FLevelInformation& LevelInformation,
	FMatchmakingHandle& OutHandle
)
{
	UMatchmakingCallProxy* Proxy = NewObject<UMatchmakingCallProxy>(PlayerController);
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->T_LevelInformation = LevelInformation;

	FMatchmakingHandle Handle;
	Handle.MatchmakingCallProxy = Proxy;
	OutHandle = Handle;
	return Proxy;
}

void UMatchmakingCallProxy::Activate()
{
	SessionInt = Online::GetSessionInterface();
	World = PlayerControllerWeakPtr->GetWorld();

	MatchmakingSessionName = FName(NAME_PartySession);

	if (UClass* GameModeClass = LoadClass<ADreamGameMode>(nullptr, *UGameMapsSettings::GetGameModeForName(T_LevelInformation.GameNodeClassAlias)))
	{
		MaxPlayers = GameModeClass->GetDefaultObject<ADreamGameMode>()->GetGameModeMaxPlayers();
	}
	else
	{
		MaxPlayers = ADreamGameMode::DEFAULT_MAX_PLAYERS;
	}

	if (PlayerControllerWeakPtr.IsValid() && SessionInt.IsValid() && World.IsValid())
	{
		FindRetryCount = 0;
		WaitStartTime = 0;

		SearchSettings = MakeShareable(new FOnlineSessionSearch);
		SearchSettings->bIsLanQuery = false;
		SearchSettings->MaxSearchResults = 1;
		SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SETTING_MAPNAME, T_LevelInformation.Map.GetAssetName(), EOnlineComparisonOp::Equals);

		FindLobby();
	}
	else
	{
		ClearAllHandle();
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::ClearAllHandle()
{
	World->GetTimerManager().ClearTimer(Handle_RetryMatching);
	World->GetTimerManager().ClearTimer(Handle_SessionTIck);

	SessionInt->ClearOnUpdateSessionCompleteDelegate_Handle(Handle_UpdateSessionComplete);
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);
	SessionInt->ClearOnJoinSessionCompleteDelegate_Handle(Handle_JoinSessionComplete);
	SessionInt->ClearOnStartSessionCompleteDelegate_Handle(Handle_StartSessionComplete);
	SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);
}

void UMatchmakingCallProxy::Tick()
{
	FNamedOnlineSession* Session = SessionInt->GetNamedSession(MatchmakingSessionName);

	switch (State)
	{
		case EMatchmakingState::WAITING_TEAM:
			{
				if (Session->NumOpenPublicConnections == 0 || WaitStartTime > SESSION_WAIT_TIME)
				{
					State = EMatchmakingState::CREATING_SERVER;

					FRunServerComplete Delegate;
					Delegate.BindUObject(this, &UMatchmakingCallProxy::OnCreateServerComplete);
					FRunServerParameter Parameter(T_LevelInformation.Map.GetAssetName(),
                        T_LevelInformation.GameNodeClassAlias, T_LevelInformation.Map.GetLongPackageName());
			
					FPlayerDataInterfaceStatic::Get()->RunNewDedicatedServer(Parameter, Delegate);
				}
				WaitStartTime += SESSION_WAIT_TICK_INTERVAL;
			}
		break;

		case EMatchmakingState::WAITING_SERVER:
			{
				FGetServerComplete Delegate;
				Delegate.BindUObject(this, &UMatchmakingCallProxy::OnServerReadyComplete);
				FPlayerDataInterfaceStatic::Get()->GetAvailableDedicatedServer(CreatedServerID, Delegate);
			}
		break;

		case EMatchmakingState::ATTEMPT_CONNECT_SERVER:
			{
				FString DedicatedServerAddress;
				if (Session->SessionSettings.Get(SETTING_GAME_SESSION_URI, DedicatedServerAddress))
				{
					SessionInt->StartSession(MatchmakingSessionName);
					PlayerControllerWeakPtr->ClientTravel(DedicatedServerAddress, ETravelType::TRAVEL_Absolute);
					OnSuccess.Broadcast();
					ClearAllHandle();
				}
			}
		break;
		
		default:
			break;
	}
}

void UMatchmakingCallProxy::FindLobby()
{
	if (FindRetryCount > FIND_SESSION_RETRY_COUNT)
	{
		CreateLobby();
	}
	else
	{
		Handle_FindSessionsComplete = SessionInt->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnSearchCompleted));
		
		SessionInt->FindSessions(0, SearchSettings.ToSharedRef());
	}

	FindRetryCount += FIND_SESSION_RETRY_INTERVAL;
}

void UMatchmakingCallProxy::OnSearchCompleted(bool bSuccessful)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);

	if (bSuccessful && SearchSettings->SearchResults.Num() > 0)
	{
		// 打印找到的会话信息
		UE_LOG(LogOnline, Verbose, TEXT("OnSearchCompleted"));

		Handle_JoinSessionComplete = SessionInt->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnJoinSessionCompleted));

		SessionInt->JoinSession(0, MatchmakingSessionName, SearchSettings->SearchResults[0]);
	}
	else
	{
		UE_LOG(LogOnline, Verbose, TEXT("OnSearchCompleted Retry - %d"), FindRetryCount);
		World->GetTimerManager().SetTimer(Handle_RetryMatching, this, &UMatchmakingCallProxy::FindLobby, FIND_SESSION_RETRY_INTERVAL);
	}
}

void UMatchmakingCallProxy::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type CompleteResult)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_JoinSessionComplete);
	
	if (CompleteResult == EOnJoinSessionCompleteResult::Success)
	{
		State = EMatchmakingState::ATTEMPT_CONNECT_SERVER;
		World->GetTimerManager().SetTimer(Handle_SessionTIck, this, &UMatchmakingCallProxy::Tick, SESSION_WAIT_TICK_INTERVAL, true);
		
		UE_LOG(LogOnline, Verbose, TEXT("OnJoinSessionCompleted"));
		DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		ClearAllHandle();
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
	SessionSettings.NumPublicConnections = MaxPlayers;

	SessionSettings.Set(SETTING_MAPNAME, T_LevelInformation.Map.GetAssetName(), EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_GAMEMODE, T_LevelInformation.GameNodeClassAlias, EOnlineDataAdvertisementType::ViaOnlineService);

	Handle_CreateSessionComplete = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnCreateSessionCompleted));

	SessionInt->CreateSession(0, MatchmakingSessionName, SessionSettings);
}

void UMatchmakingCallProxy::OnCreateSessionCompleted(FName SessionName, bool bSuccessful)
{
	SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);

	if (bSuccessful)
	{
		State = EMatchmakingState::WAITING_TEAM;
		World->GetTimerManager().SetTimer(Handle_SessionTIck, this, &UMatchmakingCallProxy::Tick, SESSION_WAIT_TICK_INTERVAL, true);
		
		UE_LOG(LogOnline, Verbose, TEXT("OnCreateSessionCompleted"));
		DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("Create Session Fail SessionName: %s"), *SessionName.ToString());
		ClearAllHandle();
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::OnCreateServerComplete(const FString& ServerID, const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		CreatedServerID = ServerID;
		State = EMatchmakingState::WAITING_SERVER;

		UE_LOG(LogOnline, Verbose, TEXT("OnCreateServerComplete, ServerID: %s"), *CreatedServerID);
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("CreateServer Failure: %s"), *ErrorMessage);
		OnFailure.Broadcast();
		ClearAllHandle();
	}
}

void UMatchmakingCallProxy::OnServerReadyComplete(const FFindServerResult& Result, const FString& ErrorMessage)
{
	FNamedOnlineSession* Session = SessionInt->GetNamedSession(MatchmakingSessionName);

	bool bCheck = Session != nullptr && ErrorMessage.IsEmpty();
	
	if (bCheck)
	{
		if (Result.ServerState == EServerState::IN_PROGRESS)
		{
			State = EMatchmakingState::ATTEMPT_CONNECT_SERVER;
			Handle_UpdateSessionComplete = SessionInt->AddOnUpdateSessionCompleteDelegate_Handle(
            FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnUpdateSessionComplete));

			FOnlineSessionSettings NewSettings = Session->SessionSettings;
			NewSettings.Set(SETTING_GAME_SESSION_URI, Result.ServerAddress, EOnlineDataAdvertisementType::ViaOnlineService);
			SessionInt->UpdateSession(MatchmakingSessionName, NewSettings);
		}
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("ServerReady Failure: %s"), *ErrorMessage);
		OnFailure.Broadcast();
		ClearAllHandle();
	}
}

void UMatchmakingCallProxy::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInt->ClearOnUpdateSessionCompleteDelegate_Handle(Handle_UpdateSessionComplete);

	if (bWasSuccessful)
	{
		UE_LOG(LogOnline, Verbose, TEXT("UpdateSessionComplete"));
		//DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("UpdateSession Failure"));
		OnFailure.Broadcast();
		ClearAllHandle();
	}
}
