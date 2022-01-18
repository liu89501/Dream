#include "PDI/PlayerDataInterfaceStatic.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerServerDataInterface.h"

FPlayerDataInterface* FPDIStatic::Singleton = nullptr;

void FPDIStatic::Initialize()
{
	if (Singleton == nullptr)
	{
		Singleton = new FPlayerServerDataInterface();

		Singleton->Initialize();
	}
}

void FPDIStatic::Shutdown()
{
#if WITH_EDITOR

	if (Singleton == nullptr)
	{
		return;
	}

#endif

	delete Singleton;
	Singleton = nullptr;
}

FPlayerDataInterface* FPDIStatic::Get()
{
	return Singleton;
}

bool IsLocal()
{
	FString PDI;
	GConfig->GetString(TEXT("PDISettings"), TEXT("PDI"), PDI, GEngineIni);
	return PDI.Equals(PDI_LOCAL, ESearchCase::IgnoreCase);
}

bool FPDIStatic::IsLocalInterface()
{
	static bool IsLocalPDI = IsLocal();
	return IsLocalPDI;
}
