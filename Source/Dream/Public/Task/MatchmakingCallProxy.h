// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "MatchmakingCallProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionJoinNumberChange, int32, JoinNumber);

class APlayerController;

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

	UPROPERTY(BlueprintAssignable)
	FSessionJoinNumberChange OnJoinChanged;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Dream|Async")
	static UMatchmakingCallProxy* Matchmaking(
		APlayerController* PlayerController, 
		const FName& InMapName,
		const FString& InGameMode,
		FMatchmakingHandle& OutHandle
	);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

	void ClearAllHandle();

private:

	void FindLobby();

	void CreateLobby();

	// Internal callback when the join completes, calls out to the public success/failure callbacks
	void OnSearchCompleted(bool bSuccessful);

	void OnCreateSessionCompleted(FName SessionName, bool bSuccessful);

	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type CompleteResult);

	void OnCreateServerComplete(const FString& ServerAddress, const FString& ErrorMessage);

	void OnSessionTick(float DeltaTime);

	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	void JoinLobbyGameServer(const FName& SessionName);

	FName MatchMapName;
	FString GameModeAlias;
	
	TWeakObjectPtr<UWorld> World;

	// 玩家控制器
	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	/* 在线子系统会话接口 */
	IOnlineSessionPtr SessionInt;

	/* 会话查询配置 */
	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	/* 匹配期间可能会用到的所有任务句柄 */
	FTimerHandle Handle_RetryMatching;
	FTimerHandle Handle_SessionTIck;
	FDelegateHandle Handle_UpdateSessionsComplete;
	FDelegateHandle Handle_FindSessionsComplete;
	FDelegateHandle Handle_JoinSessionComplete;
	FDelegateHandle Handle_StartSessionComplete;
	FDelegateHandle Handle_CreateSessionComplete;

	/* 查询会话失败时的重试次数 */
	uint8 FindRetryCount;

	/* 连接到会话后等待开始游戏的时间(会话的玩家人数未满时) */
	float WaitStartTime;

	bool bCreatingServer;
	
	int32 PrevJoinedPlayerNum;
};

