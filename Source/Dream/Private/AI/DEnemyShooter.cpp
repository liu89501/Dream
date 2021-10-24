// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/DEnemyShooter.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "DGameplayStatics.h"
#include "UnrealNetwork.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

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

	FHitResult Hit;
	bool bSuccess = UDGameplayStatics::LineTraceAndSendEvent(this, FireEventTag, GetActorLocation(), TargetLocation, ECC_Visibility, Hit);
	NetMulticastOpenFire(bSuccess ? Hit.ImpactPoint : TargetLocation);
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

void ADEnemyShooter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
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
