// Fill out your copyright notice in the Description page of Project Settings.


#include "DCharacterPreview.h"

#include "CharacterPreviewAnimInstance.h"
#include "DCharacterPlayer.h"
#include "DGameplayStatics.h"
#include "GameFramework/Character.h"

void ADCharacterPreview::InitialPreviewCharacter(ADCharacterPlayer* Character)
{
	OwningCharacter = Character;
	USkeletalMeshComponent* NewMeshComponent = AddPreviewSkeletalMeshFromComponent(Character->GetMesh());

	UAnimInstance* AnimInstance = NewMeshComponent->GetAnimInstance();
	if (AnimInstance->GetClass()->ImplementsInterface(UCharacterPreviewAnimInstance::StaticClass()))
	{
		if (AShootWeapon* ActiveWeapon = OwningCharacter->GetActiveWeapon())
		{
			ICharacterPreviewAnimInstance::Execute_UpdateWeaponAnimID(AnimInstance, ActiveWeapon->AnimID);
		}
	}
}

void ADCharacterPreview::UpdatePreviewWeapon(TSubclassOf<AShootWeapon> WeaponClass, int32 NewIndex)
{
	bool bActiveWeapon = NewIndex == OwningCharacter->GetActiveWeaponIndex();
	
	USkeletalMeshComponent* CharacterMeshComponent = GetCharacterMeshComponent();
	FName& SocketName = bActiveWeapon ? OwningCharacter->WeaponSocketName : OwningCharacter->WeaponHolsterSocketName;
	DestroyComponentBySocketName(SocketName);
	
	AShootWeapon* ShootWeapon = GetWorld()->SpawnActor<AShootWeapon>(WeaponClass);
	ShootWeapon->AttachToComponent(CharacterMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	ShootWeapon->SetActorRelativeTransform(bActiveWeapon ? ShootWeapon->WeaponSocketOffset : ShootWeapon->WeaponHolsterSocketOffset);

	if (bActiveWeapon)
	{
		UAnimInstance* AnimInstance = CharacterMeshComponent->GetAnimInstance();
		if (AnimInstance->GetClass()->ImplementsInterface(UCharacterPreviewAnimInstance::StaticClass()))
		{
			ICharacterPreviewAnimInstance::Execute_UpdateWeaponAnimID(AnimInstance, ShootWeapon->AnimID);
		}
	}
}

USkeletalMeshComponent* ADCharacterPreview::GetCharacterMeshComponent() const
{
	return Cast<USkeletalMeshComponent>(PreviewActorRoot->GetChildComponent(0));
}

void ADCharacterPreview::DestroyComponentBySocketName(const FName& SocketName)
{
	if (USceneComponent* Component = UDGameplayStatics::GetAttachComponentFromSocketName(GetCharacterMeshComponent(), SocketName))
	{
		Component->DestroyComponent(true);
	}
}

void ADCharacterPreview::UpdateCharacterMesh(UCharacterMesh* CharacterMesh)
{
	USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent();
	MeshComponent->SetSkeletalMesh(CharacterMesh->MasterMesh);

	static UClass* SlaveMeshAnimBPClass = LoadClass<UAnimInstance>(this, TEXT("/Game/Animation/AnimBP_CharacterCopy.AnimBP_CharacterCopy_C"));

	const TArray<USceneComponent*> AttachComponents = MeshComponent->GetAttachChildren();
	for (USceneComponent* Child : AttachComponents)
	{
		if (Child && Child->ComponentHasTag(SKIN_COMP_NAME))
		{
			Child->DestroyComponent();
		}
	}
        
	for (USkeletalMesh* SKMesh : CharacterMesh->SlaveMeshs)
	{
		USkeletalMeshComponent* SKComponent = NewObject<USkeletalMeshComponent>(this);
		SKComponent->RegisterComponent();
		SKComponent->SetSkeletalMesh(SKMesh);
		SKComponent->SetAnimClass(SlaveMeshAnimBPClass);
		SKComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SKComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		SKComponent->ComponentTags.Add(SKIN_COMP_NAME);
	}
}

int32 ADCharacterPreview::GetActiveWeaponAnimID() const
{
	if (AShootWeapon* ActiveWeapon = OwningCharacter->GetActiveWeapon())
	{
		return ActiveWeapon->AnimID;
	}
	return 0;
}

int32 ADCharacterPreview::GetActiveWeaponIndex() const
{
	return OwningCharacter->GetActiveWeaponIndex();
}
