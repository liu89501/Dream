// Fill out your copyright notice in the Description page of Project Settings.

#include "ConnectToServer.h"
#include "DreamLoadingScreen.h"
#include "Online.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"


UConnectToServer* UConnectToServer::Connect(UObject* WorldContextObject, const FLevelInformation& LevelInformation)
{
	UConnectToServer* ConnectToServer = NewObject<UConnectToServer>(WorldContextObject);
	ConnectToServer->T_LevelInformation = LevelInformation;
	return ConnectToServer;
}

void UConnectToServer::Activate()
{
	SessionInterface = Online::GetSessionInterface();

	FOnlineSessionSettings SessionSettings;
	SessionSettings.Set(SETTING_MAPNAME, T_LevelInformation.Map.GetAssetName());
	SessionSettings.Set(SETTING_GAMEMODE, T_LevelInformation.GameModeClassAlias);
	SessionSettings.NumPublicConnections = T_LevelInformation.MaxPlayers;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinViaPresence = true;

	Handle_CreateSession = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UConnectToServer::OnCreateSession));
	
	SessionInterface->CreateSession(0, NAME_PartySession, SessionSettings);
}

void UConnectToServer::OnCreateSession(FName SessionName, bool bSuccessfully)
{
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSession);
	
	if (!bSuccessfully)
	{
		OnFailure.Broadcast();
		UE_LOG(LogDream, Error, TEXT("CreateSession Fail %s"), *SessionName.ToString());
		return;
	}
	
	FLaunchServerParam Param;
	Param.MapAssetPath = T_LevelInformation.Map.GetLongPackageName();
	Param.ModeName = T_LevelInformation.GameModeClassAlias;

	FOnLaunchServer::FDelegate OnSearchServer;
	OnSearchServer.BindUObject(this, &UConnectToServer::OnLaunchServer);
	
	Handle_SearchServer = GDataInterface->AddOnLaunchServer(OnSearchServer);
	GDataInterface->LaunchDedicatedServer(Param);
}

void UConnectToServer::OnLaunchServer(const FLaunchServerResult& Result, bool bSuccessfully)
{
	GDataInterface->RemoveOnLaunchServer(Handle_SearchServer);
	
	if (bSuccessfully)
	{
		OnSuccess.Broadcast();

		IDreamLoadingScreenModule::Get().StartInGameLoadingScreen();
		
		int32 ClientPlayerID = GDataInterface->GetClientPlayerID();
		GEngine->SetClientTravel(GetWorld(), *Result.GetConnectURL(ClientPlayerID), ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogDream, Error, TEXT("SearchServer Fail"));
		OnFailure.Broadcast();
	}
}
