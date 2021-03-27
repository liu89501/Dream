﻿#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "PlayerDataStoreType.h"
#include "PDSAsync_PayItem.generated.h"


UCLASS()
class DREAM_API UPDSAsync_PayItem : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(const FString& ErrorMessage);

	void Init(int64 ItemId);
	
private:

	FDelegateHandle Handle;
};
