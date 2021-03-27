// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "DPlayerController.h"
#include "DreamType.h"
#include "UdpSocketReceiver.h"
#include "PlayerDataStoreType.h"
#include "GameFramework/GameModeBase.h"
#include "DreamGameMode.generated.h"

UCLASS(minimalapi)
class ADreamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ADreamGameMode();

public:
	
	EGameType GetGameType() const
	{
		return GameType;
	}

	float GetPlayerResurrectionTime() const
	{
		return PlayerResurrectionTime;
	}

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dream|Settings")
	EGameType GameType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dream|Settings")
	float PlayerResurrectionTime;
	
	UPROPERTY()
	bool bAutoShutdown;

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	void OnPlayerGetWeapons(const TArray<FPlayerWeapon>& AddedWeapons);

	virtual void GameCompleted();

private:

	FThreadSafeCounter PlayerNumCounter;

	float IdleTimeCount;
};
