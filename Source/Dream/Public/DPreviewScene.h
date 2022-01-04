// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DPreviewScene.generated.h"

UCLASS(Abstract)
class DREAM_API ADPreviewScene : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADPreviewScene();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* PreviewActorPoint;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneActor;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
