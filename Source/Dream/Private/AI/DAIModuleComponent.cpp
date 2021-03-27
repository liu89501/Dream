// Fill out your copyright notice in the Description page of Project Settings.


#include "DAIModuleComponent.h"

#include "AIController.h"
#include "DreamType.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Team.h"

// Sets default values for this component's properties
UDAIModuleComponent::UDAIModuleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDAIModuleComponent::BeginPlay()
{
	Super::BeginPlay();

	//DREAM_NLOG(Error, TEXT("Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"));

	if (GetOwnerRole() == ROLE_Authority)
	{
		AIController = GetOwnerAI()->GetAIController();
		AIPerception = GetOwnerAI()->AIPerception;
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &UDAIModuleComponent::OnTargetPerceptionUpdated);
	}
}

void UDAIModuleComponent::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	ADCharacterBase* StimulusPawn = Cast<ADCharacterBase>(Actor);

	TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
	
	if (SenseClass->IsChildOf<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ActivateHostile(StimulusPawn);
		}
		else
		{
			RefreshActiveHostile();
		}
	}
	else if (SenseClass->IsChildOf<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ActivateHostile(StimulusPawn);
		}
	}
	else if (SenseClass->IsChildOf<UAISense_Team>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			/*if (ADEnemyBase* Teammate = Cast<ADEnemyBase>(Target))
			{
			Teammate->GetPerceptionComponent()->GetHostileActors();
			AIPerception->GetActorInfo()
			}*/
		}
		DREAM_NLOG(Error, TEXT("UAISense_Team: %s, %s"), *StimulusPawn->GetName(), *Stimulus.GetDebugDescription());
	}
}

void UDAIModuleComponent::ActivateHostile(ADCharacterBase* Hostile)
{
	ADEnemyBase* EnemyAI = GetOwnerAI();

	EnemyAI->GetCharacterMovement()->MaxWalkSpeed = EnemyAI->RunSpeed;
	
	AIController->SetFocus(Hostile);
    
	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();

	Blackboard->SetValueAsObject(BlackboardName_TargetPawn, Hostile);

	if (!Hostile->OnCharacterDeath.IsBoundToObject(this))
	{
		Hostile->OnCharacterDeath.AddUObject(this, &UDAIModuleComponent::HostileTargetDestroy);
	}
}

void UDAIModuleComponent::RefreshActiveHostile()
{
	TArray<AActor*> HostileActors;
	AIPerception->GetHostileActors(HostileActors);

	if (HostileActors.Num() > 0)
	{
		if (ADCharacterBase* NewHostile = Cast<ADCharacterBase>(HostileActors[0]))
		{
			ActivateHostile(NewHostile);
		}
	}
	else
	{
		LostAllHostileTarget();
	}
}

void UDAIModuleComponent::LostAllHostileTarget()
{
	AIController->ClearFocus(EAIFocusPriority::Gameplay);
	AIController->GetBlackboardComponent()->ClearValue(BlackboardName_TargetPawn);
	GetOwnerAI()->GetCharacterMovement()->MaxWalkSpeed = GetOwnerAI()->GetClass()->GetDefaultObject<ADEnemyBase>()->GetCharacterMovement()->MaxWalkSpeed;
}

void UDAIModuleComponent::HostileTargetDestroy(ADCharacterBase* DestroyedActor) const
{
	AIPerception->ForgetActor(DestroyedActor);
}

// Called every frame
void UDAIModuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
