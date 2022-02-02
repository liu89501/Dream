#pragma once

#define NUMBER_ASSERT(T) static_assert( \
				TIsSame<T, int32>::Value || \
                TIsSame<T, uint32>::Value || \
                TIsSame<T, float>::Value, \
                "Support only (int32, uint32, float)")

namespace SlateUtils
{
	UMaterialInstanceDynamic* GetDynamicFromBrush(FSlateBrush& Brush, UObject* Outer);

	template<typename N>
	bool NumberEqual(N A, N B)
	{
		NUMBER_ASSERT(N);
		return A == B;
	}
	
	template<>
	inline bool NumberEqual(float A, float B)
	{
		return FMath::IsNearlyEqual(A, B);
	}
}

struct FNumberCounter
{

public:

	bool IsActive() const
	{
		return bActive;
	}

	void Toggle()
	{
		bActive = !bActive;
	}

	void Initialize(uint32 TargetValue)
	{
		Current = 0;
		Target = TargetValue;
	}
	
	uint32 Increment()
	{
		return ++Current;
	}
	
	uint32 Decrement()
	{
		return --Current;
	}

	uint32 GetValue() const
	{
		return Current;
	}

	bool IsCompleted() const
	{
		return Current == Target;
	}

	FNumberCounter()
		: Current(0),
    	  Target(0),
		  bActive(false)
	{
	}
	
private:

	uint32 Current;
	uint32 Target;
	bool bActive;
};

struct FLinearAnimation 
{

public:
	
	FLinearAnimation()
		: TargetValue(0),
		  CurrentValue(0),
		  bActivate(false)
	{
	}

	FLinearAnimation(float Value)
		: TargetValue(Value),
		  CurrentValue(0),
		  bActivate(false)
	{
	}
	
	FLinearAnimation(float Current, float Target)
		: TargetValue(Current),
		  CurrentValue(Target),
		  bActivate(false)
	{
	}

	FLinearAnimation& operator=(const float& InTargetValue)
	{
		TargetValue = InTargetValue;
		return *this;
	}

	void Set(float InCurrent, float InTarget)
	{
		CurrentValue = InCurrent;
		TargetValue = InTarget;
	}

	void Activate()
	{
		bActivate = true;
	}

	void Deactivate()
	{
		bActivate = false;
	}

	void Reset(float InCurrentValue = 0)
	{
		CurrentValue = InCurrentValue;
	}
	
	FORCEINLINE void InterpConstantTo(float Delta, float InterpSpeed = 1.f)
	{
		CurrentValue = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);
	}

	FORCEINLINE void ConstantToAndDeactivate(float Delta, float InterpSpeed = 1.f)
	{
		CurrentValue = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);

		if (FMath::IsNearlyEqual(TargetValue, CurrentValue))
		{
			bActivate = false;
		}
	}

	FORCEINLINE void InterpTo(float Delta, float InterpSpeed = 1.f)
	{
		CurrentValue = FMath::FInterpTo(CurrentValue, TargetValue, Delta, InterpSpeed);
	}

	FORCEINLINE bool IsCompleted() const
	{
		return FMath::IsNearlyEqual(TargetValue, CurrentValue);
	}

	bool IsActive() const
	{
		return bActivate;
	}

	FORCEINLINE float GetInProgress() const
	{
		if (TargetValue == 0)
		{
			return 0;
		}
		
		return CurrentValue / TargetValue;
	}
	
	FORCEINLINE float GetReverseProgress() const
	{
		if (TargetValue == 0)
		{
			return 0;
		}
		
		return 1 - CurrentValue / TargetValue;
	}

	float GetValue() const
	{
		return CurrentValue;
	}

private:
	
	float TargetValue;
	float CurrentValue;
	
	bool bActivate;
};
