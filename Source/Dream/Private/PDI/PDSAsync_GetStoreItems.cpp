
#include "PDI/PDSAsync_GetStoreItems.h"

#include "DGameplayStatics.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetStoreInformation* UPDSAsync_GetStoreInformation::PDI_GetStoreInformation(UObject* WorldContextObject, int32 StoreId, int32 Page)
{
	UPDSAsync_GetStoreInformation* Information = NewObject<UPDSAsync_GetStoreInformation>(WorldContextObject);
	Information->T_StoreId = StoreId;
	Information->T_Page = Page;
	return Information;
}

void UPDSAsync_GetStoreInformation::Activate()
{
	Handle = FPDIStatic::Get()->AddOnGetStoreItems(FOnGetStoreItems::FDelegate::CreateUObject(this, &UPDSAsync_GetStoreInformation::OnLoadCompleted));
	FPDIStatic::Get()->GetStoreItems(FSearchStoreItemsParam(T_StoreId, T_Page));
}

void UPDSAsync_GetStoreInformation::OnLoadCompleted(const FStoreInformation& StoreInfo, bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnGetStoreItems(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast(StoreInfo);
	}
	else
	{
		OnFailure.Broadcast(StoreInfo);
	}
}
