// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_HideWeapon.h"
#include "DCharacterPlayer.h"
#include "ShootWeapon.h"


FString UAnimNotify_HideWeapon::GetNotifyName_Implementation() const
{
	static FString NotifyName = TEXT("HideWeapon");
	return NotifyName;
}

void UAnimNotify_HideWeapon::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration)
{
	if (ADCharacterPlayer* OwningPlayer = Cast<ADCharacterPlayer>(MeshComp->GetOwner()))
	{
		if (AShootWeapon* ActiveWeapon = OwningPlayer->GetActiveWeapon())
		{
			ActiveWeapon->SetActorHiddenInGame(true);
		}
	}
}

void UAnimNotify_HideWeapon::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (ADCharacterPlayer* OwningPlayer = Cast<ADCharacterPlayer>(MeshComp->GetOwner()))
	{
		if (AShootWeapon* ActiveWeapon = OwningPlayer->GetActiveWeapon())
		{
			ActiveWeapon->SetActorHiddenInGame(false);
		}
	}
}
