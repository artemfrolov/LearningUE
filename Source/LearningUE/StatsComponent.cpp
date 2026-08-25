// Learning project - written by hand, not from the template.

#include "StatsComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	CurrentStamina = MaxStamina;

	// UE idiom: a looping timer instead of Tick. The last argument is bLoop.
	// This belongs in BeginPlay and not the constructor because the TimerManager
	// lives on the World, and in the constructor there is no World yet.
	GetWorld()->GetTimerManager().SetTimer(
		StaminaRegenTimer,          // the receipt, so this timer can be stopped later
		this,                       // the object the function belongs to
		&UStatsComponent::RegenerateStamina,
		RegenInterval,
		true);                      // repeat forever
}

void UStatsComponent::RegenerateStamina()
{
	// nothing to give back to a corpse, and nothing to do on a full bar
	if (!IsAlive() || CurrentStamina >= MaxStamina)
	{
		return;
	}

	// hold off until the player has stopped spending for a moment - without this,
	// tapping sprint or dodge would refill between presses
	if (GetWorld()->GetTimeSeconds() - LastStaminaSpendTime < StaminaRegenDelay)
	{
		return;
	}

	// rate is per SECOND, so one call is worth one interval of it. Same reasoning as
	// multiplying by DeltaTime on Tick - change RegenInterval and the speed is unchanged.
	CurrentStamina = FMath::Min(CurrentStamina + StaminaRegenRate * RegenInterval, MaxStamina);
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

float UStatsComponent::GetStaminaPercent() const
{
	// same divide guard as health
	return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
}

bool UStatsComponent::TryConsumeStamina(float Amount)
{
	// a nonsense request: a negative cost, or a corpse trying to act
	if (Amount <= 0.0f || !IsAlive())
	{
		return false;
	}

	// a valid request we can afford. Kept separate from the guard above because the two
	// deserve different reactions later - this one earns a "too tired" grunt, that one
	// is a bug.
	if (Amount <= CurrentStamina)
	{
		// no Clamp needed: affordability is already proven, so this cannot go negative
		CurrentStamina -= Amount;

		// restart the regen delay: spending is what pushes regen away
		LastStaminaSpendTime = GetWorld()->GetTimeSeconds();

		return true;
	}

	// valid, but too expensive - refuse and spend nothing
	return false;
}
