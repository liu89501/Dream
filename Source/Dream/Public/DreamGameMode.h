// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DreamGameMode.generated.h"

UCLASS(minimalapi)
class ADreamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ADreamGameMode();

public:

	
	UFUNCTION(BlueprintCallable, Category = DreamGameMode)
	void SetPlayerRespawnDelay(float NewRespawnDelay);

	UFUNCTION(BlueprintCallable, Category = DreamGameMode)
	void EndMatch();

	virtual void OnPlayerCharacterDie(APlayerController* PlayerController);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DreamGameMode)
	float DefaultPlayerRespawnDelay;

protected:

	virtual void HandleEndMatch();

	virtual void InitGameState() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

private:

	FTimerHandle Handle_PlayerRespawn;
};
