
#include "PDI/PDSAsync_EquipWeapon.h"
#include "DCharacterPlayer.h"
#include "DMProjectSettings.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_EquipWeapon* UPDSAsync_EquipWeapon::PDI_EquipWeapon(ADCharacterPlayer* Character, int64 WeaponId, int32 EquipIndex)
{
	UPDSAsync_EquipWeapon* PDSI = NewObject<UPDSAsync_EquipWeapon>();
	PDSI->T_WeaponId = WeaponId;
	PDSI->T_EquipIndex = EquipIndex;
	PDSI->T_Character = Character;
	return PDSI;
}

void UPDSAsync_EquipWeapon::Activate()
{
	Handle = GDataInterface->AddOnEquipWeapon(FOnEquipWeapon::FDelegate::CreateUObject(this, &UPDSAsync_EquipWeapon::OnCompleted));
	GDataInterface->EquipWeapon(FEquipWeaponParam(T_WeaponId, T_EquipIndex));
}

void UPDSAsync_EquipWeapon::OnCompleted(const FPlayerWeapon& EquippedWeapon, bool bSuccess)
{
	GDataInterface->RemoveOnEquipWeapon(Handle);
	
	if (bSuccess)
	{
		if (T_Character)
		{
			const FItemDef& ItemDefinition = GSProject->GetItemDefinition(EquippedWeapon.ItemGuid);

			if (UClass* ItemClass = ItemDefinition.GetItemClass())
			{
				FGearDesc GearDesc;
				GearDesc.Attributes = EquippedWeapon.Attributes;
				GearDesc.EquippedIdx = EquippedWeapon.Index;
				GearDesc.GearClass = ItemClass;
				GearDesc.GearLevel = EquippedWeapon.WeaponLevel;
				GearDesc.GearQuality = ItemDefinition.ItemBaseInfo.PropsQuality;
				
				T_Character->EquipWeapon(GearDesc);
			}
		}
		
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

