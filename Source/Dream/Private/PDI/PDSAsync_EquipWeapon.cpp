
#include "PDI/PDSAsync_EquipWeapon.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_EquipWeapon* UPDSAsync_EquipWeapon::PDI_EquipWeapon(UObject* WorldContextObject, int64 WeaponId, int32 EquipIndex)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_EquipWeapon* PDSI = NewObject<UPDSAsync_EquipWeapon>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [WeaponId, EquipIndex, PDSI]
        {
			FCommonCompleteNotify Delegate;
			Delegate.BindUObject(PDSI, &UPDSAsync_EquipWeapon::OnLoadCompleted);
			FPlayerDataInterfaceStatic::Get()->EquipWeapon(WeaponId, EquipIndex, Delegate);
			
        }, 0.001f, false);
		
		return PDSI;
	}
	
	return nullptr;
}

void UPDSAsync_EquipWeapon::OnLoadCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

