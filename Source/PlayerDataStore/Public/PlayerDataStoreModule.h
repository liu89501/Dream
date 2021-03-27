#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPlayerDataStore;

class FPlayerDataStoreModule : public IModuleInterface
{

public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	PLAYERDATASTORE_API static FPlayerDataStore* Get();
	
private:

	static FPlayerDataStore* Singleton;
};