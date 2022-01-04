#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_MdfTaskTracking.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

UCLASS()
class UPDSAsync_MdfTaskTracking: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_MdfTaskTracking* PDI_ModifyTaskTracking(UObject* WorldContextObject, int64 InTaskId, bool bTrackingState);

	void OnCompleted(bool bSuccess);

	virtual void Activate() override;

private:

	int64 T_TaskId;

	bool T_TrackingState;
	
	FDelegateHandle Handle;
};
