// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BroadcastReceiverComponent.generated.h"

USTRUCT(BlueprintType)
struct FTargetDelegate
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	AActor* Target;

	UPROPERTY(EditInstanceOnly)
	FName DelegateName;

	UPROPERTY(EditInstanceOnly)
	FName TriggerFunctionName;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAM_API UBroadcastReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBroadcastReceiverComponent();

	UPROPERTY(EditInstanceOnly, Category=BroadcastReceiver)
	TArray<FTargetDelegate> Descriptions;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
