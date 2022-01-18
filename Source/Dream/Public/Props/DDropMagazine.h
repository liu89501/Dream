// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDropStaticMesh.h"
#include "DDropMagazine.generated.h"

enum class EAmmoType : uint8;

UCLASS(Abstract)
class DREAM_API ADDropMagazine : public ADDropStaticMesh
{
	GENERATED_BODY()

public:
	
	ADDropMagazine();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=DropMagazine)
	EAmmoType AmmoType;

protected:

	virtual void BeginPlay() override;
};

