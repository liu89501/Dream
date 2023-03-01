// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DMMessageGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DreamGameSession.h"
#include "OnlineBeaconHost.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "PDI/PlayerDataInterface.h"


ADMMessageGameMode::ADMMessageGameMode()
{
	PlayerStateClass = APlayerState::StaticClass();
	GameSessionClass = ADreamGameSession::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}

void ADMMessageGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	OnlineBeacon_Listener = GetWorld()->SpawnActor<AOnlineBeaconHost>();
	if (OnlineBeacon_Listener->InitHost())
	{
		UE_LOG(LogDream, Log, TEXT("============================"));
		UE_LOG(LogDream, Log, TEXT("PlayerBeaconHost initialized"));
		UE_LOG(LogDream, Log, TEXT("============================"));
	}
	else
	{
		UE_LOG(LogDream, Error, TEXT("OnlineBeacon_Listener initialization failed"));
	}
}

void ADMMessageGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (OnlineBeacon_Listener)
	{
		OnlineBeacon_Listener->DestroyBeacon();
		OnlineBeacon_Listener = nullptr;
	}
}

void ADMMessageGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
}

FString ADMMessageGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Error = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	
	if (Error.IsEmpty())
	{
		if (NewPlayerController->PlayerState)
		{
			int32 PlayerId = UGameplayStatics::GetIntOption(Options, TEXT("PlayerId"), 0);
			NewPlayerController->PlayerState->SetPlayerId(PlayerId);	
		}
	}

	return Error;
}
