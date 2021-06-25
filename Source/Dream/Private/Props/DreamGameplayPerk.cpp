// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameplayPerk.h"
#include "AbilitySystemComponent.h"
#include "DCharacterBase.h"

UDreamGameplayPerk::UDreamGameplayPerk()
{
}

AActor* UDreamGameplayPerk::SpawnActor(TSubclassOf<AActor> ActorClass, const FVector& Location,
	ESpawnActorCollisionHandlingMethod CollisionOverride, const FActivationOpportunityParams& OpportunityParams)
{
	ADCharacterBase* OwnerCharacter = Cast<ADCharacterBase>(OpportunityParams.SourceComponent->GetOwnerActor());
	if (OwnerCharacter == nullptr)
	{
		return nullptr;
	}
	
	if (UWorld* World = GEngine->GetWorldFromContextObject(OwnerCharacter, EGetWorldErrorMode::ReturnNull))
	{
		FActorSpawnParameters Parameters;
		Parameters.Instigator = OwnerCharacter;
		Parameters.Owner = OwnerCharacter;
		Parameters.SpawnCollisionHandlingOverride = CollisionOverride;
		return World->SpawnActor<AActor>(ActorClass, Location, FRotator::ZeroRotator, Parameters);
	}
	return nullptr;
}

FActiveGameplayEffectHandle UDreamGameplayPerk::ActivationPerkEffect_Implementation(const FActivationOpportunityParams& OpportunityParams)
{
	if (PerkEffect && OpportunityParams.IsValid())
	{
		if (OpportunityParams.SourceComponent->IsOwnerActorAuthoritative())
		{
			UGameplayEffect* GE = PerkEffect->GetDefaultObject<UGameplayEffect>();
			if (ApplyTarget == EApplyTarget::Source)
			{
				return OpportunityParams.SourceComponent->ApplyGameplayEffectToSelf(GE, INDEX_NONE, OpportunityParams.SourceComponent->MakeEffectContext());
			}
		
			if (OpportunityParams.TargetComponent != nullptr)
			{
				return OpportunityParams.SourceComponent->ApplyGameplayEffectToTarget(GE, OpportunityParams.TargetComponent);
			}
		}
	}

	return FActiveGameplayEffectHandle();
}

