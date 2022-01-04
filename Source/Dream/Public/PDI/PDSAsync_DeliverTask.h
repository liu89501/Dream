#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_DeliverTask.generated.h"


UCLASS()
class UPDSAsync_DeliverTask: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_DeliverTask* PDI_DeliverTask(UObject* WorldContextObject, int64 TaskId);

	void OnCompleted(bool bSuccess);

	virtual void Activate() override;

private:

	int64 T_TaskId;

	FDelegateHandle Handle;
};
