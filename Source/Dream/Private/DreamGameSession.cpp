// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameSession.h"
#include "OnlineSubsystem.h"
#include "DreamGameMode.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"
#include "Interfaces/OnlineSessionInterface.h"

void ADreamGameSession::RegisterServer()
{
    if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
    {
        int32 Port = GetNetDriver()->GetLocalAddr()->GetPort();
        PDS->RegisterServer(Port, MaxPlayers, GetWorld()->GetMapName());
    }

	/*if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInt = OSS->GetSessionInterface();

		if (SessionInt.IsValid())
		{
            HostSettings.Set(SETTING_MAPNAME, GetWorld()->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
            HostSettings.bUsesPresence = false;
            HostSettings.bIsLANMatch = false;
            HostSettings.bAllowInvites = true;
            HostSettings.bIsDedicated = true;
            HostSettings.bShouldAdvertise = true;
            HostSettings.bAllowJoinInProgress = true;
            HostSettings.NumPublicConnections = MaxPlayers;

            FName SessionName(GameSessionName);

            UE_LOG_ONLINE(Log, TEXT("GameSessionName: %s"), *SessionName.ToString());

            SessionInt->CreateSession(0, SessionName, HostSettings);
		}
	}*/
}
