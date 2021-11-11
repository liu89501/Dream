#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_EquipWeapon.generated.h"


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

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_EquipWeapon* PDI_EquipWeapon(UObject* WorldContextObject, int64 WeaponId, int32 EquipIndex);

	virtual void Activate() override;

	void OnLoadCompleted(bool bSuccess) const;

private:

	int64 T_WeaponId;
	int32 T_EquipIndex;
	FDelegateHandle Handle;
};
