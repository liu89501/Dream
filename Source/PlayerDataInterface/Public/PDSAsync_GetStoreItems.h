#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_GetStoreItems.generated.h"

UCLASS()
class PLAYERDATAINTERFACE_API UPDSAsync_GetStoreItems : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncTaskCompoleteDelegate, const TArray<FDItemInfo>&, Items, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(const TArray<FDItemInfo>& Items, const FString& ErrorMessage) const;

	void Init(int32 StoreId);

private:

	FDelegateHandle Handle;
};
