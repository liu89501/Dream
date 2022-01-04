// Fill out your copyright notice in the Description page of Project Settings.


#include "DAbilityGen_TrackingExplosive.h"
#include "DGameplayStatics.h"
#include "DProjectileComponent.h"
#include "UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

ADAbilityGen_TrackingExplosive::ADAbilityGen_TrackingExplosive()
	: TargetActor(nullptr)
	, ImpactRadius(600.f)
	, ExplodeParticle(nullptr)
	, ExplodeSound(nullptr)
	, ApplyEffect(nullptr)
	, bImpact(false)
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	RootComponent = SphereCollision;

	SphereCollision->SetSphereRadius(10.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(Collision_ObjectType_Projectile);

	ProjectileMovement = CreateDefaultSubobject<UDProjectileComponent>("ProjectileMovement");
	ProjectileMovement->bIsHomingProjectile = true;

	bReplicates = true;
	SetReplicatingMovement(true);

	PrimaryActorTick.bCanEverTick = false;

	InitialLifeSpan = 5.f;

	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ADAbilityGen_TrackingExplosive::OnImpact);
}

void ADAbilityGen_TrackingExplosive::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		FTimerHandle Temporary;
		GetWorldTimerManager().SetTimer(Temporary, this, &ADAbilityGen_TrackingExplosive::OnAcceleration, StartAccelerationTime);
	}
}

void ADAbilityGen_TrackingExplosive::OnAcceleration() const
{
	FVector AccelerationDir;
	
	if (TargetActor)
	{
		ProjectileMovement->HomingTargetComponent = TargetActor->GetRootComponent();
		AccelerationDir = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}
	else
	{
		AccelerationDir = GetInstigator()->GetViewRotation().Vector();
	}
	
	ProjectileMovement->Velocity = ProjectileMovement->MaxSpeed * AccelerationDir;
}

void ADAbilityGen_TrackingExplosive::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereCollision->MoveIgnoreActors.Add(GetInstigator());

	if (GetLocalRole() == ROLE_Authority)
	{
		float RandomRoll = UKismetMathLibrary::RandomFloatInRange(-120.f, 120.f);
		FRotator ComposeRotators = GetActorRotation() + FRotator(60.f, RandomRoll, 0);
		ProjectileMovement->Velocity = ComposeRotators.Vector() * ProjectileMovement->InitialSpeed;
	}
}

void ADAbilityGen_TrackingExplosive::OnImpact(const FHitResult& ImpactResult)
{
	if (GetLocalRole() != ROLE_Authority || bImpact)
	{
		return;
	}

#if WITH_EDITOR

	if (GetNetMode() == NM_Standalone)
	{
		OnRep_Impact();
	}

#endif

	if (ImpactResult.bBlockingHit)
	{
		SetActorHiddenInGame(true);
		
		UDGameplayStatics::SphereTraceAndApplyEffect(GetInstigator(), GetActorLocation(),
			ImpactRadius, ETraceTypeQuery::TraceTypeQuery1, ApplyEffect, true, false, true);
	}

	bImpact = true;
}

void ADAbilityGen_TrackingExplosive::OnRep_Impact()
{
	if (ExplodeParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeParticle, GetActorLocation(),
			FRotator::ZeroRotator, FVector::OneVector, true, EPSCPoolMethod::AutoRelease);
	}

	if (ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}

	SetLifeSpan(2.f);
}

void ADAbilityGen_TrackingExplosive::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ADAbilityGen_TrackingExplosive, bImpact);
}
