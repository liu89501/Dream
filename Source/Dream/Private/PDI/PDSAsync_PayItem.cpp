
#include "PDI/PDSAsync_PayItem.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_PayItem* UPDSAsync_PayItem::PDI_BuyItem(UObject* WorldContextObject, int32 StoreId, int64 ItemId)
{
	UPDSAsync_PayItem* PDSPI = NewObject<UPDSAsync_PayItem>(WorldContextObject);
	PDSPI->Init(StoreId, ItemId);
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

void UPDSAsync_PayItem::Init(int32 StoreId, int64 ItemId) const
{
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [=]
    {
        FCommonCompleteNotify Delegate;
        Delegate.BindUObject(this, &UPDSAsync_PayItem::OnLoadCompleted);
        FPlayerDataInterfaceStatic::Get()->PayItem(StoreId, ItemId, Delegate);
		
    }, 0.001f, false);
}

