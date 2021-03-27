// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEnemyBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "Components/ActorComponent.h"
#include "DAIModuleComponent.generated.h"

class ADCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName="BaseAIModule", ScriptName = "BaseAIModule") )
class DREAM_API UDAIModuleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDAIModuleComponent();

	UPROPERTY(EditDefaultsOnly, Category = AIModule)
	FName BlackboardName_TargetPawn;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void ActivateHostile(ADCharacterBase* Hostile);

	virtual void RefreshActiveHostile();

	virtual void LostAllHostileTarget();

	FORCEINLINE ADEnemyBase* GetOwnerAI() const 
	{
		return Cast<ADEnemyBase>(GetOwner());
	}
    
	UFUNCTION()
    virtual void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

	void HostileTargetDestroy(ADCharacterBase* DestroyedActor) const;

public:

	/*UFUNCTION(BlueprintImplementableEvent, Category=AIModule, meta = (DisplayName = "OnPerceptionUpdated", ScriptName = "OnPerceptionUpdated"))
    void BP_OnPerceptionUpdated(ADCharacterBase* Target, FAIStimulus Stimulus, class AAIController* AIC);

	UFUNCTION(BlueprintImplementableEvent, Category=AIModule, meta = (DisplayName = "OnDiscoverHostileTarget", ScriptName = "OnDiscoverHostileTarget"))
    void BP_OnDiscoverHostileTarget();*/
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	UPROPERTY()
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY()
	class AAIController* AIController;
};
