#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "EdGraphUtilities.h"
#include "SGearAttributesPin.h"

class FGearAttributesGraphPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && Pin->PinType.PinSubCategoryObject == FAttributeHandle::StaticStruct())
		{
			return SNew(SGearAttributesPin, Pin);
		}
		return nullptr;
	}
};
