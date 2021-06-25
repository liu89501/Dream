// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTask_EnemyOpenFire.h"

#include "DEnemyShooter.h"
#include "GameplayTasksComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"


UGameplayTask_EnemyOpenFire* UGameplayTask_EnemyOpenFire::EnemyOpenFire(ADEnemyShooter* Shooter, AActor* InTargetPawn, float InIntervalTime, int InMinNumOfTimes, int InMaxNumOfTimes)
{
	if (Shooter)
	{
		UGameplayTask_EnemyOpenFire* Task = NewTask<UGameplayTask_EnemyOpenFire>(Shooter->TasksComponent);

		if (Task)
		{
			Task->OwnerEnemy = Shooter;
			Task->TargetPawn = InTargetPawn;
			Task->IntervalTime = InIntervalTime;
			Task->ActualNumberOfTimes = UKismetMathLibrary::RandomIntegerInRange(InMinNumOfTimes, InMaxNumOfTimes);
			return Task;
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
	GetWorld()->GetTimerManager().SetTimer(Handle_OpenFire, this, &UGameplayTask_EnemyOpenFire::OnFiring, IntervalTime, true, 0.f);
}

void UGameplayTask_EnemyOpenFire::OnFiring()
{
	OwnerEnemy->OpenFire(TargetPawn->GetActorLocation());

	ActualNumberOfTimes--;

	if (ActualNumberOfTimes == 0)
	{
		AbortTask();
	}
}