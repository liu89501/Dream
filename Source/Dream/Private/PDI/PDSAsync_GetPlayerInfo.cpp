
#include "PDI/PDSAsync_GetPlayerInfo.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


UPDSAsync_GetPlayerInfo* UPDSAsync_GetPlayerInfo::PDI_GetPlayerInformation(UObject* WorldContextObject, EGetEquipmentCondition Condition)
{
	UPDSAsync_GetPlayerInfo* PDSGP = NewObject<UPDSAsync_GetPlayerInfo>(WorldContextObject);
	PDSGP->T_Condition = Condition;
	return PDSGP;
}

void UPDSAsync_GetPlayerInfo::OnLoadCompleted(const FPlayerInfo& PlayerInfo, bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnGetPlayerInfo(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast(PlayerInfo);
	}
	else
	{
		OnFailure.Broadcast(PlayerInfo);
	}
}

void UPDSAsync_GetPlayerInfo::Activate()
{
	Handle = FPDIStatic::Get()->AddOnGetPlayerInfo(FOnGetPlayerInfo::FDelegate::CreateUObject(this, &UPDSAsync_GetPlayerInfo::OnLoadCompleted));
	FPDIStatic::Get()->GetPlayerInfo(T_Condition);
}

