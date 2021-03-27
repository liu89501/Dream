// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DreamDropProps.generated.h"

UCLASS()
class DREAM_API ADreamDropProps : public AActor
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	ADreamDropProps();
	
public:	
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
