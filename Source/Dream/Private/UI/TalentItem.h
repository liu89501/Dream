// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserWidget.h"
#include "PlayerDataInterfaceType.h"
#include "TalentItem.generated.h"

/**
 * 
 */
UCLASS()
class UTalentItem : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Talent)
	int32 TalentIndex;
	
	UPROPERTY(BlueprintReadOnly, Category=Talent)
	bool bLearnable;

	UPROPERTY(BlueprintReadWrite, Category=Talent)
	FTalentInfo TalentInfo;

    void InitialTalentItem(const FTalentInfo& Info);

	void NativeRefreshTalentItem(bool bInLearnable);

	UFUNCTION(BlueprintCallable, Category=Talent)
	void LearningTalent();

	UFUNCTION(BlueprintCallable, Category=Talent)
	bool IsLearnedTalent() const;
	
protected:

	UFUNCTION(BlueprintImplementableEvent)
	void InitialTalentWidget();

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshTalentItem();
};
