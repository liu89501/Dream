
#include "PDI/PDSAsync_PayItem.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_PayItem* UPDSAsync_PayItem::PDI_BuyItem(UObject* WorldContextObject, int64 ItemId)
{
	UPDSAsync_PayItem* PDSPI = NewObject<UPDSAsync_PayItem>(WorldContextObject);
	PDSPI->T_ItemId = ItemId;
	return PDSPI;
}

void UPDSAsync_PayItem::OnLoadCompleted(bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnBuyItem(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

void UPDSAsync_PayItem::Activate()
{
	Handle = FPDIStatic::Get()->AddOnBuyItem(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_PayItem::OnLoadCompleted));
	FPDIStatic::Get()->PayItem(T_ItemId);
}

