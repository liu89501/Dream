// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DAIGeneratorBase.h"
#include "DAIGeneratorAssociated.generated.h"

UCLASS(Abstract)
class DREAM_API ADAIGeneratorAssociated : public ADAIGeneratorBase
{
	GENERATED_BODY()

	ADAIGeneratorAssociated();

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=AIGenerator)
	class UBroadcastReceiverComponent* BroadcastReceiver;

	/**
	 *  Value 为生成的AI个数
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AIGenerator)
	TArray<class ADAIGeneratorUnit*> AIInstance;

protected:

	virtual int32 ProcessAIGenerate() override;

	virtual void Destroyed() override;

	
#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

};
