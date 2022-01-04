// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DItemPreview.h"
#include "GameFramework/Actor.h"
#include "DPropsPreview.generated.h"

UCLASS()
class DREAM_API ADPropsPreview : public ADItemPreview
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = ItemPreview)
	AActor* AddPreviewActorFromClass(TSubclassOf<AActor> ActorClass);

};
