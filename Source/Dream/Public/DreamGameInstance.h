// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DreamGameInstance.generated.h"


/**
 *
 */
UCLASS()
class DREAM_API UDreamGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UDreamGameInstance();

protected:

	virtual void Init() override;

	virtual void OnStart() override;

	virtual void Shutdown() override;

	void OnPDIConnectionLose();

};