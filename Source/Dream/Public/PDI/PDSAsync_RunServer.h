// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_RunServer.generated.h"

/**
 * 
 */
UCLASS()
class UPDSAsync_RunServer : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FString&, ServerAddr);

public:

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_RunServer* PDI_GetServer(UObject* WorldContextObject, const FRunServerParameter& Parameter);

	void OnLoadCompleted(const FString& ServerAddr, const FString& ErrorMessage) const;
	
};
