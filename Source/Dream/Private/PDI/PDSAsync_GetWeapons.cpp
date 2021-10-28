
#include "PDI/PDSAsync_GetWeapons.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetWeapons* UPDSAsync_GetWeapons::PDI_GetWeapons(UObject* WorldContextObject, EGetEquipmentCondition Condition)
{
	UPDSAsync_GetWeapons* PDSGW = NewObject<UPDSAsync_GetWeapons>(WorldContextObject);
	PDSGW->T_Condition = Condition;
	return PDSGW;
}

void UPDSAsync_GetWeapons::OnLoadCompleted(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Weapons);
	}
	else
	{
		OnFailure.Broadcast(Weapons);
	}
}

void UPDSAsync_GetWeapons::Activate()
{
	FGetWeaponComplete Delegate;
	Delegate.BindUObject(this, &UPDSAsync_GetWeapons::OnLoadCompleted);
	FPlayerDataInterfaceStatic::Get()->GetPlayerWeapons(T_Condition, Delegate);
}

