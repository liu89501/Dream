
#include "PDI/PDSAsync_GetStoreInformation.h"

#include "DGameplayStatics.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetStoreInformation* UPDSAsync_GetStoreInformation::PDI_GetStoreInformation(UObject* WorldContextObject, int32 StoreId)
{
	UPDSAsync_GetStoreInformation* Information = NewObject<UPDSAsync_GetStoreInformation>(WorldContextObject);
	Information->T_StoreId = StoreId;
	return Information;
}

void UPDSAsync_GetStoreInformation::Activate()
{
	FGetStoreItemsComplete Delegate;
	Delegate.BindUObject(this, &UPDSAsync_GetStoreInformation::OnLoadCompleted);
	FPlayerDataInterfaceStatic::Get()->GetStoreItems(T_StoreId, Delegate);
}

void UPDSAsync_GetStoreInformation::OnLoadCompleted(const FStoreInformation& StoreInfo, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(StoreInfo, ErrorMessage);
	}
	else
	{
		OnFailure.Broadcast(StoreInfo, ErrorMessage);
	}
}
