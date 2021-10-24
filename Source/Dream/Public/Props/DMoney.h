// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PropsInterface.h"
#include "DMoney.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDMoney : public UObject, public IPropsInterface
{
	GENERATED_BODY()

public:

	UDMoney();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPropsInfo PropsInformation;

	virtual const FPropsInfo& GetPropsInfo() const override;
	
	virtual ERewardNotifyMode GetRewardNotifyMode() const override;
};
