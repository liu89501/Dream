
#include "PDI/PDSAsync_GetPlayerInfo.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


UPDSAsync_GetPlayerInfo* UPDSAsync_GetPlayerInfo::PDI_GetPlayerInformation(UObject* WorldContextObject,
																bool bWeapon,
                                                                bool bWeaponEquipped,
                                                                bool bModule,
                                                                bool bModuleEquipped,
                                                                bool bMaterials,
                                                                bool bSkin)
{
	
	UPDSAsync_GetPlayerInfo* PDSGP = NewObject<UPDSAsync_GetPlayerInfo>(WorldContextObject);

	int32 Condition = 0;
	Condition |= bWeapon ? EQueryCondition::Weapon : 0;
	Condition |= bWeaponEquipped ? EQueryCondition::Weapon_Equipped : 0;
	Condition |= bModule ? EQueryCondition::Module : 0;
	Condition |= bModuleEquipped ? EQueryCondition::Module_Equipped : 0;
	Condition |= bMaterials ? EQueryCondition::Materials : 0;
	Condition |= bSkin ? EQueryCondition::Skin : 0;
	
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

