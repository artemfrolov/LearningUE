// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponData.h"
#include "MeleeAttackComponent.generated.h"

class UAnimMontage;
class ACharacter;

/**
 *  Everything needed to throw a melee blow, attached rather than inherited.
 *
 *  This is the third time the project has faced the same choice. Health could have been
 *  a field on the character; it became UStatsComponent because a dummy and an enemy need
 *  it too. Attacking is now in the same position: the player has it, the enemy needs it,
 *  and they share no game class - AEnemyCharacter deliberately descends from ACharacter,
 *  not from the player.
 *
 *  The alternative was an interface. An interface says "I can attack" and leaves each
 *  class to write its own attack; a component says "I HAVE attacking" and there is only
 *  one implementation. Two actors needing identical machinery is the component case.
 *
 *  What deliberately stayed on the character: turning to face the camera, cancelling a
 *  sprint, refusing to swing mid-dodge. None of those are facts about attacking - they
 *  are facts about being that particular character.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMeleeAttackComponent();

	/**
	 *  Throw the weapon's light attack. Returns false and changes nothing if refused -
	 *  no montage, no weapon, already swinging, or not enough stamina.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool StartLightAttack();

	/** Throw the weapon's heavy attack. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool StartHeavyAttack();

	/**
	 *  Stop the swing in progress. Poise: being hit mid-attack costs you the swing and
	 *  the stamina, which is what makes trading blows a decision rather than a race.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CancelAttack();

	/** True from the moment an attack montage starts until it ends. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	/** What we are fighting with. Null is legal and simply means we cannot attack. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	UWeaponData* GetEquippedWeapon() const { return EquippedWeapon; }

	/**
	 *  Sweeps for targets in front of the given bone. Called by the Attack Hit anim
	 *  notify at the frame the blow lands - never on a schedule, and never by the input
	 *  code, because only the animation knows when the fist is actually out there.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DoAttackTrace(FName BoneName);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 *  Which weapon this actor fights with. Set to a DA_ asset in the Blueprint.
	 *
	 *  It lives on the component and not the character because a weapon is a fact about
	 *  the thing that swings, and the component is now that thing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UWeaponData* EquippedWeapon;

	/** How far in front of the bone the blow reaches comes from the weapon; this only
	 *  says whether to draw the shape. Turn off before packaging. */
	UPROPERTY(EditAnywhere, Category = "Combat|Debug")
	bool bShowAttackTrace = true;

	/** Shared machinery. Light and heavy differ only in the definition handed in. */
	bool StartAttack(const FAttackDefinition& Attack);

	/**
	 *  The swing currently in flight - a copy of the definition StartAttack accepted.
	 *  A copy rather than a pointer, so swapping weapons mid-swing cannot change the blow
	 *  already travelling. UPROPERTY so the collector sees the montage pointer inside.
	 */
	UPROPERTY()
	FAttackDefinition CurrentAttack;

	/** True from the moment an attack montage starts until it ends. */
	bool bIsAttacking = false;

	/**
	 *  Who this swing has already hit. Cleared when an attack starts, not when a trace
	 *  runs - a montage may carry two notifies, and the second must not re-hit whoever
	 *  the first one caught.
	 */
	UPROPERTY()
	TSet<AActor*> HitActorsThisSwing;

	/** Runs when ANY montage on the owner finishes, so it must check which one. */
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** The owner as a character, or null. Cached because everything here needs the mesh. */
	ACharacter* GetOwningCharacter() const;
};
