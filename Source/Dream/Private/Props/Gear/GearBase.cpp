// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Gear/GearBase.h"

// Sets default values
AGearBase::AGearBase()
{
	GearMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TEXT"));
	GearMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = GearMesh;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AGearBase::Equipped(UPrimitiveComponent* TargetComponent)
{
	AttachToComponent(TargetComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachSocketName);
	SetActorRelativeTransform(AttachSocketTransform);
}

// Called when the game starts or when spawned
void AGearBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGearBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

