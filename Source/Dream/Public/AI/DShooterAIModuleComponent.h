// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "AI/DAIModuleComponent.h"
#include "DShooterAIModuleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatStatusChange, bool, NewStatus);

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta = (DisplayName = "AIModuleShoote", ScriptName = "AIModuleShoote", BlueprintSpawnableComponent))
class DREAM_API UDShooterAIModuleComponent : public UDAIModuleComponent
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    FName BlackboardName_CanFire;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    FName WeaponMeshTagName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    FName WeaponSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    UAnimMontage* FireAnim;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    class USoundCue* FireSound;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    class UNiagaraSystem* TrailParticles;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    FName TrailParamName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    class UParticleSystem* MuzzleFlash;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AIModule)
    FVector MuzzleOffset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AIModule|Abilities")
    FGameplayTagContainer FireAbilityTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AIModule|Abilities")
    FGameplayTag FireEventTag;

    UPROPERTY(BlueprintAssignable, Category="AIModule|Event")
    FCombatStatusChange OnCombatStatusChange;

    UPROPERTY(BlueprintReadOnly, Category = AIModule)
    UPrimitiveComponent* WeaponMesh;
public:

    UFUNCTION(BlueprintCallable, Category = "AIModule|Shooter")
    void OpenFire(const FVector& TargetLocation);
    UFUNCTION(NetMulticast, reliable)
    void NetMulticastOpenFire(const FVector_NetQuantize100& TargetLocation);

protected:

    
    virtual void OnRegister() override;

    virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;

    virtual void LostAllHostileTarget() override;
};

