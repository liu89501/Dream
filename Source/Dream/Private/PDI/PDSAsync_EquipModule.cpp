
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
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_EquipModule::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->EquipModule(T_ModuleId, T_Category, Delegate);
}

void UPDSAsync_EquipModule::OnCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}
