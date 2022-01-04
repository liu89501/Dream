// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PropsInterface.h"
#include "DMaterial.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDMaterial : public UObject, public IPropsInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPropsInfo PropsInformation;

	virtual const FPropsInfo& GetPropsInfo() const override;
	
	virtual ERewardNotifyMode GetRewardNotifyMode() const override;
};
