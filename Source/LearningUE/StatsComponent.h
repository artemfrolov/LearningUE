// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

/**
 *  Delegate signatures. These declare the SHAPE of an event, not an event itself -
 *  one signature can be reused by several events. Dynamic = bindable from Blueprint,
 *  Multicast = any number of listeners.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDied);

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

	/** Fires whenever health changes, for any reason. The HUD will listen to this. */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatChanged OnHealthChanged;

	/** Fires whenever stamina changes, for any reason. */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatChanged OnStaminaChanged;

	/** Fires once, the moment health reaches zero. */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnDied OnDied;

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

	/** Stamina restored per second, once the regen delay has passed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float StaminaRegenRate = 20.0f;

	/** Seconds of not spending before stamina starts coming back. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float StaminaRegenDelay = 1.0f;

	/**
	 *  How often the regen timer fires. Not exposed: this is a resolution knob, not a
	 *  game design one. 10 times a second is already smoother than a player can see,
	 *  and the rate above stays correct whatever this is set to.
	 */
	float RegenInterval = 0.1f;

	/** When stamina was last spent. Runtime state, so no UPROPERTY. */
	float LastStaminaSpendTime = -1000.0f;

	/** The receipt for the regen timer, kept so we can stop it later. */
	FTimerHandle StaminaRegenTimer;

	/** Fill the resources once the component is live in the world */
	virtual void BeginPlay() override;

	/** Called by the timer, not by you. Adds one interval worth of stamina. */
	void RegenerateStamina();

	/**
	 *  The ONLY things that write CurrentHealth / CurrentStamina. Every other function
	 *  goes through these, so clamping and the broadcast cannot be forgotten at a call site.
	 */
	void SetHealth(float NewValue);
	void SetStamina(float NewValue);

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
