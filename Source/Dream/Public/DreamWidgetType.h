// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "DreamWidgetType.generated.h"

UCLASS(BlueprintType)
class UItemViewData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category=ViewData, Meta = (ExposeOnSpawn))
	int64 ItemId;

	UPROPERTY(BlueprintReadOnly, Category=ViewData, Meta = (ExposeOnSpawn))
	UItemData* ItemData;
};


UCLASS(BlueprintType)
class UTaskViewData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category=ViewData, Meta = (ExposeOnSpawn))
	FTaskInformation Task;

	UFUNCTION(BlueprintCallable, Category=ViewData)
	void ToggleTaskTracking();
};

UCLASS(BlueprintType)
class UStoreItemViewData : public UItemViewData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category=ViewData, Meta = (ExposeOnSpawn))
	TArray<FAcquisitionCost> Costs;

};