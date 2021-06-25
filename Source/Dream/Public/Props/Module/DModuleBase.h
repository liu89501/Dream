// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "DPropsType.h"
#include "DreamType.h"
#include "PropsInterface.h"
#include "DModuleBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDModuleBase : public UObject, public IPropsInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
	FPropsInfo PropsInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
	FEquipmentAttributes ModuleAttributes;

	virtual const FPropsInfo& GetPropsInfo() const override;

	virtual const FEquipmentAttributes& GetEquipmentAttributes() const override;
};
