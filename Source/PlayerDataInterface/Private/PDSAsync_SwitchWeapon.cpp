
#include "PDSAsync_SwitchWeapon.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_SwitchWeapon::OnLoadCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}

	FPlayerDataInterfaceModule::Get()->OnSwitchWeaponComplete.Remove(Handle);
}

void UPDSAsync_SwitchWeapon::Init(int32 WeaponId, int32 EquipmentIndex)
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnSwitchWeaponComplete.AddUObject(this, &UPDSAsync_SwitchWeapon::OnLoadCompleted);
		PDS->EquipmentWeapon(WeaponId, EquipmentIndex);
	}
}

