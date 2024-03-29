﻿
#include "PDI/PDSAsync_GetStoreItems.h"

#include "DGameplayStatics.h"
#include "DreamWidgetType.h"
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
	Handle = GDataInterface->AddOnGetStoreItems(FOnGetStoreItems::FDelegate::CreateUObject(this, &UPDSAsync_GetStoreInformation::OnLoadCompleted));
	GDataInterface->GetStoreItems(FSearchStoreItemsParam(T_StoreId, T_Page));
}

void UPDSAsync_GetStoreInformation::OnLoadCompleted(const FStoreInformation& StoreInfo, bool bSuccess)
{
	GDataInterface->RemoveOnGetStoreItems(Handle);

	FWStoreInformation StoreInformation;
	
	if (bSuccess)
	{
		StoreInformation.TotalPage = StoreInfo.TotalPage;
		StoreInformation.TotalItems = StoreInfo.TotalItems;
		
		for (const FStoreItem& Item : StoreInfo.Items)
		{
			UWStoreItem* UwStoreItem = NewObject<UWStoreItem>(this);
			UwStoreItem->Initialize(Item);
			StoreInformation.Items.Add(UwStoreItem);
		}
		
		OnSuccess.Broadcast(StoreInformation);
	}
	else
	{
		OnFailure.Broadcast(StoreInformation);
	}
}
