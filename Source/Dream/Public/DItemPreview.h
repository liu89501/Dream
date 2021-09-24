// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DItemPreview.generated.h"

UCLASS()
class DREAM_API ADItemPreview : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADItemPreview();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* PreviewActorRoot;

public:
	
	UFUNCTION(BlueprintCallable, Category = ItemPreview)
    USkeletalMeshComponent* AddPreviewSkeletalMeshFromComponent(USkeletalMeshComponent* CharacterMesh);
	
	UFUNCTION(BlueprintCallable, Category = ItemPreview)
    AActor* AddPreviewActorFromClass(TSubclassOf<AActor> ActorClass);
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ProcessMeshComponents(UMeshComponent* PreviewBody, UMeshComponent* Mesh);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
