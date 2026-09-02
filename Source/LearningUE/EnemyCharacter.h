// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UAnimMontage;
class UStatsComponent;
class UMeleeAttackComponent;

/**
 *  A humanoid that can be hit.
 *
 *  Note the parent: ACharacter, NOT ALearningUECharacter. The player character carries a
 *  camera boom, a follow camera, six Input Actions and a HUD class - none of which an
 *  enemy has any use for. What the two genuinely share is health and stamina, and that
 *  is not inherited from either of them. It is attached to both.
 *
 *  No AI yet. This is a body; Phase 5 adds the brain.
 */
UCLASS(abstract)
class AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	/** Constructor */
	AEnemyCharacter();

protected:

	/**
	 *  How fast this enemy moves, in cm/s. Deliberately below the player's walk speed of
	 *  500 so that escaping on foot is always possible and sneaking has a point.
	 *  Set in the constructor, so changing it here needs a rebuild.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float EnemyWalkSpeed = 400.0f;

	/** The same component the player and the training dummy carry */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStatsComponent* Stats;

	/**
	 *  The same component the player carries. Not a copy of the player's attack code -
	 *  literally the same class, holding a different weapon asset.
	 *
	 *  This is the payoff for 5.4a. Everything the enemy needs to fight - montages,
	 *  stamina cost, hit traces, damage, armour lookup, not hitting the same target
	 *  twice - arrived with this one line.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMeleeAttackComponent* MeleeAttack;

	/**
	 *  Played when a blow lands. Deliberately left empty: the template's hit reactions
	 *  are ADDITIVE animations, which need an Apply Additive node in the Anim Blueprint
	 *  to look right. Set this once that exists and the stagger below stops being needed.
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	/** How hard a hit shoves this character back, in cm/s. The stand-in for a flinch. */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float HitKnockbackImpulse = 400.0f;

public:

	/** Returns the melee attack component, so the AI controller can ask it to swing. */
	FORCEINLINE UMeleeAttackComponent* GetMeleeAttack() const { return MeleeAttack; }

protected:

	/** Death animations, one per direction the killing blow came from. */
	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	UAnimMontage* DeathMontageFront;

	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	UAnimMontage* DeathMontageBack;

	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	UAnimMontage* DeathMontageLeft;

	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	UAnimMontage* DeathMontageRight;

	/** Seconds the body stays on the ground after the death animation finishes */
	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	float CorpseLingerTime = 3.0f;

	/** Fires when the death animation is done, to hand the body over to physics */
	FTimerHandle RagdollTimer;

	/** Stops animating the skeleton and lets physics own it */
	void StartRagdoll();

	/** Picks which of the four death animations matches where the killer was standing. */
	UAnimMontage* SelectDeathMontage(AActor* Killer) const;

	/** Subscribe to the stats events */
	virtual void BeginPlay() override;

	/** Plays the flinch. Bound to the stats component's OnDamaged. */
	UFUNCTION()
	void HandleDamaged(float Amount, AActor* Causer);

	/** Logs the new health so hits can be seen without a HUD */
	UFUNCTION()
	void HandleHealthChanged(float NewValue, float MaxValue);

	/** Stops the body, plays a death animation, and schedules cleanup */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

public:

	/** Returns the Stats component **/
	FORCEINLINE class UStatsComponent* GetStats() const { return Stats; }
};
