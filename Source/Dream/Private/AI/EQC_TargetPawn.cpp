// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQC_TargetPawn.h"

#include "AIController.h"
#include "DEnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"

void UEQC_TargetPawn::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	ADEnemyBase* QuerierActor = Cast<ADEnemyBase>(QueryInstance.Owner.Get());
	if (QuerierActor == nullptr)
	{
		return;
	}

	AAIController* AIController = QuerierActor->GetAIController();
	if (AIController == nullptr)
	{
		return;
	}
	
	AActor* HostileTarget = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject(QuerierActor->BlackboardName_HostileTarget));
	if (HostileTarget)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, HostileTarget);
	}
}
