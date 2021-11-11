#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_AcceptTask.generated.h"


UCLASS()
class UPDSAsync_AcceptTask: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_AcceptTask* PDI_AcceptTask(UObject* WorldContextObject, int32 InTaskId);

	void OnCompleted(bool ) const;

	virtual void Activate() override;

private:

	int32 TaskId;

	FDelegateHandle Handle;
};
