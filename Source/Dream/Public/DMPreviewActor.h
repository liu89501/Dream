// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DMPreviewActor.generated.h"

UCLASS(Abstract)
class DREAM_API ADMPreviewActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	ADMPreviewActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* Scene;

};
