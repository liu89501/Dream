#include "PDI/PlayerDataInterfaceStatic.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerLocalDataInterface.h"
#include "PDI/PlayerServerDataInterface.h"

FPlayerDataInterface* FPlayerDataInterfaceStatic::Singleton = nullptr;

void FPlayerDataInterfaceStatic::Startup()
{
	if (Singleton == nullptr)
	{
		if (FPlayerDataInterfaceStatic::IsLocalInterface())
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
#if WITH_EDITOR

	if (Singleton == nullptr)
	{
		return;
	}

#endif
	
	if (IsRunningDedicatedServer())
    {
    	Singleton->UnRegisterServer();
    }
    else
    {
    	Singleton->Logout();
    }

	delete Singleton;
	Singleton = nullptr;
}

FPlayerDataInterface* FPlayerDataInterfaceStatic::Get()
{
	return Singleton;
}

bool IsLocal()
{
	FString PDI;
	GConfig->GetString(TEXT("MySettings"), TEXT("PDI"), PDI, GEngineIni);
	return PDI.Equals(PDI_LOCAL, ESearchCase::IgnoreCase);
}

bool FPlayerDataInterfaceStatic::IsLocalInterface()
{
	static bool IsLocalPDI = IsLocal();
	return IsLocalPDI;
}