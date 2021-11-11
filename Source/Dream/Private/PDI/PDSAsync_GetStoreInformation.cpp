
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
	Handle = FPDIStatic::Get()->AddOnGetStoreItems(FOnGetStoreItems::FDelegate::CreateUObject(this, &UPDSAsync_GetStoreInformation::OnLoadCompleted));
	FPDIStatic::Get()->GetStoreItems(T_StoreId);
}

void UPDSAsync_GetStoreInformation::OnLoadCompleted(const FStoreInformation& StoreInfo, bool bSuccess) const
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
