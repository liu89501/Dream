// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DreamGameMode.h"
#include "DMGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DreamGameSession.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


ADreamGameMode::ADreamGameMode()
	: DefaultPlayerRespawnDelay(5.f)
{
	PlayerStateClass = APlayerState::StaticClass();
	GameSessionClass = ADreamGameSession::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}


void ADreamGameMode::SetPlayerRespawnDelay(float NewRespawnDelay)
{
	// DefaultPlayerRespawnDelay = NewRespawnDelay;

	if (ADMGameState* DMGameState = GetGameState<ADMGameState>())
	{
		DMGameState->SetPlayerRespawnDelay(NewRespawnDelay);
	}
}

void ADreamGameMode::EndMatch()
{
	HandleEndMatch();
}

FString ADreamGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Error = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	
	if (Error.IsEmpty())
	{
		if (NewPlayerController->PlayerState)
		{
			//UE_LOG(LogDream, Verbose, TEXT("InitNewPlayer Options: %s"), *Options);

#if WITH_EDITOR

			NewPlayerController->PlayerState->SetPlayerId(FPDIStatic::Get()->GetClientPlayerID());

#else

			int32 PlayerId = UGameplayStatics::GetIntOption(Options, TEXT("PlayerId"), 0);
			NewPlayerController->PlayerState->SetPlayerId(PlayerId);	

#endif
		}
	}

	return Error;
}

void ADreamGameMode::UpdateActivePlayers() const
{
	FPDIStatic::Get()->UpdateActivePlayers(FUpdateServerPlayerParam(CurrentPlayers));
}

void ADreamGameMode::OnPlayerCharacterDie(APlayerController* PlayerController)
{
}

void ADreamGameMode::HandleEndMatch()
{
}

void ADreamGameMode::InitGameState()
{
	Super::InitGameState();
	
	if (ADMGameState* DMGameState = GetGameState<ADMGameState>())
	{
		DMGameState->SetPlayerRespawnDelay(DefaultPlayerRespawnDelay);
	}
}

void ADreamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

#if !WITH_EDITOR

	CurrentPlayers++;

	UpdateActivePlayers();

	if (ADPlayerController* DPlayer = Cast<ADPlayerController>(NewPlayer))
	{
		TSharedPtr<FInternetAddr> Addr = FPDIStatic::Get()->GetBackendServerAddr();

		uint32 Address;
		int32 Port;
		Addr->GetIp(Address);
		Addr->GetPort(Port);
		
		DPlayer->ClientChangeConnectedServer(Address, Port);
	}

#endif
	
}

void ADreamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

#if !WITH_EDITOR

	CurrentPlayers--;
	UpdateActivePlayers();
	
#endif
	
}
