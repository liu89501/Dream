// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DreamType.h"
#include "PropsInterface.h"
#include "UObject/NoExportTypes.h"
#include "DModuleBase.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDModuleBase : public UObject, public IPropsInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FPropsInfo PropsInfo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TArray<TSubclassOf<class UDreamGameplayPerk>> ModulePerks;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Attribute")
	float CriticalRate;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Attribute")
	float CriticalDamage;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Attribute")
	float Defense;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Attribute")
	float MoveSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Attribute")
	float DamageReduction;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Attribute")
	float Penetration;

	virtual FPropsInfo GetPropsInfo_Implementation() const override;
	
};
