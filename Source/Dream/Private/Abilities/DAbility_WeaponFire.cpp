// Fill out your copyright notice in the Description page of Project Settings.


#include "DAbility_WeaponFire.h"
#include "AbilitySystemComponent.h"
#include "DGE_WeaponBaseDamage.h"
#include "DCharacterPlayer.h"
#include "ShootWeapon.h"
#include "Camera/CameraComponent.h"

static FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.WeaponFire"));

void UDAbility_WeaponFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
    if (CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        if (ActorInfo->AbilitySystemComponent.IsValid())
        {
            DelegateHandle = ActorInfo->AbilitySystemComponent->GenericGameplayEventCallbacks
                .FindOrAdd(EventTag).AddUObject(this, &UDAbility_WeaponFire::OnTriggerEvent);
        }

        if (ActorInfo->OwnerActor.IsValid())
        {
            if (ADCharacterPlayer* PlayerShooter = Cast<ADCharacterPlayer>(ActorInfo->OwnerActor.Get()))
            {
                if (AShootWeapon* ActiveWeapon = PlayerShooter->GetActiveWeapon())
                {
                    FVector Location = PlayerShooter->TPCamera->GetComponentLocation();
                    FVector EndLocation = PlayerShooter->TPCamera->GetComponentRotation().RotateVector(Location * ActiveWeapon->TraceDistance);

                    //UDGameplayStatics::LineTraceAndSendEvent(ActorInfo->OwnerActor.Get(), EventTag, Location, EndLocation, );
                }
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UDAbility_WeaponFire::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag).Remove(DelegateHandle);
    }
}

void UDAbility_WeaponFire::OnTriggerEvent(const FGameplayEventData* EventData)
{
    ApplyGameplayEffectToAllActors(*EventData, UDGE_WeaponBaseDamage::StaticClass());
}
