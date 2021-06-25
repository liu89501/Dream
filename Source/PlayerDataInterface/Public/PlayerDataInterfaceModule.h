#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPlayerDataInterface;

class FPlayerDataInterfaceModule : public IModuleInterface
{

public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	PLAYERDATAINTERFACE_API static FPlayerDataInterface* Get();
	
private:

	static FPlayerDataInterface* Singleton;
};