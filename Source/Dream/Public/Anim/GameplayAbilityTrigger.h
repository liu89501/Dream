// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayAbilityTrigger.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UGameplayAbilityTrigger : public UAnimNotify
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category=AbilityTrigger)
	FGameplayTag GameplayEventTag;

	UPROPERTY(EditAnywhere, Category=AbilityTrigger)
	float EffectRadius;

	UPROPERTY(EditAnywhere, Category=AbilityTrigger)
	TEnumAsByte<ETraceTypeQuery> EffectTraceType;

	/**
	 * 影响的位置。 相对于Character自身
	 */
	UPROPERTY(EditAnywhere, Category=AbilityTrigger)
	FVector EffectRelativeLocation;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Category=AbilityTrigger)
	bool bDebugSphere;
	
#endif
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
