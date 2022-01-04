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
	SessionSettings.NumPublicConnections = T_LevelInformation.GetGameModeMaxPlayers();
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
	
	FSearchServerParam Param;
	Param.MapName = T_LevelInformation.Map.GetAssetName();
	Param.MapAssetPath = T_LevelInformation.Map.GetLongPackageName();
	Param.ModeName = T_LevelInformation.GameModeClassAlias;

	FPlayerDataInterface* DataInterface = FPDIStatic::Get();
	
	FOnSearchServer::FDelegate OnSearchServer;
	OnSearchServer.BindUObject(this, &UConnectToServer::OnPDISearchServer);
	
	Handle_SearchServer = DataInterface->AddOnSearchServer(OnSearchServer);
	DataInterface->SearchDedicatedServer(Param);
}

void UConnectToServer::OnPDISearchServer(const FSearchServerResult& Result, bool bSuccessfully)
{
	FPDIStatic::Get()->RemoveOnSearchServer(Handle_SearchServer);
	
	if (bSuccessfully)
	{
		OnSuccess.Broadcast();

		IDreamLoadingScreenModule::Get().StartInGameLoadingScreen();
		
		int32 ClientPlayerID = FPDIStatic::Get()->GetClientPlayerID();
		GEngine->SetClientTravel(GetWorld(), *Result.GetConnectURL(ClientPlayerID), ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogDream, Error, TEXT("SearchServer Fail"));
		OnFailure.Broadcast();
	}
}
