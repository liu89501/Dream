#include "DMMantleAsset.h"

const FMantleInformation& UDMMantleAsset::GetMantleInfo(EMantleType MantleType, EOverlayState OverlayState) const
{
	if (MantleType == EMantleType::LowMantle)
	{
		switch (OverlayState)
		{
		case EOverlayState::Injured:
		case EOverlayState::HandsTied:
			return Mantle1m2H;

		case EOverlayState::Rifle:
		case EOverlayState::Pistol1H:
		case EOverlayState::Pistol2H:
			return Mantle1mRH;

		case EOverlayState::Bow:
		case EOverlayState::Torch:
		case EOverlayState::Barrel:
			return Mantle1mLH;

		case EOverlayState::Binoculars:
			return Mantle1mRH;

		case EOverlayState::Box:
			return Mantle1mBox;

		default:
			return Mantle1mDefault;
		}
	}

	return Mantle2mDefault;
}
