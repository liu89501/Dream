
#include "PDSAsync_AddWeapon.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_AddWeapon::OnLoadCompleted(bool bSuccessful) const
{
	if (bSuccessful)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}

	FPlayerDataInterfaceModule::Get()->OnAddWeaponComplete.Remove(Handle);
}

void UPDSAsync_AddWeapon::Init(const TArray<FPlayerWeaponAdd>& NewWeapons)
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnAddWeaponComplete.AddUObject(this, &UPDSAsync_AddWeapon::OnLoadCompleted);
		PDS->AddWeapons(NewWeapons);
	}
}

