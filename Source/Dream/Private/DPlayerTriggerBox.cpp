// Fill out your copyright notice in the Description page of Project Settings.


#include "DPlayerTriggerBox.h"
#include "DCharacterPlayer.h"
#include "Components/ShapeComponent.h"

ADPlayerTriggerBox::ADPlayerTriggerBox()
	: TriggerMode(ETriggerMode::ServerOnly),
	  bTriggered(false)
{
	PrimaryActorTick.bCanEverTick = false;

	GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ADPlayerTriggerBox::OnBeginOverlap);
}

void ADPlayerTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerMode == ETriggerMode::ServerOnly && GetLocalRole() != ROLE_Authority)
	{
		GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ADPlayerTriggerBox::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ADPlayerTriggerBox::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                        const FHitResult& SweepResult)
{

	if (!OtherActor->IsA<ADCharacterPlayer>())
	{
		return;
	}

	ADCharacterPlayer* PlayerCharacter = Cast<ADCharacterPlayer>(OtherActor);

	if (!PlayerCharacter)
	{
		return;
	}
	
	if (TriggerMode == ETriggerMode::LocalOnly && !PlayerCharacter->IsLocallyControlled())
	{
		return;
	}

	if (TriggerMode == ETriggerMode::ServerAndLocal && !(PlayerCharacter->IsLocallyControlled() || GetLocalRole() == ROLE_Authority))
	{
		return;
	}

	if (bTriggered)
	{
		return;
	}

	bTriggered = true;

	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// LocalOnly 和 ServerAndLocal 都触发
	OnTrigger.Broadcast();
	OnTrigger.Clear();
}
