// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameplayEffectTypes.h"
#include "DreamType.h"
#include "Components/ActorComponent.h"
#include "PerkEffectSystemComponent.generated.h"

class UDreamGameplayPerk;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAM_API UPerkEffectSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPerkEffectSystemComponent();

	void ActivationTimeFrame(ETimeFrame TimeFrame);

	UFUNCTION(Server, Reliable)
	void ServerActivationTimeFrame(ETimeFrame TimeFrame);

	void ApplyPerks(const TArray<TSubclassOf<UDreamGameplayPerk>>& Perks);

	void ClearPerks();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	TArray<FActiveGameplayEffectHandle> ActivatedEffects;

	TMap<ETimeFrame, TArray<UDreamGameplayPerk*>> AppliedPerks;
		
};
