
#include "PDSAsync_PayItem.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_PayItem::OnLoadCompleted(const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(ErrorMessage);
	}
	else
	{
		OnFail.Broadcast(ErrorMessage);
	}

	FPlayerDataStoreModule::Get()->OnPayItemComplete.Remove(Handle);
}

void UPDSAsync_PayItem::Init(int64 ItemId)
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnPayItemComplete.AddUObject(this, &UPDSAsync_PayItem::OnLoadCompleted);
		PDS->PayItem(ItemId);
	}
}

