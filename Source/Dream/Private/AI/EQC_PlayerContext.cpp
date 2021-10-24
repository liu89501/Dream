// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQC_PlayerContext.h"
#include "DCharacterPlayer.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"

void UEQC_PlayerContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	if (QueryInstance.World == nullptr)
	{
		return;
	}

	TArray<AActor*> PlayerCharacters;
	for (FConstPlayerControllerIterator It = QueryInstance.World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APawn* Pawn = It->Get()->GetPawn())
		{
			PlayerCharacters.Add(Pawn);
		}
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerCharacters);
}
