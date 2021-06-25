// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_StatusManager.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UBTService_StatusManager : public UBTService
{
	GENERATED_BODY()

public:

	/** 地方目标的黑板key */
	UPROPERTY(EditAnywhere, Category=StatusManager)
	struct FBlackboardKeySelector NearDeathSelector;
	
	UPROPERTY(EditAnywhere, Category=StatusManager)
	struct FBlackboardKeySelector TargetPawnSelector;

	/** 与目标Actor低于这个距离时 */
	UPROPERTY(EditAnywhere, Category=StatusManager)
	float ThresholdForWalking;

protected:

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
