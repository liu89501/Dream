// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMProjectSettings.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "MatchmakingCallProxy.generated.h"

class APlayerController;

USTRUCT(BlueprintType)
struct FMatchmakingHandle
{
	GENERATED_USTRUCT_BODY()

public:

	FMatchmakingHandle() = default;

	FMatchmakingHandle(UWorld* InWorld) : World(InWorld)
	{
	}

	/* 匹配期间可能会用到的所有任务句柄 */
	FTimerHandle Handle_Ticker;
	FTimerHandle Handle_Search;
	
	FDelegateHandle Handle_UpdateSessionComplete;
	FDelegateHandle Handle_FindSessionsComplete;
	FDelegateHandle Handle_JoinSessionComplete;
	FDelegateHandle Handle_CustomUpdateComplete;
	FDelegateHandle Handle_CreateSessionComplete;
	
	FDelegateHandle Handle_LaunchServer;

public:

	void Clear();

private:

	UPROPERTY()
	UWorld* World;

};

/**
 * 
 */
UCLASS()
class DREAM_API UMatchmakingCallProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

public:

	// Called when there is a successful
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	// Called when there is an unsuccessful
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Dream|Async")
	static UMatchmakingCallProxy* Matchmaking(
		APlayerController* PlayerController, 
		const FLevelInformation& LevelInformation,
		FMatchmakingHandle& OutHandle
	);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

private:

	void WaitingTick();

	void FindLobby();

	void CreateLobby();

	void OnSearchCompleted(bool bSuccessful);

	void OnCreateSessionCompleted(FName SessionName, bool bSuccessful);

	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type CompleteResult);

	void OnLaunchServer(const struct FLaunchServerResult& Result, bool bSuccessfully);
	
	void OnServerReadyComplete(FName SessionName, const FOnlineSessionSettings& Settings);

	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);

	void TravelToServer(const FString& ServerAddress) const;

private:
	
	FMatchmakingHandle Handle;

	FLevelInformation T_LevelInformation;

	// 玩家控制器
	UPROPERTY()
	APlayerController* PlayerController;

	/* 在线子系统会话接口 */
	IOnlineSessionPtr SessionInt;

	FName MatchmakingSessionName;

	/* 会话查询配置 */
	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	/* 会话查询时间 */
	uint16 SearchingSeconds;
	uint16 LastSearchingTime;

	/* 连接到会话后等待开始游戏的时间(会话的玩家人数未满时) */
	uint8 WaitingStartTime;

};


