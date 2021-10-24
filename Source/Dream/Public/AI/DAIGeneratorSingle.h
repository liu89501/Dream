// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DAIGeneratorBase.h"
#include "DAIGeneratorSingle.generated.h"

UCLASS()
class DREAM_API ADAIGeneratorSingle : public ADAIGeneratorBase
{
	GENERATED_BODY()

	ADAIGeneratorSingle();

public:

	/**
	 *  Value 为生成的AI个数
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AIGenerator)
	TSubclassOf<class ADEnemyBase> GenerateAIClass;

protected:

	virtual void ProcessAIGenerate() override;

};
