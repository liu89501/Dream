// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameModeInLevel.h"
#include "DMGameState.h"
#include "DMPlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ADGameModeInLevel::ADGameModeInLevel()
	: DefaultSettlementDelay(30.f)
	, SurvivingPlayers(0)
	, StartSpotIndex(0)
{
	DefaultPlayerRespawnDelay = 10.f;
}

void ADGameModeInLevel::SetProgressTag(FName InProgressTag)
{
	LevelProgressTag = InProgressTag;
	
	StartSpotIndex = 0;

	AvailableStartSpot.Reset();
	
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart && PlayerStart->PlayerStartTag == InProgressTag)
		{
			AvailableStartSpot.Add(PlayerStart);
		}
	}
}

void ADGameModeInLevel::OnPlayerCharacterDie(APlayerController* PlayerController)
{
	Super::OnPlayerCharacterDie(PlayerController);
	
	SurvivingPlayers = FMath::Max(SurvivingPlayers - 1, 0);

	if (SurvivingPlayers == 0)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ADMPlayerController* PC = Cast<ADMPlayerController>(It->Get()))
			{
				PC->CancelRespawnCharacter();
			}
		}
		
		FString Param = FString::Printf(TEXT("?Restart&ProgressTag=%s"), *LevelProgressTag.ToString());

		// todo 重新开始比赛。。。 待处理
		GetWorld()->ServerTravel(*Param, true);
	}
}

bool ADGameModeInLevel::PlayerCanRestart_Implementation(APlayerController* Player)
{
	bool bPlayerCanRestart = Super::PlayerCanRestart_Implementation(Player);

	return bPlayerCanRestart && SurvivingPlayers > 0;
}

void ADGameModeInLevel::HandleEndMatch()
{
	if (ADMGameState* DMGameState = GetGameState<ADMGameState>())
	{
		DMGameState->NotifyEndMatch();

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, this, &ADGameModeInLevel::OnResetGame, DefaultSettlementDelay + 1);
	}
}

void ADGameModeInLevel::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	FString ProgressTag = UGameplayStatics::ParseOption(Options, TEXT("ProgressTag"));

	if (!ProgressTag.IsEmpty())
	{
		SetProgressTag(FName(ProgressTag));
	}
}

void ADGameModeInLevel::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	SurvivingPlayers++;
}

void ADGameModeInLevel::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	SurvivingPlayers = FMath::Max(SurvivingPlayers - 1, 0);
}

AActor* ADGameModeInLevel::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (AvailableStartSpot.Num() > 0)
	{
		if (AvailableStartSpot.IsValidIndex(StartSpotIndex))
		{
			StartSpotIndex = StartSpotIndex == AvailableStartSpot.Num() - 1 ? 0 : StartSpotIndex + 1;

			return AvailableStartSpot[StartSpotIndex];
		}
	}
	
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void ADGameModeInLevel::OnResetGame()
{
	ReturnToMainMenuHost();
	GetWorld()->ServerTravel(TEXT("?Restart"), true);
}
