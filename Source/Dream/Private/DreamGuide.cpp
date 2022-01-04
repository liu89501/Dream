// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGuide.h"
#include "DreamWidgetStatics.h"
#include "Components/BillboardComponent.h"

// Sets default values
ADreamGuide::ADreamGuide()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	IconComponent = CreateDefaultSubobject<UIconComponent>("IconComponent");
	IconComponent->SetAutoActivate(true);

	Billboard = CreateDefaultSubobject<UBillboardComponent>("Billboard");
	RootComponent = Billboard;

	bReplicates = false;
}

// Called when the game starts or when spawned
void ADreamGuide::BeginPlay()
{
	Super::BeginPlay();
	
#if !UE_SERVER
	
	if (bAutoRegister)
	{
		UDreamWidgetStatics::AddGuideActorToLocalPlayer(this);
	}

#endif

}

UIconComponent* ADreamGuide::GetIconComponent() const
{
	return IconComponent;
}

void ADreamGuide::AddSelf()
{

#if !UE_SERVER

	UDreamWidgetStatics::AddGuideActorToLocalPlayer(this);

#endif
	
}

void ADreamGuide::RemoveSelf()
{
	
#if !UE_SERVER

	UDreamWidgetStatics::RemoveGuideActorFromLocalPlayer(this);

#endif
	
}

