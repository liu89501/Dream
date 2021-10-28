// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameSession.h"
#include "OnlineSubsystem.h"
#include "DreamGameMode.h"
#include "OnlineSubsystemUtils.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "Interfaces/OnlineSessionInterface.h"

void ADreamGameSession::RegisterServer()
{
	FString GameModeName;
	FParse::Value(FCommandLine::Get(), TEXT("Game="), GameModeName);

	ADreamGameMode* GameMode = Cast<ADreamGameMode>(GetWorld()->GetAuthGameMode());
	MaxPlayers = GameMode->GetGameModeMaxPlayers();

	FDedicatedServerInformation Information;
	Information.Port = GetNetDriver()->GetLocalAddr()->GetPort();
	Information.MapName = GetWorld()->GetMapName();
	Information.MaxPlayers = MaxPlayers;
	Information.GameModeName = GameModeName;
	
	FPlayerDataInterfaceStatic::Get()->RegisterServer(Information);

	UE_LOG_ONLINE(Log, TEXT("MapName: %s"), *GetWorld()->GetMapName());
	UE_LOG_ONLINE(Log, TEXT("GameModeName: %s"), *GameModeName);

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
