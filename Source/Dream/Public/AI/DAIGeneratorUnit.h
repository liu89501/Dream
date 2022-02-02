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

	UPROPERTY(EditInstanceOnly, Category=DMAIGenerator)
	TSubclassOf<class ADEnemyBase> AIClass;

#if WITH_EDITORONLY_DATA
	
	UPROPERTY(VisibleAnywhere, Category=DMAIGenerator)
	class UArrowComponent* Arrow;
	
#endif 

public:

	void InitializeAI(ADEnemyBase* AI);

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

};


