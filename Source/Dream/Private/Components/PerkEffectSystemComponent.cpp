// Fill out your copyright notice in the Description page of Project Settings.


#include "PerkEffectSystemComponent.h"



#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "DreamGameplayPerk.h"

// Sets default values for this component's properties
UPerkEffectSystemComponent::UPerkEffectSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	//SetIsReplicated(true);
	// ...
}

void UPerkEffectSystemComponent::ActivationTimeFrame(ETimeFrame TimeFrame)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerActivationTimeFrame(TimeFrame);
		return;
	}
	
	TArray<UDreamGameplayPerk*>* GameplayPerks = AppliedPerks.Find(TimeFrame);
	if (GameplayPerks == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (AbilitySystemComponent == nullptr)
	{
		DREAM_NLOG(Warning, TEXT("AbilitySystemComponent Invalid"));
		return;
	}

	for (UDreamGameplayPerk* Perk : *GameplayPerks)
	{
		if (Perk == nullptr)
		{
			continue;
		}

		FActiveGameplayEffectHandle ActivationPerkEffect = Perk->ActivationPerkEffect(AbilitySystemComponent);
		if (ActivationPerkEffect.IsValid())
		{
			ActivatedEffects.Add(ActivationPerkEffect);
		}
	}
}

void UPerkEffectSystemComponent::ServerActivationTimeFrame_Implementation(ETimeFrame TimeFrame)
{
	ActivationTimeFrame(TimeFrame);
}

void UPerkEffectSystemComponent::ApplyPerks(const TArray<TSubclassOf<UDreamGameplayPerk>>& Perks)
{
	AppliedPerks.Reset();

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	if (ActivatedEffects.Num() > 0)
	{
		for (FActiveGameplayEffectHandle Handle : ActivatedEffects)
		{
			AbilitySystem->RemoveActiveGameplayEffect(Handle);
		}

		ActivatedEffects.Reset();
	}

	for (TSubclassOf<UDreamGameplayPerk> Perk : Perks)
	{
		if (!Perk)
		{
			continue;
		}
		
		UDreamGameplayPerk* GameplayPerkCDO = Perk->GetDefaultObject<UDreamGameplayPerk>();
		AppliedPerks.FindOrAdd(GameplayPerkCDO->TimeFrame).Add(GameplayPerkCDO);
	}

	ActivationTimeFrame(ETimeFrame::Immediately);
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

