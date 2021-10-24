// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void SetMagazineBrush(const FSlateBrush& NewMagazineBrush);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void SetMagazineInfo(int NewAmmoNum, int NewTotalAmmo, float AmmoPercent);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void SetCrossHairVisibility(ESlateVisibility InVisibility);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void ShowHitEnemyTips(bool bKilledEnemy);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void SetCrossHairBrush(const FSlateBrush& InSlateBrush);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void SetHealth(float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void AddBuffIcon(const FGameplayTag& EffectOwningTag, UTexture2D* Icon, int32 StackCount, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void RefreshBuffIcon(const FGameplayTag& EffectOwningTag, int32 StackCount, float Duration);
	
	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void RemoveBuffIcon(const FGameplayTag& EffectOwningTag);

	UFUNCTION(BlueprintImplementableEvent, Category=PlayerHUD)
	void AddInteractiveUI(UWidget* Widget);

	UFUNCTION(BlueprintCallable, Meta = (DisplayName="AppendToInteractiveUI", ScriptName="AppendToInteractiveUI"), Category=PlayerHUD)
    void BP_AddInteractiveUI(UWidget* Widget);
};
