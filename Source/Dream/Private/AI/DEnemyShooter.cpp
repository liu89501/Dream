// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/DEnemyShooter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DGameplayStatics.h"
#include "DGameplayTags.h"
#include "DMShooterWeaponComponent.h"
#include "UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"

void FOpenFireAction::UpdateOperation(FLatentResponse& Response)
{
	TickCount += Response.ElapsedTime();

	if (TickCount >= Interval)
	{
		TickCount = 0.f;
		NumberOfFire--;

		if (ADEnemyShooter* EnemyShooter = Shooter.Get())
		{
			EnemyShooter->HandleFiring(Hostile.Get());
		}
	}
	
	Response.FinishAndTriggerIf(bStop || NumberOfFire == 0, ExecutionFunction, OutputLink, CallbackTarget);
}

ADEnemyShooter::ADEnemyShooter()
	: RandomFiringRange(FRangeRandomInt(5, 15))
	, FirePerMinute(500)
{
}

void ADEnemyShooter::OpenFire(AActor* TargetActor, FLatentActionInfo LatentInfo, EOpenFireOption Option)
{
	FLatentActionManager& LatentActionManager = GetWorld()->GetLatentActionManager();

	FOpenFireAction* ExistingAction = LatentActionManager.FindExistingAction<FOpenFireAction>(this, LatentInfo.UUID);

	if (Option == EOpenFireOption::Fire)
	{
		if (ExistingAction == nullptr)
		{
			float Interval = 1 / (FirePerMinute / 60.f);
			int32 Num = RandomFiringRange.GetRandomInt();
			FOpenFireAction* OpenFireAction = new FOpenFireAction(LatentInfo, Interval, Num, this, TargetActor);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, OpenFireAction);
		}
	}
	else if (ExistingAction != nullptr)
	{
		ExistingAction->bStop = true;
	}
}

void ADEnemyShooter::HandleFiring(AActor* TargetActor)
{
	if (WeaponTypeComponent)
	{
		WeaponTypeComponent->OpenFire(TargetActor);
	}
}

void ADEnemyShooter::SetDiscoverEnemy(bool bNewDiscoverEnemy)
{
	bDiscoverEnemy = bNewDiscoverEnemy;
	MARK_PROPERTY_DIRTY_FROM_NAME(ADEnemyShooter, bDiscoverEnemy, this);
}

FVector ADEnemyShooter::GetWeaponMuzzleLocation() const
{
	return GetWeaponMesh()->GetSocketLocation(WeaponMuzzleSocketName);
}

void ADEnemyShooter::ApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin)
{
	OnApplyProjectileDamage.Broadcast(HitResult, Origin);
}

void ADEnemyShooter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	WeaponTypeComponent = FindComponentByClass<UDMShooterWeaponComponent>();
}

void ADEnemyShooter::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(WeaponTypeComponent, TEXT("WeaponTypeComponent Require not null"));
}

void ADEnemyShooter::HealthChanged(const FOnAttributeChangeData& AttrData)
{
	Super::HealthChanged(AttrData);
}

void ADEnemyShooter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params_DiscoverEnemy;
	Params_DiscoverEnemy.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ADEnemyShooter, bDiscoverEnemy, Params_DiscoverEnemy);
}

void ADEnemyShooter::OnTargetPerceptionUpdated(AActor* StimulusActor, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(StimulusActor, Stimulus);
}

void ADEnemyShooter::LostAllHostileTarget()
{
	Super::LostAllHostileTarget();
}
