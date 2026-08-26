// Learning project - written by hand, not from the template.

#include "PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "GameFramework/Pawn.h"
#include "StatsComponent.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();

	if (!OwningPawn)
	{
		return;
	}

	// UE idiom: ask what components an actor HAS, not what class it IS. Same reasoning as
	// Cast<ICombatDamageable> in Epic's combat sample - anything carrying stats can be shown.
	Stats = OwningPawn->FindComponentByClass<UStatsComponent>();

	if (!Stats)
	{
		return;
	}

	// The component filled its values in its own BeginPlay, which has already run by the
	// time this widget exists - so the opening broadcast is long gone. Read the current
	// state once to draw the bars correctly, then react to changes from here on.
	HandleHealthChanged(Stats->GetHealth(), Stats->GetMaxHealth());
	HandleStaminaChanged(Stats->GetStamina(), Stats->GetMaxStamina());

	Stats->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::HandleHealthChanged);
	Stats->OnStaminaChanged.AddDynamic(this, &UPlayerHUDWidget::HandleStaminaChanged);
}

void UPlayerHUDWidget::NativeDestruct()
{
	// Unsubscribe on the way out. Not strictly required - dynamic delegates drop dead
	// listeners on their own - but a widget that is removed and re-added would otherwise
	// end up subscribed twice, and then every change would redraw twice.
	if (Stats)
	{
		Stats->OnHealthChanged.RemoveDynamic(this, &UPlayerHUDWidget::HandleHealthChanged);
		Stats->OnStaminaChanged.RemoveDynamic(this, &UPlayerHUDWidget::HandleStaminaChanged);
	}

	Super::NativeDestruct();
}

void UPlayerHUDWidget::HandleHealthChanged(float NewValue, float MaxValue)
{
	// SetPercent takes 0..1. No null check on HealthBar: BindWidget already guaranteed
	// it exists, or the Blueprint would not have compiled.
	HealthBar->SetPercent(MaxValue > 0.0f ? NewValue / MaxValue : 0.0f);
}

void UPlayerHUDWidget::HandleStaminaChanged(float NewValue, float MaxValue)
{
	StaminaBar->SetPercent(MaxValue > 0.0f ? NewValue / MaxValue : 0.0f);
}
