// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "Engine/DataAsset.h"
#include "SurfaceImpactAsset.generated.h"

#define SurfaceType_Asphalt SurfaceType1
#define SurfaceType_Bark SurfaceType2
#define SurfaceType_Brick SurfaceType3
#define SurfaceType_CarbonFibre SurfaceType4
#define SurfaceType_Ceramic SurfaceType5
#define SurfaceType_Concrete SurfaceType6
#define SurfaceType_Dirt SurfaceType7
#define SurfaceType_Dirt2 SurfaceType8
#define SurfaceType_Acid SurfaceType9
#define SurfaceType_Electrical SurfaceType10
#define SurfaceType_EnergyShield SurfaceType11
#define SurfaceType_Fabric SurfaceType12
#define SurfaceType_Flesh SurfaceType13
#define SurfaceType_Glass SurfaceType14
#define SurfaceType_Grass SurfaceType15
#define SurfaceType_Gravel SurfaceType16
#define SurfaceType_Ice SurfaceType17
#define SurfaceType_Lava SurfaceType18
#define SurfaceType_Leather SurfaceType19
#define SurfaceType_Limestone SurfaceType20
#define SurfaceType_Metal SurfaceType21
#define SurfaceType_MetalDull SurfaceType22
#define SurfaceType_Mud SurfaceType23
#define SurfaceType_Oil SurfaceType24
#define SurfaceType_Paper SurfaceType25
#define SurfaceType_Plastic SurfaceType26
#define SurfaceType_Polystyrene SurfaceType27
#define SurfaceType_Powder SurfaceType28
#define SurfaceType_Rubber SurfaceType29
#define SurfaceType_Sand SurfaceType30
#define SurfaceType_Snow SurfaceType31
#define SurfaceType_Soil SurfaceType32
#define SurfaceType_Vegetation SurfaceType33
#define SurfaceType_Water SurfaceType34
#define SurfaceType_Wood SurfaceType35
#define SurfaceType_Rock SurfaceType36

USTRUCT()
struct FSurfaceImpactEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Default;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Asphalt;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Bark;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Brick;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect CarbonFibre;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Ceramic;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Concrete;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Dirt;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Dirt2;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Acid;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Electrical;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect EnergyShield;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Fabric;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Flesh;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Glass;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Grass;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Gravel;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Ice;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Lava;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Leather;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Limestone;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Metal;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect MetalDull;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Mud;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Oil;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Paper;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Plastic;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Polystyrene;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Powder;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Rubber;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Sand;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Snow;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Soil;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Vegetation;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Water;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Wood;

	UPROPERTY(EditAnywhere)
	FSurfaceImpactEffect Rock;

};

/**
 * 
 */
UCLASS()
class DREAM_API USurfaceImpactAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = ProjectSettings)
	FSurfaceImpactEffects ImpactEffects;

public:

	const FSurfaceImpactEffect& GetImpactEffect(EPhysicalSurface Surface);
};
