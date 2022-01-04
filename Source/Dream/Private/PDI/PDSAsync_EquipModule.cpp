
#include "PDI/PDSAsync_EquipModule.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_EquipModule* UPDSAsync_EquipModule::PDI_EquipModule(UObject* WorldContextObject, int64 ModuleId, EModuleCategory Category)
{
	if (UPDSAsync_EquipModule* PDSEM = NewObject<UPDSAsync_EquipModule>(WorldContextObject))
	{
		PDSEM->T_ModuleId = ModuleId;
		PDSEM->T_Category = Category;
		return PDSEM;
	}
	return nullptr;
}

void UPDSAsync_EquipModule::Activate()
{
	Handle = FPDIStatic::Get()->AddOnEquipModule(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_EquipModule::OnCompleted));
	FPDIStatic::Get()->EquipModule(FEquipModuleParam(T_ModuleId, T_Category));
}

void UPDSAsync_EquipModule::OnCompleted(bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnEquipModule(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}
