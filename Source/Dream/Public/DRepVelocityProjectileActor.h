// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DRepVelocityProjectileActor.generated.h"

UCLASS()
class DREAM_API ADRepVelocityProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADRepVelocityProjectileActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* ProjectileMovement;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
