#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_GetStoreInformation.generated.h"

UCLASS()
class UPDSAsync_GetStoreInformation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncTaskCompoleteDelegate, const FStoreInformation&, StoreInfo, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_GetStoreInformation* PDI_GetStoreInformation(UObject* WorldContextObject, int32 StoreId);

	void OnLoadCompleted(const FStoreInformation& StoreInfo, const FString& ErrorMessage) const;
};
