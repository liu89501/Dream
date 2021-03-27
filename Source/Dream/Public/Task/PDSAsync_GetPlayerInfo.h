#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "PlayerDataStoreType.h"
#include "PDSAsync_GetPlayerInfo.generated.h"

UCLASS()
class DREAM_API UPDSAsync_GetPlayerInfo : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncTaskCompoleteDelegate, const FPlayerInfo&, PlayerInfo, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage);

	void Init();

private:

	FDelegateHandle Handle;
};
