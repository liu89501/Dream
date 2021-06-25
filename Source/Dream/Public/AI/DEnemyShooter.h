// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/DEnemyBase.h"
#include "DEnemyShooter.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADEnemyShooter : public ADEnemyBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = AISettings)
	UPrimitiveComponent* WeaponMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	FName BlackboardName_CanOpenFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	FName WeaponMuzzleSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	float OpenFireRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	UAnimMontage* FireAnim;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	FWeaponTrailVFX TrailVfx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	class USoundCue* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AISettings|Abilities")
	FGameplayTagContainer FireAbilityTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AISettings|Abilities")
	FGameplayTag FireEventTag;

protected:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "EnemyAI|Shooter")
	bool bDiscoverEnemy;

public:

	UFUNCTION(BlueprintCallable, Category = "EnemyAI|Shooter")
    void OpenFire(const FVector& TargetLocation);


protected:

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, reliable)
    void NetMulticastOpenFire(const FVector_NetQuantize& TargetLocation);
    
	virtual void OnTargetPerceptionUpdated(ADCharacterBase* StimulusPawn, FAIStimulus Stimulus) override;

	virtual void LostAllHostileTarget() override;
};
