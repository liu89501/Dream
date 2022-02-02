// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DAIGeneratorBase.generated.h"

UCLASS()
class DREAM_API ADAIGeneratorBase : public AActor
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllAIDestroyed, ADAIGeneratorBase*, Generator);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAIDestroyed);
	
public:

	ADAIGeneratorBase();

public:

	UPROPERTY(VisibleDefaultsOnly)
	class UBillboardComponent* Billboard;

	/**
	 * 由自己生成的Ai全部死亡后重生所等待的时间
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AIGenerator)
	float ResetWaitTime;

	/**
	 *  AI重生的次数, 为-1时将无限刷新AI
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AIGenerator)
	int32 ReviveFrequency;

	/**
	 *  是否自动激活AI生成器(关卡创建时就生成AI)
	 */
	UPROPERTY(EditAnywhere, Category=AIGenerator)
	bool bAutoActive;

	UPROPERTY(BlueprintAssignable, Category=AIGenerator)
	FOnAllAIDestroyed OnAllAIDestroyed;

	/** 与 OnAllAIDestroyed 一样 只不过这个不带参数而已 */
	UPROPERTY(BlueprintAssignable, Category=AIGenerator)
	FOnAIDestroyed OnAIDestroyed;

public:

	UFUNCTION(BlueprintCallable, Category=AIGenerator)
	void ActivateAIGenerator();

	void AIDeathCount();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual int32 ProcessAIGenerate();

	void ReGenerate();

private:
	
	uint16 ActiveAICounter;

	int32 CurrentRevivedCount;
	
};
