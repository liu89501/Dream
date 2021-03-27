
#include "PDSAsync_SwitchWeapon.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_SwitchWeapon::OnLoadCompleted(const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}

	FPlayerDataStoreModule::Get()->OnSwitchWeaponComplete.Remove(Handle);
}

void UPDSAsync_SwitchWeapon::Init(int32 WeaponId, int32 EquipmentIndex)
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnSwitchWeaponComplete.AddUObject(this, &UPDSAsync_SwitchWeapon::OnLoadCompleted);
		PDS->EquipmentWeapon(WeaponId, EquipmentIndex);
	}
}

