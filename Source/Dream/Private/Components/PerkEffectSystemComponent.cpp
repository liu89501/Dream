// Fill out your copyright notice in the Description page of Project Settings.


#include "PerkEffectSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DreamGameplayPerk.h"

FActivationOpportunityParams::FActivationOpportunityParams(AActor* Source, AActor* Target)
{
	SourceComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Source);
	TargetComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
}

// Sets default values for this component's properties
UPerkEffectSystemComponent::UPerkEffectSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicated(false);
}

void UPerkEffectSystemComponent::ActivationOpportunity(EOpportunity Opportunity, const FActivationOpportunityParams& OpportunityParams)
{
	/*if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}*/

	TArray<TMap<EOpportunity, TArray<UDreamGameplayPerk*>>> ValueArray;
	AppliedPerks.GenerateValueArray(ValueArray);
	
	for (TPair<EPerkChannel, TMap<EOpportunity, TArray<UDreamGameplayPerk*>>> Pair : AppliedPerks)
	{
		TArray<UDreamGameplayPerk*>* Perks = Pair.Value.Find(Opportunity);
		if (Perks == nullptr)
		{
			continue;
		}

		for (UDreamGameplayPerk* Perk : *Perks)
		{
			if (Perk == nullptr)
			{
				continue;
			}

			FActiveGameplayEffectHandle ActivationPerkEffect = Perk->ActivationPerkEffect(OpportunityParams);
			if (ActivationPerkEffect.IsValid())
			{
				ActivatedEffects.FindOrAdd(Pair.Key).Add(ActivationPerkEffect);
			}
		}
	}
}

void UPerkEffectSystemComponent::ApplyPerks(const TArray<TSubclassOf<UDreamGameplayPerk>>& Perks, EPerkChannel PerkChannel)
{
	ClearPerks(PerkChannel);

	bool bIsAuthority = GetOwnerRole() == ROLE_Authority;
	bool bIsLocal = GetOwnerRole() == ROLE_AutonomousProxy;

	for (TSubclassOf<UDreamGameplayPerk> Perk : Perks)
	{
		if (!Perk)
		{
			continue;
		}

		UDreamGameplayPerk* GameplayPerkCDO = Perk->GetDefaultObject<UDreamGameplayPerk>();
		if ((bIsLocal && GameplayPerkCDO->bLocalRunning) || (bIsAuthority && !GameplayPerkCDO->bLocalRunning))
		{
			TMap<EOpportunity, TArray<UDreamGameplayPerk*>>& PerkMap = AppliedPerks.FindOrAdd(PerkChannel);
			PerkMap.FindOrAdd(GameplayPerkCDO->Opportunity).AddUnique(GameplayPerkCDO);
		}
	}

	ActivationOpportunity(EOpportunity::Immediately, FActivationOpportunityParams (GetOwner(), GetOwner()));
}

void UPerkEffectSystemComponent::ClearPerks(EPerkChannel PerkChannel)
{
	if (ActivatedEffects.Num() > 0)
	{
		UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

		for (TPair<EPerkChannel, TArray<FActiveGameplayEffectHandle>> Effect : ActivatedEffects)
		{
			if (Effect.Key == PerkChannel)
			{
				for (FActiveGameplayEffectHandle Handle : Effect.Value)
				{
					AbilitySystem->RemoveActiveGameplayEffect(Handle);
				}

				break;
			}
		}

		ActivatedEffects.Remove(PerkChannel);
	}
	
	AppliedPerks.Remove(PerkChannel);
}


// Called when the game starts
void UPerkEffectSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPerkEffectSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

