#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_PayItem.generated.h"


UCLASS()
class UPDSAsync_PayItem : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_PayItem* PDI_BuyItem(UObject* WorldContextObject, int32 StoreId, int64 ItemId);

	UFUNCTION()
	void OnLoadCompleted(const FString& ErrorMessage) const;

	void Init(int32 StoreId, int64 ItemId) const;
};
