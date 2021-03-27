
#include "PDSAsync_GetStoreItems.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_GetStoreItems::OnLoadCompleted(const TArray<FStoreItem>& Items, const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Items, ErrorMessage);
	}
	else
	{
		OnFail.Broadcast(Items, ErrorMessage);
	}

	FPlayerDataStoreModule::Get()->OnGetWeaponComplete.Remove(Handle);
}

void UPDSAsync_GetStoreItems::Init(int32 StoreId)
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnGetStoreItemComplete.AddUObject(this, &UPDSAsync_GetStoreItems::OnLoadCompleted);
		PDS->GetStoreItems(StoreId);
	}
}

