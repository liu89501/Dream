// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamGameMode.h"
#include "DGameModeInLevel.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADGameModeInLevel : public ADreamGameMode
{
	GENERATED_BODY()

public:

	ADGameModeInLevel();

public:

	UFUNCTION(BlueprintCallable, Category=DGameMode)
	void SetProgressTag(FName InProgressTag);


	float GetDefaultSettlementDelay() const
	{
		return DefaultSettlementDelay;
	}

	TSubclassOf<UUserWidget> GetSettlementWidgetClass() const
	{
		return SettlementWidgetClass;
	}

	TSubclassOf<UUserWidget> GetGameOverWidgetClass() const
	{
		return GameOverWidgetClass;
	}

protected:

	virtual void OnPlayerCharacterDie(APlayerController* PlayerController) override;

	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	
	virtual void HandleEndMatch() override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	virtual void Logout(AController* Exiting) override;

	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

private:

	void OnResetGame();

private:

	UPROPERTY(EditAnywhere, Category = DreamGameMode)
	TSubclassOf<UUserWidget> SettlementWidgetClass;

	UPROPERTY(EditAnywhere, Category = DreamGameMode)
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	/**
	* 游戏结束时的等待时间，通常是到达这个时间时需要将玩家踢出世界
	*/
	UPROPERTY(EditAnywhere, Category = DreamGameMode)
	float DefaultSettlementDelay;

private:

	int32 SurvivingPlayers;

	int32 StartSpotIndex;

	/** 当前玩家进行到的进度 */
	FName LevelProgressTag;
	
	UPROPERTY()
	TArray<class APlayerStart*> AvailableStartSpot;
};

