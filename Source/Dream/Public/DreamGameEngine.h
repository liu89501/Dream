// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "DreamGameEngine.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDreamGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:

	UDreamGameEngine();

	virtual void Init(IEngineLoop* InEngineLoop) override;

	virtual void PreExit() override;

private:

	FSystemWideCriticalSection* CriticalSection;
};
