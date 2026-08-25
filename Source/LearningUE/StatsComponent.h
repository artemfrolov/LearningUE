// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

/**
 *  Holds the gameplay resources of whatever actor it is attached to.
 *  It deliberately knows nothing about characters: attach it to a crate,
 *  a barrel or an enemy and it works the same. That is the point of a component.
 */
UCLASS(ClassGroup=(Stats), meta=(BlueprintSpawnableComponent))
class UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Constructor */
	UStatsComponent();

protected:

	/** Full health for this actor. Set per actor in its Blueprint - the player and a
	 *  crate share this component and simply carry different numbers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Health")
	float MaxHealth = 100.0f;

	/** Health right now. Runtime state, so designers may look but not set. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Health")
	float CurrentHealth = 0.0f;

	/** Full stamina for this actor. A setting, so it is editable per actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float MaxStamina = 100.0f;

	/** Stamina right now. Runtime state, so designers may look but not set. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float CurrentStamina = 0.0f;

	/** Fill the resources once the component is live in the world */
	virtual void BeginPlay() override;

public:

	/** Removes health, never below zero. Returns how much was actually removed. */
	UFUNCTION(BlueprintCallable, Category="Stats|Health")
	float ApplyDamage(float Amount);

	/** Restores health, never above MaxHealth. Returns how much was actually restored. */
	UFUNCTION(BlueprintCallable, Category="Stats|Health")
	float Heal(float Amount);

	/** True while this actor still has health left */
	UFUNCTION(BlueprintPure, Category="Stats|Health")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category="Stats|Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Stats|Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** 0..1, ready for a progress bar in Phase 2.5 */
	UFUNCTION(BlueprintPure, Category="Stats|Health")
	float GetHealthPercent() const;


	UFUNCTION(BlueprintPure, Category = "Stats|Stamina")
	float GetStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Stats|Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	/** 0..1, ready for a progress bar in Phase 2.5 */
	UFUNCTION(BlueprintPure, Category = "Stats|Stamina")
	float GetStaminaPercent() const;

	/**
	 *  Spends stamina all-or-nothing: if the full Amount cannot be paid, nothing is
	 *  spent and this returns false. Check and deduct happen in one call so no caller
	 *  can spend without asking, or ask without spending.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats|Stamina")
	bool TryConsumeStamina(float Amount);
};
