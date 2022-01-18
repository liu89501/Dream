// Fill out your copyright notice in the Description page of Project Settings.

#include "DDropSkeletalMesh.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ADDropSkeletalMesh::ADDropSkeletalMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetSimulatePhysics(true);
	Mesh->BodyInstance.bGenerateWakeEvents = true;

	RootComponent = Mesh;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(RootComponent);

	Effect = CreateDefaultSubobject<UNiagaraComponent>("Effect");
	Effect->SetupAttachment(RootComponent);
	Effect->bAutoActivate = false;
	
	Mesh->OnComponentSleep.AddDynamic(this, &ADDropSkeletalMesh::OnSleep);
	Mesh->OnComponentWake.AddDynamic(this, &ADDropSkeletalMesh::OnWake);
}

void ADDropSkeletalMesh::OnSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	Effect->SetWorldRotation(FRotator::ZeroRotator);
	Effect->Activate(true);
}

void ADDropSkeletalMesh::OnWake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	Effect->Deactivate();
}