// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameSession.h"
#include "OnlineSubsystem.h"
#include "DreamGameMode.h"
#include "Online.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "Kismet/GameplayStatics.h"

void ADreamGameSession::RegisterServer()
{
	// 注册会话到steam
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterfaceChecked();

	FOnlineSessionSettings Settings;

	FString MapName = GetWorld()->GetMapName();
		
	FString GameModeName;
	FParse::Value(FCommandLine::Get(), TEXT("Game="), GameModeName);
		
	Settings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Settings.Set(SETTING_GAMEMODE, GameModeName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			
	Settings.bUsesPresence = false;
	Settings.bIsLANMatch = false;
	Settings.bAllowInvites = true;
	Settings.bIsDedicated = true;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.NumPublicConnections = MaxPlayers;

	Handle_CreateSession = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &ADreamGameSession::OnCreateSession));
		
	SessionInterface->CreateSession(0, SessionName, Settings);
}

void ADreamGameSession::OnCreateSession(FName InSessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterfaceChecked();
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSession);
	
	FPlayerDataInterface* DataInterface = FPDIStatic::Get();

	FOnCompleted::FDelegate Delegate;
	Delegate.BindUObject(this, &ADreamGameSession::OnLoginCallback);
	Handle_Login = DataInterface->AddOnLogin(Delegate);
	DataInterface->Login();
	
	if (bWasSuccessful)
	{
		UE_LOG(LogDream, Log, TEXT("Create Session Success: %s"), *InSessionName.ToString());
	}
	else
	{
		UE_LOG(LogDream, Error, TEXT("Create Session Failure: %s"), *InSessionName.ToString());
	}
}

void ADreamGameSession::OnLoginCallback(bool bSuccessfully)
{
	FPDIStatic::Get()->RemoveOnLogin(Handle_Login);
	
	if (!bSuccessfully)
	{
		DREAM_NLOG(Error, TEXT("连接服务器失败"));
		FPlatformMisc::RequestExit(false);
	}
	
	FString ServerID;
	if (FParse::Value(FCommandLine::Get(), TEXT("ServerId="), ServerID))
	{
		FParse::Value(FCommandLine::Get(), TEXT("MaxPlayers="), MaxPlayers);
		
		int32 Port = GetNetDriver()->GetLocalAddr()->GetPort();
		
		FLaunchNotifyParam Param;
		Param.ServerId = ServerID;
		Param.ListenPort = Port;
		Param.MaxPlayers = MaxPlayers;

		FParse::Value(FCommandLine::Get(), TEXT("PlayerId="), Param.PlayerId);
		
		FPDIStatic::Get()->NotifyBackendServer(Param);
	}
	
	/*
	else
	{
		FDedicatedServerInformation Information;
		Information.Port = Port;
		Information.MapName = GetWorld()->GetMapName();
		Information.MaxPlayers = MaxPlayers;
		Information.GameModeName = GetWorld()->GetAuthGameMode()->GetName();
	
		FPDIStatic::Get()->RegisterServer(Information);

		UE_LOG_ONLINE(Log, TEXT("MapName: %s"), *GetWorld()->GetMapName());
		UE_LOG_ONLINE(Log, TEXT("GameModeName: %s"), *Information.GameModeName);
	}*/
}
