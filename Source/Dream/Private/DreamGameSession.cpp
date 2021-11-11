// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameSession.h"
#include "OnlineSubsystem.h"
#include "DreamGameMode.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "Kismet/GameplayStatics.h"

void ADreamGameSession::RegisterServer()
{
	int32 Port = GetNetDriver()->GetLocalAddr()->GetPort();

	FString ServerID;
	if (FParse::Value(FCommandLine::Get(), TEXT("ServerId="), ServerID))
	{
		FPDIStatic::Get()->NotifyBackendServer(FLaunchNotifyParam(ServerID, Port));
	}
	else
	{
		FString GameModeName;
		FParse::Value(FCommandLine::Get(), TEXT("Game="), GameModeName);

		ADreamGameMode* GameMode = Cast<ADreamGameMode>(GetWorld()->GetAuthGameMode());
		MaxPlayers = GameMode->GetGameModeMaxPlayers();

		FDedicatedServerInformation Information;
		Information.Port = Port;
		Information.MapName = GetWorld()->GetMapName();
		Information.MaxPlayers = MaxPlayers;
		Information.GameModeName = GameModeName;
	
		FPDIStatic::Get()->RegisterServer(Information);

		UE_LOG_ONLINE(Log, TEXT("MapName: %s"), *GetWorld()->GetMapName());
		UE_LOG_ONLINE(Log, TEXT("GameModeName: %s"), *GameModeName);
	}

	/*IOnlineSessionPtr SessionInt = Online::GetSessionInterface();
	if (SessionInt.IsValid())
	{
		FString GameModeName;
		FParse::Value(FCommandLine::Get(), TEXT("Game="), GameModeName);
		
		FOnlineSessionSettings SessionSettings;
		SessionSettings.Set(SETTING_MAPNAME, GetWorld()->GetMapName());
		SessionSettings.Set(SETTING_GAMEMODE, GameModeName);
		SessionSettings.bUsesPresence = false;
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bAllowInvites = true;
		SessionSettings.bIsDedicated = true;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.NumPublicConnections = MaxPlayers;

		UE_LOG_ONLINE(Log, TEXT("RegisterServer: %s"), *SessionName.ToString());
		UE_LOG_ONLINE(Log, TEXT("GameModeName: %s"), *GameModeName);

		SessionInt->CreateSession(0, GameSessionName, SessionSettings);
	}*/
}
