
#include "PDI/PDSAsync_GetPlayerInfo.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


UPDSAsync_GetPlayerInfo* UPDSAsync_GetPlayerInfo::PDI_GetPlayerInformation(UObject* WorldContextObject,
																bool bWeapon,
                                                                bool bWeaponEquippedOnly,
                                                                bool bModule,
                                                                bool bModuleEquippedOnly,
                                                                bool bMaterials,
                                                                bool bSkin)
{
	
	UPDSAsync_GetPlayerInfo* PDSGP = NewObject<UPDSAsync_GetPlayerInfo>(WorldContextObject);

	int32 Condition = 0;
	Condition |= bWeapon ? EQueryCondition::Weapon : 0;
	Condition |= bWeaponEquippedOnly ? EQueryCondition::Weapon_EquippedOnly : 0;
	Condition |= bModule ? EQueryCondition::Module : 0;
	Condition |= bModuleEquippedOnly ? EQueryCondition::Module_EquippedOnly : 0;
	Condition |= bMaterials ? EQueryCondition::Materials : 0;
	Condition |= bSkin ? EQueryCondition::Skin : 0;
	
	PDSGP->T_Condition = Condition;
	return PDSGP;
}

void UPDSAsync_GetPlayerInfo::OnLoadCompleted(const FPlayerInfo& PlayerInfo, bool bSuccess)
{
	GDataInterface->RemoveOnGetPlayerInfo(Handle);
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
	Handle = GDataInterface->AddOnGetPlayerInfo(FOnGetPlayerInfo::FDelegate::CreateUObject(this, &UPDSAsync_GetPlayerInfo::OnLoadCompleted));
	GDataInterface->GetPlayerInfo(T_Condition);
}

