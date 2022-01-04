// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameState.h"
#include "DCharacterPlayer.h"
#include "DGameplayStatics.h"
#include "DreamGameMode.h"
#include "UnrealNetwork.h"
#include "UserWidget.h"

void ADGameState::StartMatch()
{
	bMatchEnded = false;

	OnRep_MatchEnded();
}

void ADGameState::EndMatch()
{
	bMatchEnded = true;

	/** 服务器不会触发OnRep事件 手动调用一下 */
	OnRep_MatchEnded();
}

void ADGameState::OnRep_MatchEnded()
{
	UE_LOG(LogDream, Verbose, TEXT("EndMatch: %d"), GetLocalRole());
	
	if (IsRunningDedicatedServer())
	{
		return;
	}

	const ADreamGameMode* DGameMode = GetDefaultGameMode<ADreamGameMode>();

	if (DGameMode == nullptr)
	{
		UE_LOG(LogDream, Error, TEXT("OnRep_MatchEnded GameMode Invalid"));
		return;
	}

	APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());

	if (PlayerController == nullptr)
	{
		UE_LOG(LogDream, Error, TEXT("OnRep_MatchEnded PlayerController Invalid"));
		return;
	}

	ADCharacterPlayer* Character = Cast<ADCharacterPlayer>(PlayerController->GetPawn());

	if (Character == nullptr)
	{
		UE_LOG(LogDream, Error, TEXT("OnRep_MatchEnded Character Invalid"));
		return;
	}

	Character->StopAllActions();
	Character->RemovePlayerHUD();
	
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			
	PlayerController->SetInputMode(Mode);
	PlayerController->bShowMouseCursor = true;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, DGameMode->GetSettlementWidgetClass());
	Widget->AddToViewport(EWidgetOrder::Max);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &ADGameState::ReturnToMainWorld, SettlementWaitTime);
}

void ADGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADGameState, bMatchEnded);
	DOREPLIFETIME_CONDITION(ADGameState, SettlementWaitTime, COND_InitialOnly);
}

void ADGameState::ReturnToMainWorld()
{
	UDGameplayStatics::ReturnToHomeWorld(this);
}
