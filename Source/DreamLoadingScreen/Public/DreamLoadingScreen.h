// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ModuleInterface.h"
#include "ModuleManager.h"

/** Module interface for this game's loading screens */
class IDreamLoadingScreenModule : public IModuleInterface
{
public:

	static IDreamLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IDreamLoadingScreenModule>(TEXT("DreamLoadingScreen"));
	}

	virtual TSharedRef<class SWidget> NewLoadingScreen() = 0;
	

	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen() = 0;
};

