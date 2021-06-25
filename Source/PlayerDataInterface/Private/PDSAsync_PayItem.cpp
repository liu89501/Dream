
#include "PDSAsync_PayItem.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_PayItem::OnLoadCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(ErrorMessage);
	}
	else
	{
		OnFail.Broadcast(ErrorMessage);
	}

	FPlayerDataInterfaceModule::Get()->OnPayItemComplete.Remove(Handle);
}

void UPDSAsync_PayItem::Init(int64 ItemId)
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnPayItemComplete.AddUObject(this, &UPDSAsync_PayItem::OnLoadCompleted);
		PDS->PayItem(ItemId);
	}
}

