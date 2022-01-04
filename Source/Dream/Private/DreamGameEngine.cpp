// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameEngine.h"

UDreamGameEngine::UDreamGameEngine() : CriticalSection(nullptr)
{
}

void UDreamGameEngine::Init(IEngineLoop* InEngineLoop)
{

#if !UE_SERVER

	CriticalSection = new FSystemWideCriticalSection(TEXT("DreamApplication"));

	if (!CriticalSection->IsValid())
	{
		delete CriticalSection;
		FPlatformMisc::RequestExit(true);
	}

#endif 
	
	Super::Init(InEngineLoop);
}

void UDreamGameEngine::PreExit()
{
	Super::PreExit();

	if (CriticalSection)
	{
		delete CriticalSection;
	}
}
