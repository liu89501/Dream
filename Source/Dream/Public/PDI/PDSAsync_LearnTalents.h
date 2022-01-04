#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_LearnTalents.generated.h"


UCLASS()
class UPDSAsync_LearnTalents : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_LearnTalents* PDI_LearnTalents(UObject* WorldContextObject, int64 TalentIds);

	UFUNCTION()
	void OnCompleted(bool bSuccess);

private:

	int64 T_TalentIds;
	FDelegateHandle Handle;
};
