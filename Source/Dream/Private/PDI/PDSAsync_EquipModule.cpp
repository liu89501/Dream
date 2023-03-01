
#include "PDI/PDSAsync_EquipModule.h"
#include "DCharacterPlayer.h"
#include "DMPlayerController.h"
#include "DMProjectSettings.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_EquipModule* UPDSAsync_EquipModule::PDI_EquipModule(ADCharacterPlayer* Character, int64 ModuleId, EModuleCategory Category)
{
	if (UPDSAsync_EquipModule* PDSEM = NewObject<UPDSAsync_EquipModule>())
	{
		PDSEM->T_ModuleId = ModuleId;
		PDSEM->T_Category = Category;
		PDSEM->T_Character = Character;
		return PDSEM;
	}
	return nullptr;
}

void UPDSAsync_EquipModule::Activate()
{
	Handle = GDataInterface->AddOnEquipModule(FOnEquipModule::FDelegate::CreateUObject(this, &UPDSAsync_EquipModule::OnCompleted));
	GDataInterface->EquipModule(FEquipModuleParam(T_ModuleId, T_Category));
}

void UPDSAsync_EquipModule::OnCompleted(const FPlayerModule& EquippedModule, bool bSuccess)
{
	GDataInterface->RemoveOnEquipModule(Handle);
	if (bSuccess)
	{
		if (T_Character)
		{
			const FItemDef& ItemDefinition = GSProject->GetItemDefinition(EquippedModule.ItemGuid);

			if (UClass* ItemClass = ItemDefinition.GetItemClass())
			{
				FGearDesc GearDesc;
				GearDesc.Attributes = EquippedModule.Attributes;
				GearDesc.EquippedIdx = static_cast<uint8>(EquippedModule.Category);
				GearDesc.GearClass = ItemClass;
				GearDesc.GearLevel = EquippedModule.ModuleLevel;
				GearDesc.GearQuality = ItemDefinition.ItemBaseInfo.PropsQuality;
				
				T_Character->EquipModule(GearDesc);
			}
		}
		
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}
