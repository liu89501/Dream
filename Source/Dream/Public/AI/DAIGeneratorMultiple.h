// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DAIGeneratorBase.h"
#include "DAIGeneratorMultiple.generated.h"

UCLASS()
class DREAM_API ADAIGeneratorMultiple : public ADAIGeneratorBase
{
	GENERATED_BODY()

	ADAIGeneratorMultiple();

public:

	UPROPERTY(VisibleDefaultsOnly)
	class USplineComponent* GeneratePath;

	/**
	 *  Value 为生成的AI个数
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=AIGenerator)
	TMap<TSubclassOf<class ADEnemyBase>, int32> GenerateAIClass;

protected:

	virtual void ProcessAIGenerate() override;

};
