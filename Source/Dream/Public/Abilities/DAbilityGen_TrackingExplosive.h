// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DAbilityGen_TrackingExplosive.generated.h"

UCLASS()
class DREAM_API ADAbilityGen_TrackingExplosive : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADAbilityGen_TrackingExplosive();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AbilityGen)
	class UDProjectileComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AbilityGen)
	class USphereComponent* SphereCollision;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnImpact(const FHitResult& ImpactResult);

	UFUNCTION()
	void OnRep_Impact();

	void OnAcceleration() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn))
	AActor* TargetActor;
	

	UPROPERTY(EditDefaultsOnly, Category=AbilityGen)
	float ImpactRadius;
	
	UPROPERTY(EditDefaultsOnly, Category=AbilityGen)
	float StartAccelerationTime;

	UPROPERTY(EditDefaultsOnly, Category=AbilityGen)
	UParticleSystem* ExplodeParticle;
	
	UPROPERTY(EditDefaultsOnly, Category=AbilityGen)
	class USoundCue* ExplodeSound;

	UPROPERTY(EditDefaultsOnly, Category=AbilityGen)
	TSubclassOf<class UGameplayEffect> ApplyEffect;

private:

	UPROPERTY(ReplicatedUsing=OnRep_Impact)
	bool bImpact;
};

