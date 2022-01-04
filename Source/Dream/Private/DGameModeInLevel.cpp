// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameModeInLevel.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

ADGameModeInLevel::ADGameModeInLevel()
{
	PlayerResurrectionTime = 10.f;
	StartSpotIndex = 0;
}

void ADGameModeInLevel::RefreshStartSpot(FName PlayerStartTag)
{
	StartSpotIndex = 0;

	AvailableStartSpot.Reset();
	
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart && PlayerStart->PlayerStartTag == PlayerStartTag)
		{
			AvailableStartSpot.Add(PlayerStart);
		}
	}
}

bool ADGameModeInLevel::GetPlayerRestartTransform(FTransform& RestartTransform)
{
	if (AvailableStartSpot.Num() == 0)
	{
		return false;
	}

	if (AvailableStartSpot.IsValidIndex(StartSpotIndex))
	{
		RestartTransform = AvailableStartSpot[StartSpotIndex]->GetActorTransform();
		
		StartSpotIndex = StartSpotIndex == AvailableStartSpot.Num() - 1 ? 0 : StartSpotIndex + 1;

		return true;
	}

	return false;
}
