
#include "PDSAsync_GetStoreItems.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_GetStoreItems::OnLoadCompleted(const TArray<FDItemInfo>& Items, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Items, ErrorMessage);
	}
	else
	{
		OnFail.Broadcast(Items, ErrorMessage);
	}

	FPlayerDataInterfaceModule::Get()->OnGetWeaponComplete.Remove(Handle);
}

void UPDSAsync_GetStoreItems::Init(int32 StoreId)
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnGetStoreItemComplete.AddUObject(this, &UPDSAsync_GetStoreItems::OnLoadCompleted);
		PDS->GetStoreItems(StoreId);
	}
}

