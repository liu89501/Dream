// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DQuestCondition.generated.h"

USTRUCT()
struct FQuestAction
{
	GENERATED_BODY()

	virtual ~FQuestAction() = default;
	
	virtual UScriptStruct* GetStructType() const
	{
		return StaticStruct();
	}
};

USTRUCT()
struct FQuestAction_KilledTarget : public FQuestAction
{
	GENERATED_BODY()

	FQuestAction_KilledTarget() : TargetClass(nullptr)
	{
		
	}

	explicit FQuestAction_KilledTarget(UClass* InTargetClass)
        : TargetClass(InTargetClass)
	{
	}

	virtual UScriptStruct* GetStructType() const override
	{
		return StaticStruct();
	}

	UClass* GetTargetClass() const
	{
		return TargetClass;
	}

private:

	UPROPERTY()
	UClass* TargetClass;
};


USTRUCT()
struct FQuestActionHandle
{
	GENERATED_BODY()

public:

	FQuestActionHandle() : ActionData(nullptr)
	{
	}

	FQuestActionHandle(FQuestAction* InData)
	{
		ActionData = TSharedPtr<FQuestAction>(InData);
	}
	
	FQuestAction* GetData() const
	{
		return ActionData.Get();
	}

private:

	TSharedPtr<FQuestAction> ActionData;
};

UCLASS(EditInlineNew)
class DREAM_API UDQuestCondition : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ConditionDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* ConditionIcon;

	virtual float GetQuestProgressPercent() const;

	virtual void UpdateCondition(const FQuestActionHandle& Handle);

	virtual bool IsCompleted() const;
	
};

UCLASS()
class DREAM_API UDQuestCondition_KillTarget : public UDQuestCondition
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 KillNum;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> TargetType;

	virtual void UpdateCondition(const FQuestActionHandle& Handle) override;

	virtual float GetQuestProgressPercent() const override;

	virtual bool IsCompleted() const override;

private:

	int32 CurrentKilled;
};
