#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_UpgradeGear.generated.h"


UCLASS()
class UPDSAsync_UpgradeGear: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, bool, bUpgradeResult);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	/**
	 * 升级装备
	 * @bIsWeapon 如果是武器的话请勾选这个选项, 否则会当作模组
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_UpgradeGear* PDI_UpgradeGear(UObject* WorldContextObject, int64 ItemId, bool bIsWeapon = true);

	void OnCompleted(bool bUpgradeResult, bool bSuccess);

	virtual void Activate() override;

private:

	int64 T_ItemId;
	bool T_bIsWeapon;

	FDelegateHandle Handle;
};
