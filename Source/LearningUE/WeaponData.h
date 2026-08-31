#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "WeaponData.generated.h"

class UAnimMontage;

/**
 *  One attack's numbers: what it plays, what it does, what it costs.
 *
 *  A light attack and a heavy attack differ only in these three values, so instead of
 *  writing the three fields twice we describe the SHAPE once and use it twice. Adding a
 *  fourth field later (windup time, stagger amount) reaches both attacks at once.
 */
USTRUCT(BlueprintType)
struct FAttackDefinition
{
	GENERATED_BODY()

	/** The animation this attack plays. The anim notify inside it is what deals the blow. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	UAnimMontage* Montage = nullptr;

	/**
	 *  What kind of blow this is. On the ATTACK, not on the weapon, so one weapon can
	 *  stab on its light and swing on its heavy - a halberd, a bayonet, a spear butt.
	 *  Costs nothing extra: the struct was already here.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	EDamageType DamageType = EDamageType::Slash;

	/** Damage on a clean hit, before any future modifiers. ClampMin stops negative damage
	 *  - a typo that would otherwise HEAL whatever you punched. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Damage = 25.0f;

	/** Stamina this attack costs to throw. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float StaminaCost = 10.0f;

	/**
	 *  How fast the montage plays. 1.0 is the animation as the animator authored it,
	 *  0.6 is 40% slower, 1.4 is faster. This is what "heavy" actually means: not just
	 *  bigger numbers, but a longer commitment before the blow lands and a longer
	 *  recovery after it.
	 *
	 *  The hit notify sits at a fixed POINT IN THE ANIMATION, not at a fixed number of
	 *  seconds, so slowing the montage delays the hit automatically. Nothing else has
	 *  to know this value exists.
	 *
	 *  ClampMin 0.1 because 0 would freeze the montage forever - the attack would never
	 *  end, and the character would be stuck attacking until the level unloads.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.1"))
	float PlayRate = 1.0f;
};

/**
 *  Everything that makes one weapon that weapon: its two attacks and its reach.
 *
 *  These are BASE values. Nothing here knows about the character holding it, its
 *  condition, or any buff - later systems multiply these numbers, they do not replace
 *  them. That separation is the whole point: the weapon states its own facts, and
 *  everything else is an opinion applied on top.
 *
 *  Lives as its own asset in the Content Browser, so a new weapon is a new file, not
 *  new C++.
 */
UCLASS(BlueprintType)
class UWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Human-readable name, for a future inventory or tooltip. FText, not FString,
	 *  because names shown to a player are the things that get translated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Tapped attack: cheap, fast, low damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacks")
	FAttackDefinition LightAttack;

	/** Held attack: slower, dearer, harder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacks")
	FAttackDefinition HeavyAttack;

	/** How far in front of the bone the blow reaches, in cm. A weapon property, not an
	 *  attack property: a spear out-reaches a dagger on BOTH its attacks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reach", meta = (ClampMin = "0.0"))
	float TraceDistance = 75.0f;

	/** How wide the blow is, in cm. Forgiveness: bigger means easier to land. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reach", meta = (ClampMin = "0.0"))
	float TraceRadius = 40.0f;
};
