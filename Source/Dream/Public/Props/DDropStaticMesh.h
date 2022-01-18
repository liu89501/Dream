// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDropProps.h"
#include "DDropStaticMesh.generated.h"

UCLASS(Abstract)
class DREAM_API ADDropStaticMesh : public ADDropProps
{
	GENERATED_BODY()

public:

	ADDropStaticMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USphereComponent* TriggerSphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UNiagaraComponent* IndicationEffect;
	
protected:

	UFUNCTION()
    void OnSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

	UFUNCTION()
    void OnWake(UPrimitiveComponent* WakingComponent, FName BoneName);

};
