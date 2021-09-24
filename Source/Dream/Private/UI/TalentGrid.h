// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GridPanel.h"
#include "TalentGrid.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTalentItemLearnedCompleted, class UTalentItem*, TalentItem);

/**
 * 
 */
UCLASS()
class UTalentGrid : public UGridPanel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FTalentItemLearnedCompleted LearnedCompleted;

	UFUNCTION(BlueprintCallable, Category=TalentGrid)
	void InitialTalentGrid(const TArray<struct FTalentInfo>& Talents);

	UFUNCTION(BlueprintCallable, Category=TalentGrid)
	void RefreshTalents();

	UFUNCTION(BlueprintCallable, Category=TalentGrid)
	void GetLearnedTalents(TArray<int32>& TalentIdArray, TArray<TSubclassOf<class UDreamGameplayAbility>>& TalentClasses) const;

	UFUNCTION(BlueprintCallable, Category=TalentGrid)
	void ResetAllTalents();

public:

	virtual void OnWidgetRebuilt() override;

	FTalentItemLearnedCompleted& GetLearnedCompleted()
	{
		return LearnedCompleted;
	}

private:

	UPROPERTY()
	TArray<class UTalentItem*> TalentWidgets;
	
};
