// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DPlayerState.generated.h"

/**
 *
 */
UCLASS()
class DREAM_API ADPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	ADPlayerState();

public:

	void RecordDamage(uint32 Damage);

	void IncreaseKills();

	FORCEINLINE uint32 GetTotalDamage() const
	{
		return TotalDamage;
	}
	
	FORCEINLINE uint16 GetKills() const
	{
		return Kills;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(Replicated)
	uint32 TotalDamage;
	
	UPROPERTY(Replicated)
	uint16 Kills;

};
