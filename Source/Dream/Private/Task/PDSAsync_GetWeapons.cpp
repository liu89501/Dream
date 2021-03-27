
#include "PDSAsync_GetWeapons.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_GetWeapons::OnLoadCompleted(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Weapons);
	}
	else
	{
		OnFail.Broadcast(Weapons);
	}

	FPlayerDataStoreModule::Get()->OnGetWeaponComplete.Remove(Handle);
}

void UPDSAsync_GetWeapons::Init(EGetWeaponCondition Condition)
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnGetWeaponComplete.AddUObject(this, &UPDSAsync_GetWeapons::OnLoadCompleted);
		PDS->GetPlayerWeapons(Condition);
	}
}

