
#include "PDI/PDSAsync_Decompose.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_Decompose* UPDSAsync_Decompose::PDI_DecomposeItem(UObject* WorldContextObject, int64 ItemId, int32 ItemGuid)
{
	UPDSAsync_Decompose* PDSI = NewObject<UPDSAsync_Decompose>(WorldContextObject);
	PDSI->T_ItemGuid = ItemGuid;
	PDSI->T_ItemId = ItemId;
	return PDSI;
}

void UPDSAsync_Decompose::OnCompleted(bool bSuccess) 
{
	GDataInterface->RemoveOnDecomposeItem(Handle);
	
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

void UPDSAsync_Decompose::Activate()
{
	FPlayerDataInterface* Interface = GDataInterface;
	
	Handle = Interface->AddOnDecomposeItem(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_Decompose::OnCompleted));
	
	Interface->DecomposeItem(FDecomposeParam(T_ItemId, T_ItemGuid));
}
