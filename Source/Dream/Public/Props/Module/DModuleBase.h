// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
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

	UPROPERTY(BlueprintReadOnly, Category = "Module")
	FEquipmentAttributes ModuleAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
	EModuleCategory Category;

	virtual const FPropsInfo& GetPropsInfo() const override;
	
	virtual ERewardNotifyMode GetRewardNotifyMode() const override;
};
