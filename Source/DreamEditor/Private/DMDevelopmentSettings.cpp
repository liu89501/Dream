
#include "DMDevelopmentSettings.h"


USkeleton* UDMDevelopmentSettings::GetPreviewTargetSkeletal() const
{
	return Cast<USkeleton>(WeaponPreviewSkeletal.TryLoad());
}

FName UDMDevelopmentSettings::GetActivePreviewSocketName() const
{
	return ActiveSocketName;
}

FName UDMDevelopmentSettings::GetHolsterPreviewSocketName() const
{
	return HolsterSocketName;
}

const UDMDevelopmentSettings* UDMDevelopmentSettings::Get()
{
	return GetDefault<UDMDevelopmentSettings>();
}
