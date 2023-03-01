// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LatentActions.h"
#include "AI/DEnemyBase.h"
#include "DEnemyShooter.generated.h"

class ADEnemyShooter;
class UDMShooterWeaponComponent;

USTRUCT()
struct FAIShotParams
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Hostile;

	UPROPERTY()
	uint8 NumberOfShots;
};

class FOpenFireAction : public FPendingLatentAction
{
	
public:
	
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	float Interval;
	int32 NumberOfFire;

	bool bStop;

	TWeakObjectPtr<ADEnemyShooter> Shooter;
	TWeakObjectPtr<AActor> Hostile;

	FOpenFireAction(const FLatentActionInfo& LatentInfo,
					float InInterval,
					int32 InNumberOfFire,
					ADEnemyShooter* InShooter,
					AActor* InHostile)

		: ExecutionFunction(LatentInfo.ExecutionFunction)
		  , OutputLink(LatentInfo.Linkage)
		  , CallbackTarget(LatentInfo.CallbackTarget)
		  , Interval(InInterval)
		  , NumberOfFire(InNumberOfFire)
		  , bStop(false)
		  , Shooter(InShooter)
		  , Hostile(InHostile)
		  , TickCount(0)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override;

private:

	float TickCount;
};

UENUM(BlueprintType)
enum class EOpenFireOption : uint8
{
	Fire,
    StopFire
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnApplyProjectileDamage, const FHitResult&, const FVector&);

/**
 * 
 */
UCLASS(Abstract)
class DREAM_API ADEnemyShooter : public ADEnemyBase, public IProjectileDamageInterface
{
	GENERATED_BODY()

public:

	ADEnemyShooter();
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	FName WeaponMuzzleSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	UAnimMontage* FireAnim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	class USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AISettings)
	FRangeRandomInt RandomFiringRange;

	UPROPERTY(EditDefaultsOnly, Category = AISettings)
	float FirePerMinute;

protected:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "EnemyAI|Shooter")
	bool bDiscoverEnemy;
	
	UPROPERTY()
	UDMShooterWeaponComponent* WeaponTypeComponent;
	
public:

	FOnApplyProjectileDamage OnApplyProjectileDamage;
	
public:

	UFUNCTION(BlueprintCallable, Meta=(Latent, LatentInfo="LatentInfo", ExpandEnumAsExecs="Option"), Category = "EnemyAI|Shooter")
    void OpenFire(AActor* TargetActor, FLatentActionInfo LatentInfo, EOpenFireOption Option);

	void HandleFiring(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "EnemyAI|Shooter")
	void SetDiscoverEnemy(bool bNewDiscoverEnemy);

	UFUNCTION(BlueprintImplementableEvent)
	UMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category = "EnemyAI|Shooter")
	FVector GetWeaponMuzzleLocation() const;
	
protected:

	virtual void ApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin) override;

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
	virtual void OnTargetPerceptionUpdated(AActor* StimulusActor, FAIStimulus Stimulus) override;

	virtual void LostAllHostileTarget() override;
};

