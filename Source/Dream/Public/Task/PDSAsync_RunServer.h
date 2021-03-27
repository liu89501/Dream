// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlayerDataStoreType.h"
#include "PDSAsync_RunServer.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UPDSAsync_RunServer : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FString&, ServerAddr);

public:

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;


	UFUNCTION()
	void OnLoadCompleted(const FString& ServerAddr, const FString& ErrorMessage);

	void Init(const FRunServerParameter& Parameter);

private:

	FDelegateHandle Handle;
	
};
