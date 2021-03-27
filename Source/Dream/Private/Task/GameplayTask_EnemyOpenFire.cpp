// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTask_EnemyOpenFire.h"
#include "DShooterAIModuleComponent.h"
#include "GameplayTasksComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"


UGameplayTask_EnemyOpenFire* UGameplayTask_EnemyOpenFire::EnemyOpenFire(ADEnemyBase* Shooter, AActor* InTargetPawn, float InIntervalTime, int InMinNumOfTimes, int InMaxNumOfTimes)
{
	if (Shooter)
	{
		UGameplayTask_EnemyOpenFire* Task = NewTask<UGameplayTask_EnemyOpenFire>(Shooter->TasksComponent);

		if (Task)
		{
			if (UDShooterAIModuleComponent* ShooterModule = Shooter->FindComponentByClass<UDShooterAIModuleComponent>())
			{
				Task->TargetPawn = InTargetPawn;
				Task->ShooterModuleComponent = ShooterModule;
				Task->IntervalTime = InIntervalTime;
				Task->ActualNumberOfTimes = UKismetMathLibrary::RandomIntegerInRange(InMinNumOfTimes, InMaxNumOfTimes);

				return Task;
			}
		}
	}
	
	return nullptr;
}

void UGameplayTask_EnemyOpenFire::AbortTask()
{
	GetWorld()->GetTimerManager().ClearTimer(Handle_OpenFire);
	OnCompleted.Broadcast();
	EndTask();
}

void UGameplayTask_EnemyOpenFire::Activate()
{
	GetWorld()->GetTimerManager().SetTimer(Handle_OpenFire, this, &UGameplayTask_EnemyOpenFire::OnFireComplete, IntervalTime, true, 0.f);
}

void UGameplayTask_EnemyOpenFire::OnFireComplete()
{
	ActualNumberOfTimes--;

	if (ShooterModuleComponent && TargetPawn)
	{
		ShooterModuleComponent->OpenFire(TargetPawn->GetActorLocation());
	}

	if (ActualNumberOfTimes == 0)
	{
		AbortTask();
	}
}