// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UStatsComponent;

/**
 *  Screen HUD showing the possessing pawn's health and stamina.
 *
 *  It finds the stats component by TYPE, not by character class, so it works on
 *  anything that carries a UStatsComponent. It never pushes values on a schedule:
 *  it subscribes to the component's events and redraws only when something changed.
 */
UCLASS(abstract)
class UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	/**
	 *  meta=(BindWidget) means the widget Blueprint MUST contain a Progress Bar named
	 *  exactly "HealthBar". Unreal wires this pointer up for us, and refuses to compile
	 *  the Blueprint if it is missing - so a typo becomes an editor error you see
	 *  immediately, instead of a null pointer at runtime.
	 */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	/** The component being displayed. Found on construct, not handed to us. */
	UPROPERTY()
	UStatsComponent* Stats;

	/** UMG's BeginPlay: runs when the widget is added to the screen. */
	virtual void NativeConstruct() override;

	/** Runs when the widget leaves the screen. Where the subscriptions are dropped. */
	virtual void NativeDestruct() override;

	/** UFUNCTION is mandatory - dynamic delegates bind by name. */
	UFUNCTION()
	void HandleHealthChanged(float NewValue, float MaxValue);

	UFUNCTION()
	void HandleStaminaChanged(float NewValue, float MaxValue);
};
