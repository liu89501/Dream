#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_Decompose.generated.h"


UCLASS()
class UPDSAsync_Decompose: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	/**
	 * 分解物品
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_Decompose* PDI_DecomposeItem(UObject* WorldContextObject, int64 ItemId, int32 ItemGuid);

	void OnCompleted(bool bSuccess);

	virtual void Activate() override;

private:

	int64 T_ItemId;
	int64 T_ItemGuid;

	FDelegateHandle Handle;
};
