// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameInstance.h"

#if WITH_EDITOR

#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"

#endif

UDreamGameInstance::UDreamGameInstance()
{
}

void UDreamGameInstance::Init()
{
	Super::Init();
	
#if WITH_EDITOR

	FPDIStatic::Get()->Login();
	
#endif
}

void UDreamGameInstance::OnStart()
{
	Super::OnStart();
}

void UDreamGameInstance::Shutdown()
{
	Super::Shutdown();
}
