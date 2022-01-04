#include "DCharacterPlayerBase.h"

ADCharacterPlayerBase::ADCharacterPlayerBase()
{
}

AShootWeapon* ADCharacterPlayerBase::GetActiveWeapon() const
{
}

int32 ADCharacterPlayerBase::GetActiveWeaponIndex() const
{
}

ADPlayerController* ADCharacterPlayerBase::GetPlayerController() const
{
}

void ADCharacterPlayerBase::EquipWeapon(int32 Index, const FEquipmentAttributes& Attrs,
	TSubclassOf<AShootWeapon> WeaponClass)
{
}

void ADCharacterPlayerBase::EquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs)
{
}

void ADCharacterPlayerBase::LearningTalents(const TArray<TSubclassOf<UDreamGameplayAbility>>& TalentClasses)
{
}

void ADCharacterPlayerBase::SwitchWeapon(int32 NewWeaponIndex)
{
}

void ADCharacterPlayerBase::SetCharacterMesh(UCharacterMesh* CharacterMesh)
{
}

void ADCharacterPlayerBase::SetCameraFieldOfView(float NewFOV)
{
}

void ADCharacterPlayerBase::CameraAimTransformLerp(float Alpha)
{
}

void ADCharacterPlayerBase::TriggerAbilityFromTag(const FGameplayTag& Tag, AActor* Target)
{
}

void ADCharacterPlayerBase::OnRep_ActiveWeapon()
{
}

void ADCharacterPlayerBase::SwitchWeapon()
{
}

void ADCharacterPlayerBase::HandleSwitchWeapon(int32 WeaponIndex)
{
}

void ADCharacterPlayerBase::SwitchFinished(int32 WeaponIndex)
{
}

void ADCharacterPlayerBase::ServerSwitchWeapon_Implementation()
{
}

void ADCharacterPlayerBase::MulticastSwitchWeapon_Implementation()
{
}

void ADCharacterPlayerBase::SwitchWeaponToFirst()
{
}

void ADCharacterPlayerBase::SwitchWeaponToSecond()
{
}

void ADCharacterPlayerBase::ServerEquipWeapon_Implementation(int32 Index, const FEquipmentAttributes& Attrs,
	TSubclassOf<AShootWeapon> NewWeaponClass)
{
}

void ADCharacterPlayerBase::DoServerEquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, UClass* NewWeaponClass)
{
}

void ADCharacterPlayerBase::ServerEquipModule_Implementation(TSubclassOf<UDModuleBase> ModuleClass,
	const FEquipmentAttributes& Attrs)
{
}

void ADCharacterPlayerBase::DoServerEquipModule(UClass* ModuleClass, const FEquipmentAttributes& Attrs)
{
}

void ADCharacterPlayerBase::ServerLearningTalent_Implementation(
	const TArray<TSubclassOf<UDreamGameplayAbility>>& TalentClasses)
{
}

void ADCharacterPlayerBase::PostInitializeComponents()
{
}

void ADCharacterPlayerBase::BeginPlay()
{
}

void ADCharacterPlayerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void ADCharacterPlayerBase::SetActiveWeapon(AShootWeapon* NewWeapon)
{
}

void ADCharacterPlayerBase::Destroyed()
{
}

void ADCharacterPlayerBase::OnInitPlayer(const FPlayerInfo& PlayerInfo, bool bSuccess)
{
}

void ADCharacterPlayerBase::OnPlayerPropertiesChanged(const FPlayerProperties& Properties)
{
}

void ADCharacterPlayerBase::AdditiveAttributes(const FEquipmentAttributes& Attributes)
{
}

void ADCharacterPlayerBase::RefreshAttributeBaseValue()
{
}

void ADCharacterPlayerBase::ServerInitializePlayer_Implementation(const FPlayerInfo& PlayerInfo)
{
}

void ADCharacterPlayerBase::ServerUpdateCharacterMesh_Implementation(UCharacterMesh* CharacterMesh)
{
}

void ADCharacterPlayerBase::OnRep_CharacterMesh()
{
}

void ADCharacterPlayerBase::UpdateCharacterMesh()
{
}
