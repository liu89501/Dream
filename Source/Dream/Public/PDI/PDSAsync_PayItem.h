#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_PayItem.generated.h"


UCLASS()
class UPDSAsync_PayItem : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_PayItem* PDI_BuyItem(UObject* WorldContextObject, int32 StoreId, int64 ItemId);

	UFUNCTION()
	void OnLoadCompleted(bool bSuccess) const;

	virtual void Activate() override;

private:

	int32 T_StoreId;
	int64 T_ItemId;
	FDelegateHandle Handle;
	
};
