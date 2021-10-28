
#include "PDI/PDSAsync_EquipWeapon.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_EquipWeapon* UPDSAsync_EquipWeapon::PDI_EquipWeapon(UObject* WorldContextObject, int64 WeaponId, int32 EquipIndex)
{
	UPDSAsync_EquipWeapon* PDSI = NewObject<UPDSAsync_EquipWeapon>(WorldContextObject);
	PDSI->T_WeaponId = WeaponId;
	PDSI->T_EquipIndex = EquipIndex;
	return PDSI;
}

void UPDSAsync_EquipWeapon::Activate()
{
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_EquipWeapon::OnLoadCompleted);
	FPlayerDataInterfaceStatic::Get()->EquipWeapon(T_WeaponId, T_EquipIndex, Delegate);
}

void UPDSAsync_EquipWeapon::OnLoadCompleted(const FString& ErrorMessage) const
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

