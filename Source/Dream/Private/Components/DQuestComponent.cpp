// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DQuestComponent.h"


#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"

// Sets default values for this component's properties
UDQuestComponent::UDQuestComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UDQuestComponent::UpdateTaskState(const FQuestActionHandle& Handle)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FPDIStatic::Get()->UpdateTaskState(Handle);
		
	}
	else
	{
		for (TPair<int32, FTaskInformation>& Pair : InProgressTasks)
		{
			Pair.Value.CompleteCondition->UpdateCondition(Handle);
		}
	}
}


// Called when the game starts
void UDQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UDQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

