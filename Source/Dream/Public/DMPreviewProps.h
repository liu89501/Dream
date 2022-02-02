// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMPreviewActor.h"
#include "GameFramework/Actor.h"
#include "DMPreviewProps.generated.h"

UCLASS()
class DREAM_API ADMPreviewProps : public ADMPreviewActor
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = ItemPreview)
	AActor* AddPreviewActorFromClass(TSubclassOf<AActor> ActorClass);

};
