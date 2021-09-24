
#include "Quest/DQuestCondition.h"

float UDQuestCondition::GetQuestProgressPercent() const
{
	return 0.f;
}

void UDQuestCondition::UpdateCondition(const FQuestActionHandle& Handle)
{
}

bool UDQuestCondition::IsCompleted() const
{
	return false;
}

void UDQuestCondition_KillTarget::UpdateCondition(const FQuestActionHandle& Handle)
{
	if (CurrentKilled >= KillNum)
	{
		return;
	}
	
	if (FQuestAction* QuestAction = Handle.GetData())
	{
		if (QuestAction->GetStructType() == FQuestAction_KilledTarget::StaticStruct())
		{
			FQuestAction_KilledTarget* Action_KilledTarget = static_cast<FQuestAction_KilledTarget*>(QuestAction);
			if (Action_KilledTarget->GetTargetClass() == *TargetType)
			{
				FPlatformAtomics::InterlockedIncrement(&CurrentKilled);
			}
		}
	}
}

float UDQuestCondition_KillTarget::GetQuestProgressPercent() const
{
	return CurrentKilled / static_cast<float>(KillNum);
}

bool UDQuestCondition_KillTarget::IsCompleted() const
{
	return FMath::Min(CurrentKilled, KillNum) == KillNum;
}
