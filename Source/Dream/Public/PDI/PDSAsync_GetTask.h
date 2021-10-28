#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_GetTask.generated.h"


UCLASS()
class UPDSAsync_GetTask: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const TArray<FTaskInformation>&, Tasks);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_GetTask* PDI_GetTask(UObject* WorldContextObject, int32 TaskGroupId, EGetTaskCondition Condition);

	virtual void Activate() override;
	
	void OnCompleted(const TArray<FTaskInformation>& Tasks, const FString& ErrorMessage) const;

private:

	int32 T_TaskGroupId;
	EGetTaskCondition T_Condition;
};
