// Fill out your copyright notice in the Description page of Project Settings.

#include "DMGameState.h"
#include "DCharacterPlayer.h"
#include "DGameModeInLevel.h"
#include "DGameplayStatics.h"
#include "UnrealNetwork.h"
#include "UserWidget.h"

ADMGameState::ADMGameState()
	: PlayerRespawnDelay(0)
{
	
}

float ADMGameState::GetSettlementDelay() const
{
	if (const ADGameModeInLevel* GameModeInLevel = GetDefaultGameMode<ADGameModeInLevel>())
	{
		return GameModeInLevel->GetDefaultSettlementDelay();
	}

	return 30.f;
}

void ADMGameState::NotifyEndMatch()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastMatchEnd();
	}
}

float ADMGameState::GetPlayerRespawnDelay(AController* Controller) const
{
	return PlayerRespawnDelay;
}

void ADMGameState::MulticastMatchEnd_Implementation()
{
	OnMatchEnded.Broadcast();

	if (GetLocalRole() != ROLE_Authority)
	{
		AddSettlementUIToViewport();
	}
}

void ADMGameState::AddSettlementUIToViewport()
{
	const ADGameModeInLevel* GameModeCDO = GetDefaultGameMode<ADGameModeInLevel>();

	if (GameModeCDO == nullptr)
	{
		UE_LOG(LogDream, Error, TEXT("AddSettlementUIToViewport ADGameModeInLevel Invalid"));
		return;
	}
	
	APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());

	if (PlayerController == nullptr)
	{
		UE_LOG(LogDream, Error, TEXT("AddSettlementUIToViewport PlayerController Invalid"));
		return;
	}

	ADCharacterPlayer* Character = Cast<ADCharacterPlayer>(PlayerController->GetPawn());

	if (Character == nullptr)
	{
		UE_LOG(LogDream, Error, TEXT("AddSettlementUIToViewport Character Invalid"));
		return;
	}

	Character->StopAllActions();
	Character->SetPlayerHUDVisible(false);
	
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			
	PlayerController->SetInputMode(Mode);
	PlayerController->bShowMouseCursor = true;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, GameModeCDO->GetSettlementWidgetClass());
	Widget->AddToViewport(EWidgetOrder::Max);
}

void ADMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADMGameState, PlayerRespawnDelay);
}
