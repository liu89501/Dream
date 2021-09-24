
#include "PDI/PDSAsync_GetStoreInformation.h"

#include "DGameplayStatics.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetStoreInformation* UPDSAsync_GetStoreInformation::PDI_GetStoreInformation(UObject* WorldContextObject, int32 StoreId)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_GetStoreInformation* Information = NewObject<UPDSAsync_GetStoreInformation>(WorldContextObject);
		
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [=]
        {
	        FGetStoreItemsComplete Delegate;
            Delegate.BindUObject(Information, &UPDSAsync_GetStoreInformation::OnLoadCompleted);
            FPlayerDataInterfaceStatic::Get()->GetStoreItems(StoreId, Delegate);
			
        }, 0.001f, false);

		return Information;
	}

	return nullptr;
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
