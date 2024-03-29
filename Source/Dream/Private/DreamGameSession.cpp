// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameSession.h"
#include "OnlineSubsystem.h"
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
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &ADreamGameSession::OnCreatedSession));
		
	SessionInterface->CreateSession(0, SessionName, Settings);
}

void ADreamGameSession::OnCreatedSession(FName InSessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterfaceChecked();
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(Handle_CreateSession);
	
	FOnCompleted::FDelegate Delegate;
	Delegate.BindUObject(this, &ADreamGameSession::OnLoginCallback);
	Handle_Login = GDataInterface->AddOnLogin(Delegate);
	GDataInterface->Login();
	
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
	GDataInterface->RemoveOnLogin(Handle_Login);
	
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

		FParse::Value(FCommandLine::Get(), TEXT("OwningPlayerId="), Param.PlayerId);
		
		GDataInterface->NotifyServerLaunched(Param);
	}
}
