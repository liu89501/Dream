
#include "PDSAsync_GetWeapons.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_GetWeapons::OnLoadCompleted(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Weapons);
	}
	else
	{
		OnFail.Broadcast(Weapons);
	}

	FPlayerDataInterfaceModule::Get()->OnGetWeaponComplete.Remove(Handle);
}

void UPDSAsync_GetWeapons::Init(EGetWeaponCondition Condition)
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnGetWeaponComplete.AddUObject(this, &UPDSAsync_GetWeapons::OnLoadCompleted);
		PDS->GetPlayerWeapons(Condition);
	}
}

