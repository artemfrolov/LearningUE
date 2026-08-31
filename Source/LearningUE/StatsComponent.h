// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "StatsComponent.generated.h"

/**
 *  Delegate signatures. These declare the SHAPE of an event, not an event itself -
 *  one signature can be reused by several events. Dynamic = bindable from Blueprint,
 *  Multicast = any number of listeners.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDied, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamaged, float, Amount, AActor*, Causer);

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

	/**
	 *  Fires when health is REMOVED, carrying how much and who did it. Distinct from
	 *  OnHealthChanged, which also fires on healing and says nothing about the source.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnDamaged OnDamaged;

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

	/**
	 *  What this actor is wearing. Data, not behaviour - which is why it is a field and
	 *  not a subclass. Two enemies of the SAME Blueprint can carry different values by
	 *  overriding this on the placed instance in the level.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Defence")
	EArmourType ArmourType = EArmourType::Unarmoured;

	/**
	 *  How much armour, independent of what kind. Type decides WHICH blows work; value
	 *  decides how much protection there is at all. A leather jerkin and full plate can
	 *  both be "Light" with very different numbers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Defence", meta = (ClampMin = "0.0"))
	float ArmourValue = 0.0f;

	/**
	 *  Tuning constant for the armour curve, K in  K / (K + ArmourValue).
	 *  It is the armour value at which damage is halved: at K = 100, 100 armour means
	 *  50% damage, 200 armour means 33%, 300 means 25%.
	 *
	 *  Lower K makes armour matter more. This is the single dial for how armoured the
	 *  whole game feels, which is why it is exposed rather than hardcoded.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Defence", meta = (ClampMin = "1.0"))
	float ArmourHalvingPoint = 100.0f;

	/**
	 *  The shared matchup table. Set to DT_ArmourMatchups in the Blueprint.
	 *
	 *  Every actor with stats points at the SAME asset - the pointer is duplicated, the
	 *  numbers are not. Leave it empty and every multiplier falls back to 1.0, so armour
	 *  type quietly stops mattering while armour value keeps working.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Defence")
	UDataTable* ArmourMatchups = nullptr;

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
	void SetHealth(float NewValue, AActor* Causer = nullptr);
	void SetStamina(float NewValue);

public:

	/**
	 *  Removes health, never below zero. Returns how much was actually removed.
	 *  Causer is passed through to OnDamaged listeners and is not used here - the
	 *  component has no opinion about who hit it, it just carries the message.
	 */
	UFUNCTION(BlueprintCallable, Category="Stats|Health")
	float ApplyDamage(float Amount, AActor* Causer = nullptr);

	/** Restores health, never above MaxHealth. Returns how much was actually restored. */
	UFUNCTION(BlueprintCallable, Category="Stats|Health")
	float Heal(float Amount);

	/** True while this actor still has health left */
	UFUNCTION(BlueprintPure, Category="Stats|Health")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category="Stats|Health")
	float GetHealth() const { return CurrentHealth; }

	/**
	 *  Runs an incoming blow through this actor's defences and returns what actually
	 *  lands. Does NOT apply it - the caller still decides whether to.
	 *
	 *      Final = Raw x TypeEffectiveness x (K / (K + ArmourValue))
	 *
	 *  Lives here rather than on the attacker because defence is the DEFENDER's business.
	 *  The attacker should not have to know that armour exists, and every future source
	 *  of damage - a spell, a trap, a falling rock - gets this for free by calling it.
	 */
	UFUNCTION(BlueprintPure, Category = "Stats|Defence")
	float CalculateMitigatedDamage(float RawDamage, EDamageType DamageType) const;

	/** What this actor is wearing. */
	UFUNCTION(BlueprintPure, Category = "Stats|Defence")
	EArmourType GetArmourType() const { return ArmourType; }

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
