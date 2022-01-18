// Fill out your copyright notice in the Description page of Project Settings.

#include "DDropStaticMesh.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ADDropStaticMesh::ADDropStaticMesh()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->BodyInstance.bGenerateWakeEvents = true;
	Mesh->SetSimulatePhysics(true);

	RootComponent = Mesh;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
	TriggerSphere->SetupAttachment(RootComponent);

	IndicationEffect = CreateDefaultSubobject<UNiagaraComponent>("IndicationEffect");
	IndicationEffect->SetupAttachment(RootComponent);
	IndicationEffect->bAutoActivate = false;

	Mesh->OnComponentSleep.AddDynamic(this, &ADDropStaticMesh::OnSleep);
	Mesh->OnComponentWake.AddDynamic(this, &ADDropStaticMesh::OnWake);
}

void ADDropStaticMesh::OnSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	IndicationEffect->SetWorldRotation(FRotator::ZeroRotator);
	IndicationEffect->Activate(true);
}

void ADDropStaticMesh::OnWake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	IndicationEffect->Deactivate();
}
