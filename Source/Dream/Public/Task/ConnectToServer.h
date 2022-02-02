// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMProjectSettings.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "ConnectToServer.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UConnectToServer : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

public:

	// Called when there is a successful
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	// Called when there is an unsuccessful
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Dream|Async")
	static UConnectToServer* Connect(UObject* WorldContextObject, const FLevelInformation& LevelInformation);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

private:

	void OnPDISearchServer(const struct FSearchServerResult& Result, bool bSuccessfully);
	
	void OnCreateSession(FName SessionName, bool bSuccessfully);

private:
	
	FLevelInformation T_LevelInformation;
	FDelegateHandle Handle_SearchServer;
	FDelegateHandle Handle_CreateSession;

	TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> SessionInterface;

};



