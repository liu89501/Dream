
#include "PDI/PDSAsync_PayItem.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_PayItem* UPDSAsync_PayItem::PDI_BuyItem(UObject* WorldContextObject, int32 StoreId, int64 ItemId)
{
	UPDSAsync_PayItem* PDSPI = NewObject<UPDSAsync_PayItem>(WorldContextObject);
	PDSPI->T_StoreId = StoreId;
	PDSPI->T_ItemId = ItemId;
	return PDSPI;
}

void UPDSAsync_PayItem::OnLoadCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(ErrorMessage);
	}
	else
	{
		OnFailure.Broadcast(ErrorMessage);
	}
}

void UPDSAsync_PayItem::Activate()
{
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_PayItem::OnLoadCompleted);
	FPlayerDataInterfaceStatic::Get()->PayItem(T_StoreId, T_ItemId, Delegate);
}

