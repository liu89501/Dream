#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_GetTask.generated.h"


UCLASS()
class UPDSAsync_GetTask: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FSearchTaskResult&, Result);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_GetTask* PDI_GetTask(UObject* WorldContextObject, int32 TaskGroupId, int32 Page, EGetTaskCondition Condition);

	virtual void Activate() override;
	
	void OnCompleted(const FSearchTaskResult& Result, bool bSuccess);

private:

	int32 T_TaskGroupId;
	int32 T_Page;
	EGetTaskCondition T_Condition;
	FDelegateHandle Handle;
};
