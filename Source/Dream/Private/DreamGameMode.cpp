// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DreamGameMode.h"
#include "DGameState.h"
#include "DPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DreamType.h"
#include "DreamGameSession.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


int32 ADreamGameMode::DEFAULT_MAX_PLAYERS = 4;

ADreamGameMode::ADreamGameMode()
	: PlayerResurrectionTime(5.f)
	, MaxPlayers(4)
	, SettlementWaitTime(15.f)
{
	PlayerStateClass = APlayerState::StaticClass();
	GameSessionClass = ADreamGameSession::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADreamGameMode::ReSpawnCharacter(ACharacter* Character)
{
	FTimerHandle Handle;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ADreamGameMode::OnReSpawnCharacter, Character);
	GetWorldTimerManager().SetTimer(Handle, Delegate, PlayerResurrectionTime, false);
}

FOnClientRPC& ADreamGameMode::GetClientRPCDelegate(const FGameplayTag& Tag)
{
	return ClientRPC.FindOrAdd(Tag);
}

void ADreamGameMode::BroadcastClientRPCDelegate(const FGameplayTag& Tag)
{
	if (FOnClientRPC* ClientRPCPtr = ClientRPC.Find(Tag))
	{
		ClientRPCPtr->Broadcast();
	}
}

void ADreamGameMode::OnReSpawnCharacter(ACharacter* Character)
{
	FActorSpawnParameters Params;
	Params.Owner = Character->Controller;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform Transform;
	if (!GetPlayerRestartTransform(Transform))
	{
		Transform = Character->GetActorTransform();
	}
	
	ACharacter* NewCharacter = GetWorld()->SpawnActor<ACharacter>(DefaultPawnClass, Transform, Params);
	
	Character->Controller->Possess(NewCharacter);

	Character->Destroy();
}

void ADreamGameMode::EndMatch()
{
	if (ADGameState* DGameState = GetGameState<ADGameState>())
	{
		DGameState->EndMatch();

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, this, &ADreamGameMode::OnResetGame, SettlementWaitTime + 1);
	}
}

AActor* ADreamGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

FString ADreamGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Error = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	
	if (Error.IsEmpty())
	{
		if (NewPlayerController->PlayerState)
		{
			//UE_LOG(LogDream, Verbose, TEXT("InitNewPlayer Options: %s"), *Options);
			
			int32 PlayerId = UGameplayStatics::GetIntOption(Options, TEXT("PlayerId"), 0);
			NewPlayerController->PlayerState->SetPlayerId(PlayerId);	
		}
	}

	return Error;
}

bool ADreamGameMode::GetPlayerRestartTransform(FTransform& RestartTransform)
{
	return false;
}

void ADreamGameMode::OnResetGame() const
{
	for (TPlayerControllerIterator<APlayerController>::ServerAll It(GetWorld()); It; ++It)
	{
		GameSession->KickPlayer(*It, FText::FromString(TEXT("EndMatch")));
	}
	
	GetWorld()->ServerTravel(TEXT("?Restart"), true);
}

void ADreamGameMode::UpdateActivePlayers() const
{
	FPDIStatic::Get()->UpdateActivePlayers(FUpdateServerPlayerParam(CurrentPlayers));
}

void ADreamGameMode::InitGameState()
{
	Super::InitGameState();
	
	if (ADGameState* DGameState = GetGameState<ADGameState>())
	{
		DGameState->SettlementWaitTime = SettlementWaitTime;
	}
}

void ADreamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	IfStandalone(return);
	
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
	
}

void ADreamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	IfStandalone(return);
	
	CurrentPlayers--;
	
	UpdateActivePlayers();
}

void ADreamGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ADreamGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}
