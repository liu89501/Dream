
#include "PDI/PDSAsync_GetPlayerInfo.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


UPDSAsync_GetPlayerInfo* UPDSAsync_GetPlayerInfo::PDI_GetPlayerInformation(UObject* WorldContextObject, EGetEquipmentCondition Condition)
{
	UPDSAsync_GetPlayerInfo* PDSGP = NewObject<UPDSAsync_GetPlayerInfo>(WorldContextObject);
	PDSGP->T_Condition = Condition;
	return PDSGP;
}

void UPDSAsync_GetPlayerInfo::OnLoadCompleted(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(PlayerInfo, ErrorMessage);
	}
	else
	{
		OnFailure.Broadcast(PlayerInfo, ErrorMessage);
	}
}

void UPDSAsync_GetPlayerInfo::Activate()
{
	FGetPlayerInfoComplete Delegate;
	Delegate.BindUObject(this, &UPDSAsync_GetPlayerInfo::OnLoadCompleted);
	FPlayerDataInterfaceStatic::Get()->GetPlayerInfo(T_Condition, Delegate);
}

