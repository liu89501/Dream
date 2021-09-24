// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DreamGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "DreamType.h"
#include "DreamGameSession.h"
#include "Character/DPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

#define IdleShutdownTime 300 

ADreamGameMode::ADreamGameMode()
	: Super()
{
	PlayerStateClass = APlayerState::StaticClass();
	GameSessionClass = ADreamGameSession::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = 1.f;

	PlayerResurrectionTime = 5.f;
}

AActor* ADreamGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

FString ADreamGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void ADreamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	IfStandalone(return);
	
	IdleTimeCount = 0.f;
	PlayerNumCounter.Increment();
	if (FPlayerDataInterface* PlayerDataInterface = FPlayerDataInterfaceStatic::Get())
	{
		PlayerDataInterface->UpdateActivePlayers(true);
	}
}

void ADreamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	IfStandalone(return);

	if (Exiting->IsA(APlayerController::StaticClass()))
	{
		PlayerNumCounter.Decrement();
		if (FPlayerDataInterface* PlayerDataInterface = FPlayerDataInterfaceStatic::Get())
		{
			PlayerDataInterface->UpdateActivePlayers(false);
		}
	}
}

void ADreamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PlayerNumCounter.GetValue() == 0)
	{
		if (IdleTimeCount > IdleShutdownTime)
		{
			// 长时间没有玩家连接到服务器时 服务器将关闭
			FPlatformMisc::RequestExit(false);
		}

		IdleTimeCount += DeltaSeconds;
	}
}

void ADreamGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ADreamGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (GetNetMode() != NM_Standalone)
	{
		int32 AutoShutdown = 0;
		FParse::Value(FCommandLine::Get(), TEXT("bAutoShutdown="), AutoShutdown);
		bAutoShutdown = AutoShutdown > 0;

		if (bAutoShutdown)
		{
			SetActorTickEnabled(true);
		}
	}
}
