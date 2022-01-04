// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "DPlayerTriggerBox.generated.h"

UENUM()
enum class ETriggerMode : uint8
{
	ServerOnly,
	LocalOnly,
	ServerAndLocal
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrigger);

/**
 *  只触发一次的触发器
 */
UCLASS()
class DREAM_API ADPlayerTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:

	ADPlayerTriggerBox();

	UPROPERTY(EditInstanceOnly, Category=DreamTriggerBox)
	ETriggerMode TriggerMode;

	UPROPERTY(BlueprintAssignable)
	FOnTrigger OnTrigger;

public:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	                    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                    const FHitResult& SweepResult);

private:

	bool bTriggered;
};
