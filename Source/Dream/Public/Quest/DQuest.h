// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DQuest.generated.h"

class UDQuestCondition;
class UItemData;
struct FQuestActionHandle;

UCLASS()
class DREAM_API UDQuest : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UDQuestCondition* Condition;
	
	UPROPERTY()
	UItemData* Reward;
	
public:

	void Update(const FQuestActionHandle& Handle) const;
	
};

