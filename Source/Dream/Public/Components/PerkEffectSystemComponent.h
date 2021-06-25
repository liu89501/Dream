// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameplayEffectTypes.h"
#include "DreamType.h"
#include "Components/ActorComponent.h"
#include "PerkEffectSystemComponent.generated.h"

class UDreamGameplayPerk;

UENUM()
enum class EPerkChannel : uint8
{
	Weapon,
	Module
};

USTRUCT(BlueprintType)
struct FActivationOpportunityParams
{
	GENERATED_BODY()

	FActivationOpportunityParams() = default;

	FActivationOpportunityParams(UAbilitySystemComponent* InSourceComponent, UAbilitySystemComponent* InTargetComponent)
		: SourceComponent(InSourceComponent),
		  TargetComponent(InTargetComponent)
	{
	}
	
	FActivationOpportunityParams(AActor* Source, AActor* Target);

	/** 必须不能为空 */
	UPROPERTY(BlueprintReadOnly, Category=PerkEffect)
	class UAbilitySystemComponent* SourceComponent;

	/** 可能为空 (GE应用到自身时)  */
	UPROPERTY(BlueprintReadOnly, Category=PerkEffect)
	class UAbilitySystemComponent* TargetComponent;

	bool IsValid() const
	{
		return SourceComponent != nullptr;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAM_API UPerkEffectSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPerkEffectSystemComponent();

	void ActivationOpportunity(EOpportunity Opportunity, const FActivationOpportunityParams& OpportunityParams);

	void ApplyPerks(const TArray<TSubclassOf<UDreamGameplayPerk>>& Perks, EPerkChannel PerkChannel = EPerkChannel::Weapon);

	void ClearPerks(EPerkChannel PerkChannel = EPerkChannel::Weapon);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	TMap<EPerkChannel, TArray<FActiveGameplayEffectHandle>> ActivatedEffects;

	TMap<EPerkChannel, TMap<EOpportunity, TArray<UDreamGameplayPerk*>>> AppliedPerks;
		
};
