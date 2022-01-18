// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDropProps.h"
#include "DDropSkeletalMesh.generated.h"

UCLASS(Abstract)
class DREAM_API ADDropSkeletalMesh : public ADDropProps
{
	GENERATED_BODY()
	
public:

	ADDropSkeletalMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UNiagaraComponent* Effect;
	
protected:


	UFUNCTION()
    void OnSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

	UFUNCTION()
    void OnWake(UPrimitiveComponent* WakingComponent, FName BoneName);

};
