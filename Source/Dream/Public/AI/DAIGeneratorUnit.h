// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NavigationObjectBase.h"
#include "DAIGeneratorUnit.generated.h"

UCLASS()
class DREAM_API ADAIGeneratorUnit : public ANavigationObjectBase
{
	GENERATED_BODY()

public:

	ADAIGeneratorUnit();

	UPROPERTY(EditInstanceOnly, Category=AIGenerator)
	TSubclassOf<class ADEnemyBase> AIClass;

public:

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

};

