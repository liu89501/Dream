// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShootWeapon.h"
#include "Character/DCharacterBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "DEnemyBase.generated.h"

USTRUCT()
struct FAmmunitionDropProbability
{
	GENERATED_BODY()

	FAmmunitionDropProbability()
		: Ammunition_L1(0.5f)
		, Ammunition_L2(0.3f)
		, Ammunition_L3(0.2f)
	{
	}

	/**
	 * L1 弹药的掉落几率
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float Ammunition_L1;

	/**
	* L2 弹药的掉落几率
	*/
	UPROPERTY(EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float Ammunition_L2;

	/**
	* L3 弹药的掉落几率
	*/
	UPROPERTY(EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float Ammunition_L3;

	bool RandomDrawing(EAmmoType& Type);
};

/**
 * 
 */
UCLASS(Abstract)
class DREAM_API ADEnemyBase : public ADCharacterBase, public IAIPerceptionListenerInterface
{
	GENERATED_BODY()

public:

	ADEnemyBase();

	UPROPERTY(VisibleDefaultsOnly)
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleDefaultsOnly)
	class UHealthWidgetComponent* HealthUI;
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DMAISettings)
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = DMAISettings)
	bool bAutoActivateBehaviorTree;
	
	/**
	 * 弹药掉落
	 */
	UPROPERTY(EditAnywhere, Category = DMAISettings)
	FAmmunitionDropProbability AmmunitionDrop;

	UPROPERTY(BlueprintReadOnly)
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DMAISettings)
	float HealthUIShowSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DMAISettings)
	TArray<TSubclassOf<class UGameplayAbility>> OwningAbilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DMAISettings)
	class UDRewardPool* RewardPool;

	/**
	 * 	AI最大的巡逻范围, 相对于 SpawnLocation
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DMAISettings)
	float MaxPatrolRange;
	
	UPROPERTY(EditDefaultsOnly, Category = DMAISettings)
	FName BlackboardName_HostileTarget;

	/**
	* 触发Team感知事件的直径
	*/
	UPROPERTY(EditDefaultsOnly, Category = DMAISettings)
	float NotifyTeamDiameter;

	UPROPERTY(BlueprintReadOnly, Category = DMAISettings)
	float WalkSpeed;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = DMAISettings)
	float JogSpeed;

public:

	class AAIController* GetAIController() const;

	void SetAIGenerator(class ADAIGeneratorBase* Generator);

	UFUNCTION(BlueprintCallable, Category=DreamAI)
	void ActivateBehaviorTree();

	UFUNCTION(BlueprintPure, Category=AIEnemy)
	FRotator GetReplicationControllerRotation() const;

protected:

    //void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void HiddenHealthUI();
	virtual void UpdateHealthUI();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle) override;
	
	virtual UAIPerceptionComponent* GetPerceptionComponent() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ActivateHostile(AActor* Hostile, bool bTriggerTeamStimulus = true);

	virtual void RefreshActiveHostile();

	virtual void LostAllHostileTarget();

	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* StimulusActor, struct FAIStimulus Stimulus);

	virtual void Tick(float DeltaSeconds) override;

protected:

	UPROPERTY()
	ADAIGeneratorBase* OwnerAIGenerator;

	UPROPERTY()
	class AAIController* AIController;

	UPROPERTY(Replicated)
	FVector_NetQuantizeNormal ReplicatedCtrlRotation;

	UPROPERTY(BlueprintReadOnly)
	bool bTurnInProgress;
	
	
private:

	UPROPERTY()
	AActor* FocusHostile;

	FTimerHandle Handle_ShowUI;

	FDelegateHandle HostileDeathHandle;
};
