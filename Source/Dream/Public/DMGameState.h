// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DMGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchEnded);

/**
 * 
 */
UCLASS()
class DREAM_API ADMGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	ADMGameState();

public:

	UPROPERTY(BlueprintAssignable)
	FOnMatchEnded OnMatchEnded;

public:

	UFUNCTION(BlueprintCallable, Category=DMGameState)
	float GetSettlementDelay() const;

public:

	void NotifyEndMatch();

	void SetPlayerRespawnDelay(float InPlayerRespawnDelay)
	{
		PlayerRespawnDelay = InPlayerRespawnDelay;
	}

	virtual float GetPlayerRespawnDelay(AController* Controller) const override;

protected:

	UFUNCTION(NetMulticast, Reliable)
	void MulticastMatchEnd();

	void AddSettlementUIToViewport();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(Replicated)
	float PlayerRespawnDelay;
};
