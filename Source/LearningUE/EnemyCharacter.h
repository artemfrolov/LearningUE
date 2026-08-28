// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UAnimMontage;
class UStatsComponent;

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

	/** The same component the player and the training dummy carry */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStatsComponent* Stats;

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

	/** Subscribe to the stats events */
	virtual void BeginPlay() override;

	/** Plays the flinch. Bound to the stats component's OnDamaged. */
	UFUNCTION()
	void HandleDamaged(float Amount, AActor* Causer);

	/** Logs the new health so hits can be seen without a HUD */
	UFUNCTION()
	void HandleHealthChanged(float NewValue, float MaxValue);

	/** What happens when health reaches zero. Gets a montage in 3.6c. */
	UFUNCTION()
	void HandleDeath();

public:

	/** Returns the Stats component **/
	FORCEINLINE class UStatsComponent* GetStats() const { return Stats; }
};
