#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "K2Node_BaseAsyncTask.h"
#include "K2Node_GetPlayerInfo.generated.h"

UCLASS()
class DREAMEDITOR_API UK2Node_GetPlayerInfo : public UK2Node_BaseAsyncTask
{
	GENERATED_BODY()

public:

	UK2Node_GetPlayerInfo();

	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	virtual FText GetMenuCategory() const override;
	//~ End UK2Node Interface
};

