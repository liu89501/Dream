// Fill out your copyright notice in the Description page of Project Settings.

#include "DMPreviewCharacter.h"
#include "AbilitySystemComponent.h"
#include "DCharacterPlayer.h"
#include "DGameplayStatics.h"
#include "DMAnimInterface.h"
#include "DMAttributeSet.h"
#include "DMProjectSettings.h"
#include "ShootWeapon.h"
#include "GameFramework/Character.h"

ADMPreviewCharacter::ADMPreviewCharacter()
	: PreviewCharacterMesh(nullptr)
{
	EquippedWeapons.SetNumZeroed(2);
}

void ADMPreviewCharacter::InitPreviewCharacter(int32 InUseWeaponSlot, const FPlayerInfo& PlayerInfo, ADCharacterPlayer* OwnerPlayer)
{
	UseWeaponSlot = InUseWeaponSlot;

	PreviewCharacterMesh = NewObject<USkeletalMeshComponent>(this);
	PreviewCharacterMesh->SetAnimInstanceClass(GSProject->GetMasterAnimClass());
	PreviewCharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	PreviewCharacterMesh->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	
	UpdateCharacterMesh(Cast<UCharacterMesh>(PlayerInfo.CharacterMesh.TryLoad()));

	PreviewCharacterMesh->RegisterComponent();

	FEquipmentAttributes UseWeaponAttr;

	for (const FPlayerWeapon& Weapon: PlayerInfo.Weapons)
	{
		if (Weapon.bEquipped && EquippedWeapons.IsValidIndex(Weapon.Index))
		{
			UClass* Class = GSProject->GetItemClassFromGuid(Weapon.ItemGuid);

			AShootWeapon* ShootWeapon = GetWorld()->SpawnActor<AShootWeapon>(Class);
			ShootWeapon->SetReplicates(false);

			bool bActive = Weapon.Index == UseWeaponSlot;
			ShootWeapon->AttachToCharacter(bActive, PreviewCharacterMesh);
			EquippedWeapons[Weapon.Index] = ShootWeapon;

			if (bActive)
			{
				UseWeaponAttr = Weapon.Attributes;
			}
		}
	}

	if (OwnerPlayer)
	{
		UAbilitySystemComponent* OwnerAbilitySystem = OwnerPlayer->GetAbilitySystemComponent();
		CharacterAttributes.AttackPower = OwnerAbilitySystem->GetNumericAttribute(DMAttrStatics().AttackPowerProperty);
		CharacterAttributes.CriticalDamage = OwnerAbilitySystem->GetNumericAttribute(DMAttrStatics().CriticalDamageProperty);
		CharacterAttributes.CriticalRate = OwnerAbilitySystem->GetNumericAttribute(DMAttrStatics().CriticalRateProperty);
		CharacterAttributes.Defense = OwnerAbilitySystem->GetNumericAttribute(DMAttrStatics().DefensePowerProperty);
		CharacterAttributes.HealthSteal = OwnerAbilitySystem->GetNumericAttribute(DMAttrStatics().HealthStealProperty);
		CharacterAttributes.MaxHealth = OwnerAbilitySystem->GetNumericAttribute(DMAttrStatics().MaxHealthProperty);
	}
	else
	{
		for (const FPlayerModule& Module : PlayerInfo.Modules)
		{
			if (Module.bEquipped)
			{
				CharacterAttributes.MergeAndSkipPerks(Module.Attributes);
			}
		}

		CharacterAttributes.MergeAndSkipPerks(UseWeaponAttr);
	}

	UAnimInstance* AnimInstance = PreviewCharacterMesh->GetAnimInstance();
	if (AnimInstance->GetClass()->ImplementsInterface(UDMAnimInterface::StaticClass()))
	{
		if (EquippedWeapons.IsValidIndex(UseWeaponSlot))
		{
			IDMAnimInterface::Execute_SetOverlayDetailState(AnimInstance, EquippedWeapons[UseWeaponSlot]->AnimID);
		}
	}
	
	PreviewCharacterMesh->PrestreamTextures(1, true);
}

void ADMPreviewCharacter::UpdatePreviewWeapon(TSubclassOf<AShootWeapon> WeaponClass, int32 NewIndex)
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
		if (AnimInstance->GetClass()->ImplementsInterface(UDMAnimInterface::StaticClass()))
		{
			IDMAnimInterface::Execute_SetOverlayDetailState(AnimInstance, ShootWeapon->AnimID);
		}
	}

	ShootWeapon->PrestreamTextures(1, true);
}

USkeletalMeshComponent* ADMPreviewCharacter::GetCharacterMeshComponent() const
{
	return PreviewCharacterMesh;
}

void ADMPreviewCharacter::DestroyComponentBySocketName(const FName& SocketName)
{
	USceneComponent* Component = UDGameplayStatics::GetAttachComponentFromSocketName(PreviewCharacterMesh, SocketName);
	if (Component != nullptr)
	{
		Component->DestroyComponent();
	}
}

void ADMPreviewCharacter::UpdateCharacterMesh(UCharacterMesh* CharacterMesh)
{
	PreviewCharacterMesh->SetSkeletalMesh(CharacterMesh->MasterMesh);

	const TArray<USceneComponent*>& AttachChildren = PreviewCharacterMesh->GetAttachChildren();
	for (int16 Idx = AttachChildren.Num() - 1; Idx > 0; Idx--)
	{
		USceneComponent* const Component = AttachChildren[Idx];
		if (Component && Component->GetOwner() == this)
		{
			Component->DestroyComponent();
		}
	}
        
	for (USkeletalMesh* SKMesh : CharacterMesh->SlaveMeshs)
	{
		USkeletalMeshComponent* SKComponent = NewObject<USkeletalMeshComponent>(this);
		SKComponent->RegisterComponent();
		SKComponent->SetSkeletalMesh(SKMesh);
		SKComponent->SetAnimClass(GSProject->GetSlaveAnimClass());
		SKComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SKComponent->AttachToComponent(PreviewCharacterMesh, FAttachmentTransformRules::KeepRelativeTransform);
	}

	PreviewCharacterMesh->PrestreamTextures(1, true);
}

int32 ADMPreviewCharacter::GetActiveWeaponAnimID() const
{
	if (EquippedWeapons.IsValidIndex(UseWeaponSlot))
	{
		return EquippedWeapons[UseWeaponSlot]->AnimID;
	}
	return 0;
}

int32 ADMPreviewCharacter::GetActiveWeaponIndex() const
{
	return UseWeaponSlot;
}

const FEquipmentAttributes& ADMPreviewCharacter::GetCharacterAttributes() const
{
	return CharacterAttributes;
}
