// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQC_PlayerContext.h"
#include "DCharacterPlayer.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"

void UEQC_PlayerContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QuerierActor = Cast<AActor>(QueryInstance.Owner.Get());
	if (QuerierActor == nullptr)
	{
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(QuerierActor, ADCharacterPlayer::StaticClass(), OutActors);
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, OutActors);
}
