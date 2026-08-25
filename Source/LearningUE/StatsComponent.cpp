// Learning project - written by hand, not from the template.

#include "StatsComponent.h"

UStatsComponent::UStatsComponent()
{
	// UE idiom: components tick every frame by default. This one has nothing to do
	// per frame yet, and a tick that does nothing still costs a call per frame per actor.
	// Phase 2.3 will use a timer for regen rather than turning this back on.
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start full. This belongs here and not in the constructor: at construction time
	// MaxHealth still holds the C++ default, before any Blueprint or per-actor override
	// has been applied. By BeginPlay the real value is in place.
	CurrentHealth = MaxHealth;
}

float UStatsComponent::ApplyDamage(float Amount)
{
	// ignore healing-by-negative-damage and hits on something already dead
	if (Amount <= 0.0f || !IsAlive())
	{
		return 0.0f;
	}

	const float Before = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);

	// report what actually landed - a hit for 50 on a target with 20 left removed 20
	return Before - CurrentHealth;
}

float UStatsComponent::Heal(float Amount)
{
	// the dead do not heal; resurrection is a separate decision
	if (Amount <= 0.0f || !IsAlive())
	{
		return 0.0f;
	}

	const float Before = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);

	return CurrentHealth - Before;
}

float UStatsComponent::GetHealthPercent() const
{
	// guard the divide: a designer can and eventually will type 0 into MaxHealth
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}
