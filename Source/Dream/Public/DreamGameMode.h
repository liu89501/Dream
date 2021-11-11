// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "DreamType.h"
#include "UdpSocketReceiver.h"
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

	int32 GetGameModeMaxPlayers() const
	{
		return MaxPlayers;
	}


public:

	static int32 DEFAULT_MAX_PLAYERS;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DreamGameMode)
	EGameType GameType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DreamGameMode)
	float PlayerResurrectionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DreamGameMode)
	int32 MaxPlayers;
	
	UPROPERTY()
	bool bAutoShutdown;

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

private:

	FThreadSafeCounter PlayerNumCounter;

	float IdleTimeCount;
};
