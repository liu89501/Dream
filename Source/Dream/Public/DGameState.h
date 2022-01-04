// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DGameState.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY(ReplicatedUsing=OnRep_MatchEnded)
	bool bMatchEnded;

	UPROPERTY(Replicated)
	float SettlementWaitTime;

public:

	void StartMatch();
	
	void EndMatch();

protected:

	UFUNCTION()
	void OnRep_MatchEnded();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void ReturnToMainWorld();
};
