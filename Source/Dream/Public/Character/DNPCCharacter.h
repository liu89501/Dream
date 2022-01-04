// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "IconInterface.h"
#include "GameFramework/Character.h"
#include "DNPCCharacter.generated.h"

UCLASS()
class DREAM_API ADNPCCharacter : public AActor, public IIconInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADNPCCharacter();

	UPROPERTY(VisibleAnywhere)
	class UIconComponent* IconComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual UIconComponent* GetIconComponent() const override;

};
