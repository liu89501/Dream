// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Dream.h"
#include "DMProjectSettings.h"
#include "PlayerDataInterfaceStatic.h"
#include "Modules/ModuleManager.h"
#include "UI/Style/DreamStyle.h"

class FDreamModule : public FDefaultGameModuleImpl
{
	
public:

	virtual void StartupModule() override
	{
		
#if !UE_SERVER
		
		FDreamStyle::Initialize();
		
#endif

		GSProject = NewObject<UDMProjectSettingsInstance>(GetTransientPackage());
		GSProject->Initialize();
		
		FPDIStatic::Initialize();		
	}
	
	virtual void ShutdownModule() override
	{

#if !UE_SERVER

		FDreamStyle::Shutdown();

#endif
		
		FPDIStatic::Shutdown();
	}
	
};

IMPLEMENT_PRIMARY_GAME_MODULE( FDreamModule, Dream, "Dream" );
 