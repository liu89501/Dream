// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "DModuleBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDModuleBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Module")
	FEquipmentAttributes ModuleAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
	EModuleCategory Category;
};
