#include "PDI/PlayerDataInterfaceStatic.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerLocalDataInterface.h"
#include "PDI/PlayerServerDataInterface.h"

FPlayerDataInterface* FPlayerDataInterfaceStatic::Singleton = nullptr;

void FPlayerDataInterfaceStatic::Startup()
{
	if (Singleton == nullptr)
	{
		if (FPlayerDataInterface::IsLocalInterface())
		{
			Singleton = new FPlayerLocalDataInterface();
		}
		else
		{
			Singleton = new FPlayerServerDataInterface();
		}

		Singleton->Initialize();
	}
}

void FPlayerDataInterfaceStatic::Shutdown()
{
	delete Singleton;
	Singleton = nullptr;
}

FPlayerDataInterface* FPlayerDataInterfaceStatic::Get()
{
	return Singleton;
}
