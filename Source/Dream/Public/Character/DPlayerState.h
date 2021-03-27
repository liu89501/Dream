// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
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

	UPROPERTY(BlueprintReadWrite, Replicated, Category = FpsPlayerState)
	int32 KillNum;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = FpsPlayerState)
	int32 DieNum;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = FpsPlayerState)
	ETeamName Team;

	UFUNCTION(BlueprintCallable, Category = FpsPlayerStateApi)
	FLinearColor GetTeamColor() const;

protected:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(EditAnywhere)
	FLinearColor OneTeamColor;

	UPROPERTY(EditAnywhere)
	FLinearColor TwoTeamColor;

};
