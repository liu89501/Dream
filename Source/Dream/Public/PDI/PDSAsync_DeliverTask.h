﻿#pragma once

#include "CoreMinimal.h"

#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_DeliverTask.generated.h"


UCLASS()
class UPDSAsync_DeliverTask: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, UItemData*, Rewards);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_DeliverTask* PDI_DeliverTask(UObject* WorldContextObject, int32 TaskId);

	void OnCompleted(UItemData* Rewards, bool bSuccess) const;

	virtual void Activate() override;

private:

	int32 T_TaskId;

	FDelegateHandle Handle;
};
