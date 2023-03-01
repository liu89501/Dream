// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamDamageCalculation.h"
#include "Components/ActorComponent.h"
#include "DMShooterWeaponComponent.generated.h"


UCLASS( Abstract, ClassGroup=(Custom) )
class DREAM_API UDMShooterWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UDMShooterWeaponComponent();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=ShooterWeapon)
	virtual void OpenFire(AActor* Hostile);

protected:

	virtual void BeginPlay() override;

	void PlayAnimAndFX();

public:

	UPROPERTY(EditDefaultsOnly, Category=AIWeaponSettings)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category=AIWeaponSettings)
	EDDamageType DamageType;

protected:

	UPROPERTY()
	class ADEnemyShooter* OwnerAI;
		
};
