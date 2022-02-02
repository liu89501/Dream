#pragma once

#include "CoreMinimal.h"
#include "DMDevelopmentSettings.generated.h"

UCLASS(Config=Editor, defaultconfig)
class UDMDevelopmentSettings : public UObject
{
	GENERATED_BODY()

public:

	USkeleton* GetPreviewTargetSkeletal() const;

	FName GetActivePreviewSocketName() const;
	
	FName GetHolsterPreviewSocketName() const;

	static const UDMDevelopmentSettings* Get();

private:

	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "Skeleton"), Category=WeaponPreview)
	FSoftObjectPath WeaponPreviewSkeletal;

	UPROPERTY(Config, EditAnywhere, Category=WeaponPreview)
	FName ActiveSocketName;
	
	UPROPERTY(Config, EditAnywhere, Category=WeaponPreview)
	FName HolsterSocketName;
};