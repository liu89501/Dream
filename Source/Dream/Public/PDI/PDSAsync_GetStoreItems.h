#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_GetStoreItems.generated.h"

UCLASS()
class UPDSAsync_GetStoreInformation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FStoreInformation&, StoreInfo);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_GetStoreInformation* PDI_GetStoreInformation(UObject* WorldContextObject, int32 StoreId, int32 Page);

	virtual void Activate() override;

	void OnLoadCompleted(const FStoreInformation& StoreInfo, bool bSuccess);

private:

	int32 T_StoreId;
	int32 T_Page;
	FDelegateHandle Handle;
};
