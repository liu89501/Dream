// Fill out your copyright notice in the Description page of Project Settings.


#include "SurfaceImpactAsset.h"

const FSurfaceImpactEffect& USurfaceImpactAsset::GetImpactEffect(EPhysicalSurface Surface)
{
	switch (Surface)
	{
		case SurfaceType_Asphalt:		return ImpactEffects.Asphalt;
		case SurfaceType_Bark:			return ImpactEffects.Bark;
		case SurfaceType_Brick:			return ImpactEffects.Brick;
		case SurfaceType_CarbonFibre:	return ImpactEffects.CarbonFibre;
		case SurfaceType_Ceramic:		return ImpactEffects.Ceramic;
		case SurfaceType_Concrete:		return ImpactEffects.Concrete;
		case SurfaceType_Dirt:			return ImpactEffects.Dirt;
		case SurfaceType_Dirt2:			return ImpactEffects.Dirt2;
		case SurfaceType_Acid:			return ImpactEffects.Acid;
		case SurfaceType_Electrical:	return ImpactEffects.Electrical;		
		case SurfaceType_EnergyShield:	return ImpactEffects.EnergyShield;
		case SurfaceType_Fabric:		return ImpactEffects.Fabric;
		case SurfaceType_Flesh:			return ImpactEffects.Flesh;
		case SurfaceType_Glass:			return ImpactEffects.Glass;
		case SurfaceType_Grass:			return ImpactEffects.Grass;
		case SurfaceType_Gravel:		return ImpactEffects.Gravel;
		case SurfaceType_Ice:			return ImpactEffects.Ice;
		case SurfaceType_Lava:			return ImpactEffects.Lava;
		case SurfaceType_Leather:		return ImpactEffects.Leather;
		case SurfaceType_Limestone:		return ImpactEffects.Limestone;
		case SurfaceType_Metal:			return ImpactEffects.Metal;
		case SurfaceType_MetalDull:		return ImpactEffects.MetalDull;
		case SurfaceType_Mud:			return ImpactEffects.Mud;
		case SurfaceType_Oil:			return ImpactEffects.Oil;
		case SurfaceType_Paper:			return ImpactEffects.Paper;
		case SurfaceType_Plastic:		return ImpactEffects.Plastic;
		case SurfaceType_Polystyrene:	return ImpactEffects.Polystyrene;		
		case SurfaceType_Powder:		return ImpactEffects.Powder;
		case SurfaceType_Rubber:		return ImpactEffects.Rubber;
		case SurfaceType_Sand:			return ImpactEffects.Sand;
		case SurfaceType_Snow:			return ImpactEffects.Snow;
		case SurfaceType_Soil:			return ImpactEffects.Soil;
		case SurfaceType_Vegetation:	return ImpactEffects.Vegetation;				
		case SurfaceType_Water:			return ImpactEffects.Water;
		case SurfaceType_Wood:			return ImpactEffects.Wood;
		case SurfaceType_Rock:			return ImpactEffects.Rock;
		default:						return ImpactEffects.Default;
	}
}
