// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameModeBase.h"
#include "DreamGameMode.generated.h"

UCLASS(minimalapi)
class ADreamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ADreamGameMode();

public:
	
	float GetPlayerResurrectionTime() const
	{
		return PlayerResurrectionTime;
	}

	int32 GetGameModeMaxPlayers() const
	{
		return MaxPlayers;
	}

	TSubclassOf<UUserWidget> GetSettlementWidgetClass() const
	{
		return SettlementWidget;
	}
	
	float GetSettlementWaitTime() const
	{
		return SettlementWaitTime;
	}

	void ReSpawnCharacter(ACharacter* Character);

public:

	
	UFUNCTION(BlueprintCallable, Category = DreamGameMode)
	void EndMatch();
	
public:

	static int32 DEFAULT_MAX_PLAYERS;

protected:

	UPROPERTY(EditAnywhere, Category = DreamGameMode)
	TSubclassOf<UUserWidget> SettlementWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DreamGameMode)
	float PlayerResurrectionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DreamGameMode)
	int32 MaxPlayers;

	/**
	 * 游戏结束时的等待时间，通常是到达这个时间时需要将玩家踢出世界
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DreamGameMode)
	float SettlementWaitTime;

protected:

	virtual void InitGameState() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	virtual bool GetPlayerRestartTransform(FTransform& RestartTransform);

private:

	void OnReSpawnCharacter(ACharacter* Character);

	void OnResetGame() const;

	void UpdateActivePlayers() const;

	uint16 CurrentPlayers;
};
