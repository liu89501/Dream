// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DAIGenerator.generated.h"

UCLASS()
class DREAM_API ADAIGenerator : public AActor
{
	GENERATED_BODY()

	ADAIGenerator();

public:	

	UPROPERTY(VisibleAnywhere)
	class UBillboardComponent* Billboard;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* GeneratorRangeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AIGenerator)
	TArray<TSubclassOf<class ADEnemyBase>> GenerateAIClass;

	/**
	 * 重置时的等待时间
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AIGenerator)
	float ResetWaitTime;

public:

	void AIDeathCount();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ResetGenerator();

private:
	
	FThreadSafeCounter ActiveAICounter;

};
