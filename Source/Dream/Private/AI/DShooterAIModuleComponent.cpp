// Fill out your copyright notice in the Description page of Project Settings.


#include "DShooterAIModuleComponent.h"



#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "DEnemyBase.h"
#include "DGameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Sight.h"
#include "Sound/SoundCue.h"

void UDShooterAIModuleComponent::OpenFire(const FVector& TargetLocation)
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        UAbilitySystemComponent* AbilitySystem = GetOwnerAI()->GetAbilitySystemComponent();
        if (AbilitySystem->TryActivateAbilitiesByTag(FireAbilityTag))
        {
            FHitResult Hit;
            bool bSuccess = UDGameplayStatics::LineTraceAndSendEvent(GetOwner(), FireEventTag,
                GetOwnerAI()->GetActorLocation(), TargetLocation, ECollisionChannel::ECC_Visibility, Hit);

            NetMulticastOpenFire(bSuccess ? Hit.ImpactPoint : TargetLocation);
        }
    }
}

void UDShooterAIModuleComponent::NetMulticastOpenFire_Implementation(const FVector_NetQuantize100& TargetLocation)
{
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    if (FireAnim)
    {
        GetOwnerAI()->PlayAnimMontage(FireAnim);
    }

    if (TrailParticles)
    {
        UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailParticles, WeaponMesh,
            WeaponSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
        NC->SetVectorParameter(TrailParamName, TargetLocation);
    }

    if (MuzzleFlash)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, WeaponSocketName, MuzzleOffset);
    }

    if (FireSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FireSound, WeaponMesh->GetSocketLocation(WeaponSocketName));
    }
}

void UDShooterAIModuleComponent::OnRegister()
{
    Super::OnRegister();
    
    TArray<UActorComponent*> ActorComponents = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(), WeaponMeshTagName);
    if (ActorComponents.Num() > 0)
    {
        WeaponMesh = Cast<UPrimitiveComponent>(ActorComponents[0]);
    }

    if (!WeaponMesh)
    {
        DREAM_NLOG(Error, TEXT("WeaponMesh Not Fund"));
    }
}

void UDShooterAIModuleComponent::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    Super::OnTargetPerceptionUpdated(Actor, Stimulus);
    
    UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();

    if (AIPerception->HasActiveStimulus(*Actor, UAISense::GetSenseID<UAISense_Sight>()))
    {
        OnCombatStatusChange.Broadcast(true);
        Blackboard->SetValueAsBool(BlackboardName_CanFire, true);
    }
    else
    {
        //bCombat = false;
        Blackboard->ClearValue(BlackboardName_CanFire);
    }
}

void UDShooterAIModuleComponent::LostAllHostileTarget()
{
    OnCombatStatusChange.Broadcast(false);
    AIController->GetBlackboardComponent()->ClearValue(BlackboardName_CanFire);
}
