
#include "PDI/PDSAsync_GetWeapons.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetWeapons* UPDSAsync_GetWeapons::PDI_GetWeapons(UObject* WorldContextObject, EGetEquipmentCondition Condition)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_GetWeapons* PDSGW = NewObject<UPDSAsync_GetWeapons>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [Condition, PDSGW]
        {
			FGetWeaponComplete Delegate;
            Delegate.BindUObject(PDSGW, &UPDSAsync_GetWeapons::OnLoadCompleted);
			FPlayerDataInterfaceStatic::Get()->GetPlayerWeapons(Condition, Delegate);
			
        }, 0.001f, false);

		return PDSGW;
	}

	return nullptr;
}

void UPDSAsync_GetWeapons::OnLoadCompleted(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Weapons);
	}
	else
	{
		OnFailure.Broadcast(Weapons);
	}
}

