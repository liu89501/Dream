
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
	Handle = FPDIStatic::Get()->AddOnEquipWeapon(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_EquipWeapon::OnLoadCompleted));
	FPDIStatic::Get()->EquipWeapon(FEquipWeaponParam(T_WeaponId, T_EquipIndex));
}

void UPDSAsync_EquipWeapon::OnLoadCompleted(bool bSuccess) const
{
	FPDIStatic::Get()->RemoveOnEquipWeapon(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

