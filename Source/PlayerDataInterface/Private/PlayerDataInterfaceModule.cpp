#include "PlayerDataInterfaceModule.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceType.h"
#include "OnlineSubsystem.h"

FPlayerDataInterface* FPlayerDataInterfaceModule::Singleton = nullptr;

void FPlayerDataInterfaceModule::StartupModule()
{
	if (Singleton == nullptr)
	{
		if (IOnlineSubsystem* OSSystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
		{
			Singleton = new FPlayerDataInterface(OSSystem);
		}
	}
}

void FPlayerDataInterfaceModule::ShutdownModule()
{
	delete Singleton;
	Singleton = nullptr;
}

FPlayerDataInterface* FPlayerDataInterfaceModule::Get()
{
	
#if WITH_EDITOR
	if (Singleton == nullptr)
	{
		if (IOnlineSubsystem* OSSystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
		{
			Singleton = new FPlayerDataInterface(OSSystem);
		}
	}
#endif
	
	return Singleton;
}

IMPLEMENT_MODULE(FPlayerDataInterfaceModule, PlayerDataInterface)
