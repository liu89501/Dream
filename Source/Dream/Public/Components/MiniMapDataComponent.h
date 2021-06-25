// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DreamType.h"
#include "Components/ActorComponent.h"
#include "MiniMapDataComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAM_API UMiniMapDataComponent : public UActorComponent
{
	GENERATED_BODY()

	UMiniMapDataComponent();

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniMapData")
	EMiniMapDrawType DrawType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MiniMapData")
	FSlateBrush DefaultSprite;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MiniMapData")
	FSlateBrush OverflowSprite;

public:

	FMiniMapData CalculationPosition(const FVector& TargetLocation, const FRotator& TargetRotation, float ScanRange) const;
	
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
