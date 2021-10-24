// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchmakingCallProxy.h"
#include "OnlineSubsystem.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "TimerManager.h"
#include "DreamGameInstance.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSubsystemUtils.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

#define Begin_WaitTime 60
#define FindSessionRetryCount 30
#define MaxPlayerNum 4

#define Dedicated_Server_Address TEXT("Dedicated_Server_Address")


UMatchmakingCallProxy* UMatchmakingCallProxy::Matchmaking(
	APlayerController* PlayerController,
	const FName& InMapName,
	const FString& InGameMode,
	FMatchmakingHandle& OutHandle
)
{
	UMatchmakingCallProxy* Proxy = NewObject<UMatchmakingCallProxy>(PlayerController);
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->MatchMapName = InMapName;
	Proxy->GameModeAlias = InGameMode;

	FMatchmakingHandle Handle;
	Handle.MatchmakingCallProxy = Proxy;
	OutHandle = Handle;
	return Proxy;
}

void UMatchmakingCallProxy::Activate()
{
	SessionInt = Online::GetSessionInterface();
	World = PlayerControllerWeakPtr->GetWorld();

	if (PlayerControllerWeakPtr.IsValid() && SessionInt.IsValid() && World.IsValid())
	{
		PrevJoinedPlayerNum = 1;
		FindRetryCount = 0;
		//BeginWaitTime = 0.f;

		SearchSettings = MakeShareable(new FOnlineSessionSearch);
		SearchSettings->bIsLanQuery = false;
		SearchSettings->MaxSearchResults = 1;
		SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SETTING_MAPNAME, MatchMapName.ToString(), EOnlineComparisonOp::Equals);

		/*Handle_UpdateSessionsComplete = SessionInt->AddOnUpdateSessionCompleteDelegate_Handle(
			FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnUpdateSessionComplete));*/

		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &UMatchmakingCallProxy::OnSessionTick, 1.f);
		World->GetTimerManager().SetTimer(Handle_SessionTIck, Delegate, 1.f, true);

		FindLobby();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

void UMatchmakingCallProxy::ClearAllHandle()
{
	World->GetTimerManager().ClearTimer(Handle_RetryMatching);
	World->GetTimerManager().ClearTimer(Handle_SessionTIck);

	SessionInt->ClearOnUpdateSessionCompleteDelegate_Handle(Handle_UpdateSessionsComplete);
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);
	SessionInt->ClearOnJoinSessionCompleteDelegate_Handle(Handle_JoinSessionComplete);
	SessionInt->ClearOnStartSessionCompleteDelegate_Handle(Handle_StartSessionComplete);
	SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);
}

void UMatchmakingCallProxy::FindLobby()
{
	if (FindRetryCount == FindSessionRetryCount)
	{
		CreateLobby();
	}
	else
	{
		Handle_FindSessionsComplete = SessionInt->AddOnFindSessionsCompleteDelegate_Handle(
                FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnSearchCompleted));
		
		SessionInt->FindSessions(0, SearchSettings.ToSharedRef());
	}

	FindRetryCount++;
}

void UMatchmakingCallProxy::OnSearchCompleted(bool bSuccessful)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_FindSessionsComplete);
	
	if (bSuccessful && SearchSettings->SearchResults.Num() > 0)
	{
		// 打印找到的会话信息
		for (FOnlineSessionSearchResult Result : SearchSettings->SearchResults)
		{
			DumpSession(&Result.Session);
		}

		Handle_JoinSessionComplete = SessionInt->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnJoinSessionCompleted));

		SessionInt->JoinSession(0, GameSessionName, SearchSettings->SearchResults[0]);
	}
	else
	{
		World->GetTimerManager().SetTimer(Handle_RetryMatching, this, &UMatchmakingCallProxy::FindLobby, 1.f);
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
	SessionSettings.NumPublicConnections = MaxPlayerNum;

	SessionSettings.Set(SETTING_MAPNAME, MatchMapName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(SETTING_GAMEMODE, GameModeAlias, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	Handle_CreateSessionComplete = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMatchmakingCallProxy::OnCreateSessionCompleted));

	SessionInt->CreateSession(0, GameSessionName, SessionSettings);
}

void UMatchmakingCallProxy::OnCreateSessionCompleted(FName SessionName, bool bSuccessful)
{
	SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSessionComplete);

	if (bSuccessful)
	{
		UE_LOG(LogOnline, Verbose, TEXT("OnCreateSessionCompleted"));
		DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("Create Session Fail SessionName: %s"), *SessionName.ToString());
	}
}

void UMatchmakingCallProxy::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type CompleteResult)
{
	SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(Handle_JoinSessionComplete);
	
	if (CompleteResult == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogOnline, Verbose, TEXT("OnJoinSessionCompleted"));
		DumpNamedSession(SessionInt->GetNamedSession(SessionName));
	}
	else
	{
		OnFailure.Broadcast();
		UE_LOG(LogOnline, Error, TEXT("JoinSessions Fail SessionName: %s, Result: %d"), *SessionName.ToString(), CompleteResult);
	}
}

void UMatchmakingCallProxy::OnSessionTick(float DeltaTime)
{
	FNamedOnlineSession* Session = SessionInt->GetNamedSession(GameSessionName);

	if (!Session)
	{
		return;
	}

	if (Session->SessionState == EOnlineSessionState::Pending)
	{
		OnJoinChanged.Broadcast(Session->SessionSettings.NumPublicConnections - Session->NumOpenPublicConnections);

		if ((Session->NumOpenPublicConnections == 0 || WaitStartTime > Begin_WaitTime) && !bCreatingServer)
		{
			bCreatingServer = true;
			
			IOnlineIdentityPtr IdentityPtr = Online::GetIdentityInterfaceChecked();
			if (Session->OwningUserId == IdentityPtr->GetUniquePlayerId(0))
			{
				FGetServerComplete Delegate;
				Delegate.BindUObject(this, &UMatchmakingCallProxy::OnCreateServerComplete);

				UDreamGameInstance* GI = World->GetGameInstance<UDreamGameInstance>();
				FPlayerDataInterfaceStatic::Get()->RunServer(FRunServerParameter(GI->GetMapFullName(MatchMapName), GameModeAlias), Delegate);
			}
		}
	
		WaitStartTime += DeltaTime;
	}
	else if (Session->SessionState == EOnlineSessionState::InProgress)
	{
		FString ConnectURL;
		if (SessionInt->GetResolvedConnectString(GameSessionName, ConnectURL))
		{
			PlayerControllerWeakPtr->ClientTravel(ConnectURL, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UMatchmakingCallProxy::OnCreateServerComplete(const FString& ServerAddress, const FString& ErrorMessage)
{
	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG(LogOnline, Error, TEXT("CreateServer Fail"));
		OnFailure.Broadcast();
		return;
	}

	FNamedOnlineSession* Session = SessionInt->GetNamedSession(GameSessionName);

	FOnlineSessionSettings NewSettings = Session->SessionSettings;
	NewSettings.Set(Dedicated_Server_Address, ServerAddress);
	SessionInt->UpdateSession(GameSessionName, NewSettings);

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

		//SessionInt->CreateSession()
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
		/*uint32 ServerIP;
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
		}*/

		FString ServerAddr;
		SessionInt->GetResolvedConnectString(SessionName, ServerAddr);
		UE_LOG(LogOnline, Verbose, TEXT("GetLobbyGameServer ServerAddr: %s"), *ServerAddr);
		PlayerControllerWeakPtr->ClientTravel(ServerAddr, ETravelType::TRAVEL_Absolute);

		OnSuccess.Broadcast();
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("JoinLobbyGameServer Session Invalid: %s"), *SessionName.ToString());
		OnFailure.Broadcast();
	}
}

