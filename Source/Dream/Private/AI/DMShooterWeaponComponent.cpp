// Fill out your copyright notice in the Description page of Project Settings.

#include "DMShooterWeaponComponent.h"
#include "DEnemyShooter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UDMShooterWeaponComponent::UDMShooterWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UDMShooterWeaponComponent::OpenFire(AActor* Hostile)
{
}

void UDMShooterWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerAI = Cast<ADEnemyShooter>(GetOwner());
	checkf(OwnerAI, TEXT("Onwer Actor Can Only Be ADEnemyShooter"));
}

void UDMShooterWeaponComponent::PlayAnimAndFX()
{
	if (OwnerAI->FireAnim)
	{
		OwnerAI->PlayAnimMontage(OwnerAI->FireAnim);
	}
	
	UMeshComponent* WeaponMesh = OwnerAI->GetWeaponMesh();

	FVector SocketLocation = WeaponMesh->GetSocketLocation(OwnerAI->WeaponMuzzleSocketName);

	if (OwnerAI->MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAttached(OwnerAI->MuzzleFlash, WeaponMesh, OwnerAI->WeaponMuzzleSocketName);
	}

	if (OwnerAI->FireSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, OwnerAI->FireSound, SocketLocation);
	}
}
