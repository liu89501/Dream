
#include "PDI/PDSAsync_EquipModule.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_EquipModule* UPDSAsync_EquipModule::PDI_EquipModule(UObject* WorldContextObject, int64 ModuleId, EModuleCategory Category)
{
	if (UPDSAsync_EquipModule* PDSEM = NewObject<UPDSAsync_EquipModule>(WorldContextObject))
	{
		PDSEM->Init(ModuleId, Category);
		return PDSEM;
	}
	return nullptr;
}

void UPDSAsync_EquipModule::OnCompleted(const FString& ErrorMessage) const
{
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, ErrorMessage]
    {
        if (ErrorMessage.IsEmpty())
        {
            OnSuccess.Broadcast();
        }
        else
        {
            OnFailure.Broadcast();
        }
    }, 0.001f, false);
}

void UPDSAsync_EquipModule::Init(int64 ModuleId, EModuleCategory Category) const
{
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_EquipModule::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->EquipModule(ModuleId, Category, Delegate);
}
