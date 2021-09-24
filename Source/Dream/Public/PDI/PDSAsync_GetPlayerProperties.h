#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PDSAsync_GetPlayerProperties.generated.h"


UCLASS()
class UPDSAsync_GetPlayerProperties : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FPlayerProperties&, Properties);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_GetPlayerProperties* PDI_GetPlayerProperties(UObject* WorldContextObject);

	void OnLoadCompleted(const FPlayerProperties& Properties, const FString& ErrorMessage) const;
};
