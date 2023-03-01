#include "DMRollingAsset.h"

UAnimMontage* UDMRollingAsset::GetRollingMontage(ERollingDirection Direction) const
{
	switch (Direction)
	{
	case ERollingDirection::F:		return Forward;
	case ERollingDirection::FL:		return ForwardLeft;
	case ERollingDirection::FR:		return ForwardRight;
	case ERollingDirection::L:		return Left;
	case ERollingDirection::R:		return Right;
	case ERollingDirection::B:		return Backward;
	case ERollingDirection::BL:		return BackwardLeft;
	default:						return BackwardRight;			
	}
}
