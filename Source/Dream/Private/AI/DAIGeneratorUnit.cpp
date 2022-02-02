// Fill out your copyright notice in the Description page of Project Settings.

#include "DAIGeneratorUnit.h"
#include "DEnemyBase.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"

// Sets default values
ADAIGeneratorUnit::ADAIGeneratorUnit()
{
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 92.0f);
	GetCapsuleComponent()->SetShouldUpdatePhysicsVolume(false);

#if WITH_EDITORONLY_DATA

	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(RootComponent);

#endif
	
}

void ADAIGeneratorUnit::InitializeAI(ADEnemyBase* AI)
{
	if (AI)
	{
		AI->ActivateBehaviorTree();
	}
}

#if WITH_EDITOR

void ADAIGeneratorUnit::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ADAIGeneratorUnit, AIClass))
	{
		if (AIClass)
		{
			ADEnemyBase* EnemyCDO = AIClass->GetDefaultObject<ADEnemyBase>();

			UCapsuleComponent* AICapsuleComponent = EnemyCDO->GetCapsuleComponent();
			GetCapsuleComponent()->SetCapsuleSize(
                AICapsuleComponent->GetUnscaledCapsuleRadius(), AICapsuleComponent->GetUnscaledCapsuleHalfHeight());
		}
	}
}

#endif