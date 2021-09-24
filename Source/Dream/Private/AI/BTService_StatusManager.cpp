// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_StatusManager.h"

#include "AIController.h"
#include "DEnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBTService_StatusManager::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ADEnemyBase* EnemyOwner = nullptr;
	if (AAIController* ControllerOwner = OwnerComp.GetAIOwner())
	{
		EnemyOwner = Cast<ADEnemyBase>(ControllerOwner->GetPawn());
	}

	if (EnemyOwner)
	{
		UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();

		float HealthPercentage = EnemyOwner->GetHealth() / EnemyOwner->GetMaxHealth();
		if (HealthPercentage <= 0.1f)
		{
			BlackboardComponent->SetValueAsBool(NearDeathSelector.SelectedKeyName, true);
		}

		if (AActor* Target = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetPawnSelector.SelectedKeyName)))
		{
			float NewWalkSpeed = EnemyOwner->WalkSpeed;
			
			float Distance = FVector::Distance(Target->GetActorLocation(), EnemyOwner->GetActorLocation());
			if (Distance <= ThresholdForWalking)
			{
				NewWalkSpeed = EnemyOwner->JogSpeed;
			}

			if (EnemyOwner->GetCharacterMovement()->MaxWalkSpeed != NewWalkSpeed)
			{
				EnemyOwner->GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
			}
		}
		else
		{
			BlackboardComponent->ClearValue(TargetPawnSelector.GetSelectedKeyID());
		}
	}
}
