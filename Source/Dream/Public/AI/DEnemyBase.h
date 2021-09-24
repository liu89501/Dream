// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "ShootWeapon.h"
#include "Character/DCharacterBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "DEnemyBase.generated.h"

USTRUCT(BlueprintType)
struct FRewardProbability
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Probability;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced)
	class UItemData* Reward;
};

USTRUCT(BlueprintType)
struct FRewardProbabilityList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FRewardProbability> RewardList;
};

USTRUCT(BlueprintType)
struct FReward
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FRewardProbabilityList> Rewards;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<TSubclassOf<class ADreamDropProps>, float> AmmunitionReward;
};

/**
 * 
 */
UCLASS()
class DREAM_API ADEnemyBase : public ADCharacterBase, public IAIPerceptionListenerInterface
{
	GENERATED_BODY()

public:

	ADEnemyBase();

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere)
	class UGameplayTasksComponent* TasksComponent;

	UPROPERTY(VisibleAnywhere)
	class UHealthWidgetComponent* HealthUI;
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	class UBehaviorTree* BehaviorTree;
	
	/**
	 * 被击杀时的奖励
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	FReward Reward;

	UPROPERTY(EditAnywhere, Category = "AISettings|Abilities")
	UDataTable* DefaultAttributes;

	UPROPERTY(BlueprintReadOnly)
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	float HealthUIShowSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	TArray<TSubclassOf<class UGameplayAbility>> OwningAbilities;

	/**
	 * 	AI最大的巡逻范围, 相对于 SpawnLocation
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	float MaxPatrolRange;
	
	UPROPERTY(EditDefaultsOnly, Category = AISettings)
	FName BlackboardName_HostileTarget;

	/**
	* 触发Team感知事件的直径
	*/
	UPROPERTY(EditDefaultsOnly, Category = AISettings)
	float NotifyTeamDiameter;

	UPROPERTY(BlueprintReadOnly, Category = AISettings)
	float WalkSpeed;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = AISettings)
	float JogSpeed;

public:

	class AAIController* GetAIController() const;

	void SetAIGenerator(class ADAIGenerator* Generator);

protected:

    //void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void HiddenHealthUI();
	virtual void UpdateHealthUI();

	virtual void BeginPlay() override;

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle) override;
	
	virtual UAIPerceptionComponent* GetPerceptionComponent() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ActivateHostile(ADCharacterBase* Hostile, bool bTriggerTeamStimulus = true);

	virtual void RefreshActiveHostile();

	virtual void LostAllHostileTarget();

	virtual void OnTargetPerceptionUpdated(ADCharacterBase* StimulusPawn, struct FAIStimulus Stimulus);

	virtual void Tick(float DeltaSeconds) override;

	void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

protected:

	UPROPERTY()
	ADAIGenerator* OwnerAIGenerator;

	UPROPERTY()
	class AAIController* AIController;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float CtrlYaw;

	UPROPERTY(BlueprintReadOnly)
	bool bTurnInProgress;

private:

	FTimerHandle Handle_ShowUI;

	FDelegateHandle HostileDeathHandle;

	UFUNCTION()
    void OnTargetPerceptionUpdated0(AActor* Actor, FAIStimulus Stimulus);

	void OnRewardsAddCompleted(const FString& ErrorMessage, TMap<class ADPlayerController*, TArray<UItemData*>> Rewards);
	
};
