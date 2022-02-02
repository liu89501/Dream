
#include "PDI/PDSAsync_InitBaseProps.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_InitBaseProps* UPDSAsync_InitBaseProps::PDI_InitBaseProps(UObject* WorldContextObject)
{
	if (UPDSAsync_InitBaseProps* PDSEM = NewObject<UPDSAsync_InitBaseProps>(WorldContextObject))
	{
		return PDSEM;
	}
	return nullptr;
}

void UPDSAsync_InitBaseProps::Activate()
{
	Handle = FPDIStatic::Get()->AddOnGetPlayerInfo(FOnGetPlayerInfo::FDelegate::CreateUObject(this, &UPDSAsync_InitBaseProps::OnCompleted));
	//FPDIStatic::Get()->GetPlayerInfo(Query_Cond_Gears);
}

void UPDSAsync_InitBaseProps::OnCompleted(const FPlayerInfo& PlayerInfo, bool bSuccess)
{
	FPlayerDataInterface* DataInterface = FPDIStatic::Get();
	DataInterface->RemoveOnGetPlayerInfo(Handle);
	
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}
