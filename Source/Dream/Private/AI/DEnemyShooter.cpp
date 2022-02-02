// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/DEnemyShooter.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "DGameplayStatics.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Sound/SoundCue.h"

void ADEnemyShooter::NetMulticastOpenFire_Implementation(const FVector_NetQuantize& TargetLocation)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (FireAnim)
	{
		PlayAnimMontage(FireAnim);
	}

	FVector SocketLocation = WeaponMesh->GetSocketLocation(WeaponMuzzleSocketName);

	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, WeaponMuzzleSocketName);
	}

	if (FireSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FireSound, SocketLocation);
	}

	UDGameplayStatics::SpawnWeaponTrailParticles(GetWorld(), TrailVfx, SocketLocation, TargetLocation);
}

void ADEnemyShooter::OpenFire(const FVector& TargetLocation)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	FVector ActorLocation = GetActorLocation();

	FVector Dir = TargetLocation - ActorLocation;
	Dir.Normalize();

	FVector TraceEnd = TargetLocation + Dir * 200.f;

	FHitResult Hit;
	bool bSuccess = UDGameplayStatics::LineTraceAndSendEvent(this, FireEventTag, ActorLocation, TraceEnd, ECC_Visibility, Hit);
	NetMulticastOpenFire(bSuccess ? Hit.ImpactPoint : TraceEnd);
}

void ADEnemyShooter::HealthChanged(const FOnAttributeChangeData& AttrData)
{
	Super::HealthChanged(AttrData);

	if (IsDeath())
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		WeaponMesh->SetSimulatePhysics(true);
	}
}

void ADEnemyShooter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADEnemyShooter, bDiscoverEnemy);
}

void ADEnemyShooter::OnTargetPerceptionUpdated(ADCharacterBase* StimulusPawn, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(StimulusPawn, Stimulus);
    
	/*UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();

	if (AIPerception->HasActiveStimulus(*StimulusPawn, UAISense::GetSenseID<UAISense_Sight>()))
	{
		bDiscoverEnemy = true;
		Blackboard->SetValueAsBool(BlackboardName_CanOpenFire, true);
	}
	else
	{
		Blackboard->ClearValue(BlackboardName_CanOpenFire);
	}*/
}

void ADEnemyShooter::LostAllHostileTarget()
{
	Super::LostAllHostileTarget();

	/*bDiscoverEnemy = false;
	AIController->GetBlackboardComponent()->ClearValue(BlackboardName_CanOpenFire);*/
}
