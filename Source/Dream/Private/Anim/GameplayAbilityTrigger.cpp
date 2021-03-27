// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilityTrigger.h"

#include "DGameplayStatics.h"


void UGameplayAbilityTrigger::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}
	FVector RotateVector = MeshComp->GetComponentRotation().RotateVector(EffectRelativeLocation);
	FVector Location = MeshComp->GetComponentLocation() + RotateVector;

	EDrawDebugTrace::Type DrawDebugTrace = EDrawDebugTrace::None;

#if WITH_EDITOR

	if (bDebugSphere)
	{
		DrawDebugTrace = EDrawDebugTrace::ForDuration;
	}
	
#endif
	
	UDGameplayStatics::SphereTraceAndSendEvent(MeshComp->GetOwner(), GameplayEventTag, Location, EffectRadius, EffectTraceType, false, DrawDebugTrace);
}
