// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "GameFramework/PlayerStart.h"
#include "DTeamPlayerStart.generated.h"

/**
 *
 */
UCLASS()
class DREAM_API ADTeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Team)
	ETeamName OwningTeam;
};
