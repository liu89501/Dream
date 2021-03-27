#include "PlayerDataStoreModule.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreType.h"
#include "OnlineSubsystem.h"

FPlayerDataStore* FPlayerDataStoreModule::Singleton = nullptr;

void FPlayerDataStoreModule::StartupModule()
{
	if (Singleton == nullptr)
	{
		if (IOnlineSubsystem* OSSystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
		{
			Singleton = new FPlayerDataStore(OSSystem);
		}
	}
}

void FPlayerDataStoreModule::ShutdownModule()
{
	delete Singleton;
	Singleton = nullptr;
}

FPlayerDataStore* FPlayerDataStoreModule::Get()
{
	
#if WITH_EDITOR
	if (Singleton == nullptr)
	{
		if (IOnlineSubsystem* OSSystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
		{
			Singleton = new FPlayerDataStore(OSSystem);
		}
	}
#endif
	
	return Singleton;
}

IMPLEMENT_MODULE(FPlayerDataStoreModule, PlayerDataStore)
