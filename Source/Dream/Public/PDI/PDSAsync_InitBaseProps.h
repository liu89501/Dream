#pragma once

#include "CoreMinimal.h"

#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_InitBaseProps.generated.h"


UCLASS()
class UPDSAsync_InitBaseProps : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_InitBaseProps* PDI_InitBaseProps(UObject* WorldContextObject);

	virtual void Activate() override;

	UFUNCTION()
	void OnCompleted(const FPlayerInfo& PlayerInfo, bool bSuccess);

private:
	
	FDelegateHandle Handle;
};
