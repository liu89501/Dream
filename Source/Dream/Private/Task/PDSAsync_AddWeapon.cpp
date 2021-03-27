
#include "PDSAsync_AddWeapon.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_AddWeapon::OnLoadCompleted(bool bSuccessful)
{
	if (bSuccessful)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}

	FPlayerDataStoreModule::Get()->OnAddWeaponComplete.Remove(Handle);
}

void UPDSAsync_AddWeapon::Init(const TArray<FPlayerWeaponAdd>& NewWeapons)
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnAddWeaponComplete.AddUObject(this, &UPDSAsync_AddWeapon::OnLoadCompleted);
		PDS->AddWeapons(NewWeapons);
	}
}

