// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "AIShooter_OpenFire.generated.h"

class ADEnemyShooter;

UENUM(BlueprintType)
enum class EShooterFireAction : uint8
{
	Fire,
	StopFire
};

/**
 * 
 */
UCLASS()
class DREAM_API UAIShooter_OpenFire : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

public:

	// Called when there is a successful
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnCompleted;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="AIShooter|Async")
	static UAIShooter_OpenFire* AIShooterFire(UObject* WorldContextObject, ADEnemyShooter* InAIShooter, AActor* InHostile);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

private:

	void OnFiring();

private:

	UPROPERTY()
	AActor* Hostile;

	UPROPERTY()
	ADEnemyShooter* AIShooter;

	int32 NumberOfFire;
	FTimerHandle Handle_Firing;
};



