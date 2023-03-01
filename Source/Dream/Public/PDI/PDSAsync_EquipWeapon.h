#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_EquipWeapon.generated.h"

class ADCharacterPlayer;

UCLASS()
class UPDSAsync_EquipWeapon : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_EquipWeapon* PDI_EquipWeapon(ADCharacterPlayer* Character, int64 WeaponId, int32 EquipIndex);

	virtual void Activate() override;

	void OnCompleted(const FPlayerWeapon& EquippedWeapon, bool bSuccess);

private:

	int64 T_WeaponId;
	int32 T_EquipIndex;

	UPROPERTY()
	ADCharacterPlayer* T_Character;
	
	FDelegateHandle Handle;
};
