#include "PDI/PlayerDataInterfaceStatic.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerServerDataInterface.h"

FPlayerDataInterface* GDataInterface = nullptr;

void FPDIStatic::Initialize()
{
	if (GDataInterface == nullptr)
	{
		GDataInterface = new FPlayerServerDataInterface();
		GDataInterface->Initialize();
	}
}

void FPDIStatic::Shutdown()
{
#if WITH_EDITOR

	if (GDataInterface == nullptr)
	{
		return;
	}

#endif

	delete GDataInterface;
	GDataInterface = nullptr;
}
