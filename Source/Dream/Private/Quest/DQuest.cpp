
#include "Quest/DQuest.h"
#include "Quest/DQuestCondition.h"


void UDQuest::Update(const FQuestActionHandle& Handle) const
{
	Condition->UpdateCondition(Handle);
}
