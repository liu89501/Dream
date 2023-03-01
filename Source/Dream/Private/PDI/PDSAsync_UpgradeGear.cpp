
#include "PDI/PDSAsync_UpgradeGear.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_UpgradeGear* UPDSAsync_UpgradeGear::PDI_UpgradeGear(UObject* WorldContextObject, int64 ItemId, bool bIsWeapon)
{
	UPDSAsync_UpgradeGear* PDII = NewObject<UPDSAsync_UpgradeGear>(WorldContextObject);
	PDII->T_bIsWeapon = bIsWeapon;
	PDII->T_ItemId = ItemId;
	return PDII;
}

void UPDSAsync_UpgradeGear::OnCompleted(bool bUpgradeResult, bool bSuccess) 
{
	GDataInterface->RemoveOnUpgradeGear(Handle);
	
	if (bSuccess)
	{
		OnSuccess.Broadcast(bUpgradeResult);
	}
	else
	{
		OnFailure.Broadcast(bUpgradeResult);
	}
}

void UPDSAsync_UpgradeGear::Activate()
{
	Handle = GDataInterface->AddOnUpgradeGear(FOnUpgradeGear::FDelegate::CreateUObject(this, &UPDSAsync_UpgradeGear::OnCompleted));

	if (T_bIsWeapon)
	{
		GDataInterface->UpgradeWeapon(T_ItemId);
	}
	else
	{
		GDataInterface->UpgradeModule(T_ItemId);
	}
}
