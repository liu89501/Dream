// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchmakingCallProxy.h"
#include "OnlineSubsystem.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"
#include "TimerManager.h"
#include "DreamGameInstance.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

#define Begin_WaitTime 60
#define BeginWaitTickInterval 1
#define FindSessionRetryCount 30
#define MaxPlayerNum 4


IOnlineSessionPtr GetSessionInterface()
{
	IOnlineSessionPtr SessionInt;

	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
	{
		SessionInt = OSS->GetSessionInterface();
	}

	if (!SessionInt.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("GetSessionInterface Fail"));
	}
	return SessionInt;
}

UMatchmakingCallProxy* UMatchmakingCallProxy::Matchmaking(
	UObject* WorldContextObject, 
	APlayerController* PlayerController,
	const FName& InMapName,
	const FString& InGameMode,
	FMatchmakingHandle& OutHandle
)
{
	UMatchmakingCallProxy* Proxy = NewObject<UMatchmakingCallProxy>(WorldContextObject);
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->MatchMapName = InMapName;
	Proxy->GameModeAlias = InGameMode;
	Proxy->World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	FMatchmakingHandle Handle;
	Handle.MatchmakingCallProxy = Proxy;
	OutHandle = Handle;
	return Proxy;
}

void UMatchmakingCallProxy::Activate()
{
	SessionInt = GetSessionInterface();

	if (PlayerControllerWeakPtr.IsValid() && SessionInt.IsValid() && PlayerControllerWeakPtr->PlayerState)
	{
		PrevJoinedPlayerNum = 1;
		FindRetryCount = 0;
		BeginWaitTime = 0.f;

		SearchSettings = MakeShareable(new FOnlineSessionSearch);
		SearchSettings->bIsLanQuery = false;
		SearchSettings->MaxSearchResults = 1;
		SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SETTING_MAPNAME, MatchMapName.ToString(), EOnlineComparisonOp::Equals);

		Handle_FindSessionsComplete = SessionInt->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnSearchCompleted));

		Matching();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::ClearAllHandle()
{
	if (World.IsValid())
	{
		World->GetTimerManager().ClearTimer(Handle_WaitBegin);
		World->GetTimerManager().ClearTimer(Handle_RetryMatching);
	}

	if (SessionInt.IsValid())
	{
		SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);
		SessionInt->ClearOnJoinSessionCompleteDelegate_Handle(Handle_JoinSessionComplete);
		SessionInt->ClearOnStartSessionCompleteDelegate_Handle(Handle_StartSessionComplete);
		SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);
	}

	if (FPlayerDataInterface* PlayerDataInterface = FPlayerDataInterfaceModule::Get())
	{
		PlayerDataInterface->OnRunServerComplete.Remove(Handle_RunServer);
	}
}

void UMatchmakingCallProxy::Matching()
{
	if (FindRetryCount == FindSessionRetryCount)
	{
		SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);

		FOnlineSessionSettings SessionSettings;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowInvites = true;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.NumPublicConnections = MaxPlayerNum;

		SessionSettings.Set(SETTING_MAPNAME, MatchMapName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(SETTING_GAMEMODE, GameModeAlias, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		Handle_CreateSessionComplete = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnCreateSessionCompleted));

		SessionInt->CreateSession(0, GameSessionName, SessionSettings);
	}
	else
	{
		SessionInt->FindSessions(0, SearchSettings.ToSharedRef());
	}

	FindRetryCount++;
}

void UMatchmakingCallProxy::OnSearchCompleted(bool bSuccessful)
{
	if (bSuccessful)
	{
		if (SearchSettings->SearchResults.Num() == 0)
		{
			bSuccessful = false;
		}
		else
		{
#if !UE_BUILD_SHIPPING

			for (FOnlineSessionSearchResult Result : SearchSettings->SearchResults)
			{
				UE_LOG(LogOnline, Verbose, TEXT("Find Result OwningUserName: %s, SessionInfo: %s"),
					*Result.Session.OwningUserName, *Result.Session.SessionInfo->ToDebugString());
			}

#endif // UE_BUILD_SHIPPING
			SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);

			Handle_JoinSessionComplete = SessionInt->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnJoinSessionCompleted));

			SessionInt->JoinSession(0, GameSessionName, SearchSettings->SearchResults[0]);
		}
	}

	if (!bSuccessful)
	{
		World->GetTimerManager().SetTimer(Handle_RetryMatching, this, &UMatchmakingCallProxy::Matching, 0.5f);
	}
}

void UMatchmakingCallProxy::OnCreateSessionCompleted(FName SessionName, bool bSuccessful)
{
	SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);

	if (bSuccessful)
	{
		FNamedOnlineSession* Session = SessionInt->GetNamedSession(SessionName);
		if (Session)
		{
			UE_LOG(LogOnline, Verbose, TEXT("Create Session Completed OwningUserId: %s"),
				Session->OwningUserId.IsValid() ? *Session->OwningUserId->ToString() : TEXT("NONE"));

			FTimerDelegate WaitJoinDelegate;
			WaitJoinDelegate.BindUObject(this, &UMatchmakingCallProxy::WaitBeginTick, Session);
			World->GetTimerManager().SetTimer(Handle_WaitBegin, WaitJoinDelegate, BeginWaitTickInterval, true);
		}
		else
		{
			bSuccessful = false;
		}
	}

	if (!bSuccessful)
	{
		OnFailure.Broadcast();
		UE_LOG(LogOnline, Error, TEXT("Create Session Fail SessionName: %s"), *SessionName.ToString());
	}
}

void UMatchmakingCallProxy::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type CompleteResult)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_JoinSessionComplete);

	FNamedOnlineSession* Session = SessionInt->GetNamedSession(SessionName);
	
	if (CompleteResult == EOnJoinSessionCompleteResult::Success && Session)
	{
		UE_LOG(LogOnline, Verbose, TEXT("Join Session Completed OwningUserId: %s"),
			Session->OwningUserId.IsValid() ? *Session->OwningUserId->ToString() : TEXT("NONE"));

		// 如果会话在进行中 则直接连接
		if (Session->SessionState == EOnlineSessionState::InProgress)
		{
			JoinLobbyGameServer(SessionName);
		}
	}
	else
	{
		OnFailure.Broadcast();
		UE_LOG(LogOnline, Error, TEXT("JoinSessions Fail SessionName: %s, Result: %d"), *SessionName.ToString(), CompleteResult);
	}
}

void UMatchmakingCallProxy::WaitBeginTick(FNamedOnlineSession* Session)
{
	if (Session->SessionSettings.NumPublicConnections - Session->NumOpenPublicConnections != PrevJoinedPlayerNum)
	{
		OnJoinedPlayerChanged.Broadcast();
	}

	if (Session->NumOpenPublicConnections == 0 || BeginWaitTime >= Begin_WaitTime)
	{
		if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
		{
			UDreamGameInstance* GI = World->GetGameInstance<UDreamGameInstance>();
			Handle_RunServer = PDS->OnRunServerComplete.AddUObject(this, &UMatchmakingCallProxy::OnCreateServerComplete);
			PDS->RunServer(FRunServerParameter(GI->GetMapFullName(MatchMapName), GameModeAlias));
		}
		else
		{
			UE_LOG(LogOnline, Error, TEXT("LaunchDedicatedServer Fail FPlayerDataInterfaceModule Invalid"));
			OnFailure.Broadcast();
		}

		World->GetTimerManager().ClearTimer(Handle_WaitBegin);
	}

	BeginWaitTime += BeginWaitTickInterval;
}

void UMatchmakingCallProxy::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInt->ClearOnStartSessionCompleteDelegate_Handle(Handle_StartSessionComplete);

	if (bWasSuccessful)
	{
		JoinLobbyGameServer(SessionName);
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("StartSession Fail SessionName: %s"), *SessionName.ToString());
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::JoinLobbyGameServer(const FName& SessionName)
{
	FNamedOnlineSession* Session = SessionInt->GetNamedSession(SessionName);
	if (Session)
	{
		uint32 ServerIP;
		uint16 Port;
		CSteamID ServerSteamID;

		const CSteamID SteamLobbyID(FCString::Strtoui64(*Session->SessionInfo->GetSessionId().ToString(), nullptr, 10));
		if (SteamMatchmaking()->GetLobbyGameServer(SteamLobbyID, &ServerIP, &Port, &ServerSteamID))
		{
			FString ServerAddr;
			ServerAddr.AppendInt(((ServerIP >> 24) & 255));
			ServerAddr.AppendChar(TEXT('.'));
			ServerAddr.AppendInt(((ServerIP >> 16) & 255));
			ServerAddr.AppendChar(TEXT('.'));
			ServerAddr.AppendInt(((ServerIP >> 8) & 255));
			ServerAddr.AppendChar(TEXT('.'));
			ServerAddr.AppendInt((ServerIP & 255));
			ServerAddr.AppendChar(TEXT(':'));
			ServerAddr.AppendInt(Port);

			UE_LOG(LogOnline, Verbose, TEXT("GetLobbyGameServer ServerAddr: %s"), *ServerAddr);
			OnSuccess.Broadcast();

			PlayerControllerWeakPtr->ClientTravel(ServerAddr, ETravelType::TRAVEL_Absolute);

		}
		else
		{
			UE_LOG(LogOnline, Error, TEXT("GetLobbyGameServer Fail SessionName: %s"), *SessionName.ToString());
			OnFailure.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("JoinLobbyGameServer Session Invalid: %s"), *SessionName.ToString());
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::OnCreateServerComplete(const FString& ServerAddress, const FString& ErrorMessage)
{
	FPlayerDataInterfaceModule::Get()->OnRunServerComplete.Remove(Handle_RunServer);
	
	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG(LogOnline, Error, TEXT("CreateServer Fail"));
		OnFailure.Broadcast();
		return;
	}

	FNamedOnlineSession* Session = SessionInt->GetNamedSession(GameSessionName);
	if (Session)
	{
		TArray<FString> AddrArray;
		ServerAddress.ParseIntoArray(AddrArray, TEXT(":"));

		TArray<FString> IPSegment;
		AddrArray[0].ParseIntoArray(IPSegment, TEXT("."));

		uint32 ServerIP = (FCString::Atoi(*IPSegment[3]) & 255);
		ServerIP |= (FCString::Atoi(*IPSegment[2]) & 255) << 8;
		ServerIP |= (FCString::Atoi(*IPSegment[1]) & 255) << 16;
		ServerIP |= (FCString::Atoi(*IPSegment[0]) & 255) << 24;

		const uint16 Port = FCString::Atoi(*AddrArray[1]);

		Handle_StartSessionComplete = SessionInt->AddOnStartSessionCompleteDelegate_Handle(
			FOnStartSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnStartSessionComplete));

		const CSteamID SteamLobbyId(FCString::Strtoui64(*Session->GetSessionIdStr(), nullptr, 10));
		UE_LOG(LogOnline, Verbose, TEXT("CreateServerComplete SteamLobbyId: %llu"), SteamLobbyId.ConvertToUint64());

		SteamMatchmaking()->SetLobbyGameServer(SteamLobbyId, ServerIP, Port, CSteamID());
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("SetLobbyGameServer Fail Session Invalid"));
		OnFailure.Broadcast();
	}
}