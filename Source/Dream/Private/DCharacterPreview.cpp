// Fill out your copyright notice in the Description page of Project Settings.


#include "DCharacterPreview.h"
#include "CharacterPreviewAnimInstance.h"
#include "DCharacterPlayer.h"
#include "DGameplayStatics.h"
#include "DProjectSettings.h"
#include "GameFramework/Character.h"

ADCharacterPreview::ADCharacterPreview()
{
	EquippedWeapons.SetNumZeroed(2);
}

void ADCharacterPreview::InitPreviewCharacterFromPlayerInfo(int32 InUseWeaponSlot, const FPlayerInfo& PlayerInfo)
{
	UseWeaponSlot = InUseWeaponSlot;
	UDProjectSettings* ProjectSettings = UDProjectSettings::GetProjectSettings();

	PreviewCharacterMesh = NewObject<USkeletalMeshComponent>(this);
	PreviewCharacterMesh->SetAnimInstanceClass(ProjectSettings->GetMasterAnimClass());
	PreviewCharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	PreviewCharacterMesh->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	
	UpdateCharacterMesh(Cast<UCharacterMesh>(PlayerInfo.CharacterMesh.TryLoad()));

	PreviewCharacterMesh->RegisterComponent();

	for (const FPlayerWeapon& Weapon: PlayerInfo.Weapons)
	{
		if (Weapon.bEquipped && EquippedWeapons.IsValidIndex(Weapon.Index))
		{
			UClass* Class = ProjectSettings->GetItemClassFromGuid(Weapon.ItemGuid);

			AShootWeapon* ShootWeapon = GetWorld()->SpawnActor<AShootWeapon>(Class);
			ShootWeapon->AttachToCharacter(Weapon.Index == UseWeaponSlot, PreviewCharacterMesh);
			EquippedWeapons[Weapon.Index] = ShootWeapon;
		}
	}

	UAnimInstance* AnimInstance = PreviewCharacterMesh->GetAnimInstance();
	if (AnimInstance->GetClass()->ImplementsInterface(UCharacterPreviewAnimInstance::StaticClass()))
	{
		if (EquippedWeapons.IsValidIndex(UseWeaponSlot))
		{
			ICharacterPreviewAnimInstance::Execute_UpdateWeaponAnimID(AnimInstance, EquippedWeapons[UseWeaponSlot]->AnimID);
		}
	}
	
	PreviewCharacterMesh->PrestreamTextures(1, true);
}

void ADCharacterPreview::UpdatePreviewWeapon(TSubclassOf<AShootWeapon> WeaponClass, int32 NewIndex)
{
	if (EquippedWeapons.IsValidIndex(NewIndex))
	{
		EquippedWeapons[NewIndex]->Destroy();
	}

	bool bActiveWeapon = NewIndex == UseWeaponSlot;
	
	AShootWeapon* ShootWeapon = GetWorld()->SpawnActor<AShootWeapon>(WeaponClass);
	ShootWeapon->AttachToCharacter(bActiveWeapon, PreviewCharacterMesh);

	EquippedWeapons[NewIndex] = ShootWeapon;

	if (bActiveWeapon)
	{
		UAnimInstance* AnimInstance = PreviewCharacterMesh->GetAnimInstance();
		if (AnimInstance->GetClass()->ImplementsInterface(UCharacterPreviewAnimInstance::StaticClass()))
		{
			ICharacterPreviewAnimInstance::Execute_UpdateWeaponAnimID(AnimInstance, ShootWeapon->AnimID);
		}
	}

	ShootWeapon->PrestreamTextures(1, true);
}

USkeletalMeshComponent* ADCharacterPreview::GetCharacterMeshComponent() const
{
	return PreviewCharacterMesh;
}

void ADCharacterPreview::DestroyComponentBySocketName(const FName& SocketName)
{
	USceneComponent* Component = UDGameplayStatics::GetAttachComponentFromSocketName(PreviewCharacterMesh, SocketName);
	if (Component != nullptr)
	{
		Component->DestroyComponent();
	}
}

void ADCharacterPreview::UpdateCharacterMesh(UCharacterMesh* CharacterMesh)
{
	PreviewCharacterMesh->SetSkeletalMesh(CharacterMesh->MasterMesh);

	const TArray<USceneComponent*> AttachComponents = PreviewCharacterMesh->GetAttachChildren();
	for (USceneComponent* Child : AttachComponents)
	{
		if (Child->GetOwner() == this)
		{
			Child->DestroyComponent();
		}
	}
        
	for (USkeletalMesh* SKMesh : CharacterMesh->SlaveMeshs)
	{
		USkeletalMeshComponent* SKComponent = NewObject<USkeletalMeshComponent>(this);
		SKComponent->RegisterComponent();
		SKComponent->SetSkeletalMesh(SKMesh);
		SKComponent->SetAnimClass(UDProjectSettings::GetProjectSettings()->GetSlaveAnimClass());
		SKComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SKComponent->AttachToComponent(PreviewCharacterMesh, FAttachmentTransformRules::KeepRelativeTransform);
	}

	PreviewCharacterMesh->PrestreamTextures(1, true);
}

int32 ADCharacterPreview::GetActiveWeaponAnimID() const
{
	if (EquippedWeapons.IsValidIndex(UseWeaponSlot))
	{
		return EquippedWeapons[UseWeaponSlot]->AnimID;
	}
	return 0;
}

int32 ADCharacterPreview::GetActiveWeaponIndex() const
{
	return UseWeaponSlot;
}
