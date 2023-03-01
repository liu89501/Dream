// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "DreamGameSession.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADreamGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	virtual void RegisterServer() override;

	void OnLoginCallback(bool bSuccessfully);

	void OnCreatedSession(FName InSessionName, bool bWasSuccessful);

private:

	FDelegateHandle Handle_Login;
	FDelegateHandle Handle_CreateSession;

};

