// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PropsInterface.h"
#include "DExperience.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDExperience : public UObject, public IPropsInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPropsInfo PropsInformation;

	virtual const FPropsInfo& GetPropsInfo() const override;

	virtual ERewardNotifyMode GetRewardNotifyMode() const override;
};
