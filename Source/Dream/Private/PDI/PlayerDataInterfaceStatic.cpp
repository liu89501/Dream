#include "PDI/PlayerDataInterfaceStatic.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerLocalDataInterface.h"
#include "PDI/PlayerServerDataInterface.h"

FPlayerDataInterface* FPDIStatic::Singleton = nullptr;
UTaskDataAsset* FPDIStatic::TaskDataAsset = nullptr;

void FPDIStatic::Startup()
{
	if (Singleton == nullptr)
	{
		TaskDataAsset = LoadObject<UTaskDataAsset>(nullptr, TEXT("/Game/Main/Asset/DA_TaskList"));
		checkf(TaskDataAsset, TEXT("Load TaskDataAsset Failure"));
		
		if (FPDIStatic::IsLocalInterface())
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

UTaskDataAsset* FPDIStatic::GetTaskDataAsset()
{
	return TaskDataAsset;
}
