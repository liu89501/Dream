// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DEnemyBase.h"
#include "GameplayTask.h"
#include "GameplayTask_EnemyOpenFire.generated.h"


class ADEnemyShooter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOpenFireCompletedDelegate);

/**
 * 
 */
UCLASS()
class DREAM_API UGameplayTask_EnemyOpenFire : public UGameplayTask
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "AI|Task", meta = (BlueprintInternalUseOnly = "TRUE", DisplayName = "AI OpenFire"))
	static UGameplayTask_EnemyOpenFire* EnemyOpenFire(ADEnemyShooter* Shooter, AActor* InTargetPawn, float InIntervalTime, int InMinNumOfTimes, int InMaxNumOfTimes);

	UFUNCTION(BlueprintCallable, Category = "AI|Task|OpenFire")
	void AbortTask();

protected:

	UPROPERTY(BlueprintAssignable)
	FOpenFireCompletedDelegate OnCompleted;

	virtual void Activate() override;

	void OnFiring();

private:

	float IntervalTime;

	int32 ActualNumberOfTimes;

	UPROPERTY()
	AActor* TargetPawn;
	UPROPERTY()
	ADEnemyShooter* OwnerEnemy;

	FTimerHandle Handle_OpenFire;
	
};
