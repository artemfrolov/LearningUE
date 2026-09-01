// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WeaponData.h"
#include "LearningUECharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UStatsComponent;
class UUserWidget;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ALearningUECharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Health, and later stamina and mana. Attached, not inherited. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStatsComponent* Stats;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

	/** Normal movement speed, in cm/s */
	UPROPERTY(EditAnywhere, Category="Movement")
	float WalkSpeed = 500.0f;

	/** Movement speed while sprinting, in cm/s */
	UPROPERTY(EditAnywhere, Category="Movement")
	float SprintSpeed = 900.0f;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AttackAction;

	/**
	 *  What this character fights with. Both attacks, their damage, their stamina costs
	 *  and the reach of the blow all now live in this one asset instead of in eight
	 *  fields on the character. Set to DA_Fists in the Blueprint.
	 *
	 *  A pointer to an asset, exactly like the montage pointers above - the difference is
	 *  that a montage is one animation, while this is a whole weapon's worth of facts.
	 */
	UPROPERTY(EditAnywhere, Category="Combat")
	UWeaponData* EquippedWeapon;

	/**
	 *  The swing currently in flight - a copy of the definition StartAttack accepted.
	 *
	 *  Two members used to live here, one for the montage and one for the damage. They
	 *  are one member now, because the anim notify fires later and needs to know
	 *  EVERYTHING about the attack, not two facts about it. Add a field to
	 *  FAttackDefinition and it arrives here automatically.
	 *
	 *  A copy rather than a pointer: the weapon could in principle be swapped mid-swing,
	 *  and the blow that is already travelling should still be the blow you threw.
	 *  UPROPERTY so the garbage collector sees the montage pointer inside the struct.
	 */
	UPROPERTY()
	FAttackDefinition CurrentAttack;

	/** Flinch played when a blow lands. Additive, so it layers over whatever we are doing. */
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	/** Fallback shove when no flinch montage is set, in cm/s */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float HitKnockbackImpulse = 400.0f;
	/**
	 *  Turn to face the camera when an attack starts. The character normally faces where
	 *  it is RUNNING, not where you are LOOKING, so standing still it swings wherever it
	 *  last moved. Off = the old-school behaviour, for comparison.
	 */
	UPROPERTY(EditAnywhere, Category="Combat")
	bool bFaceCameraOnAttack = true;

	/** Draw the trace shape in the world. Turn off before packaging. */
	UPROPERTY(EditAnywhere, Category="Combat|Debug")
	bool bShowAttackTrace = true;

	/** Dodge Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DodgeAction;

	/**
	 *  The dodge animation. Carries root motion, so the ANIMATION moves the character
	 *  rather than a velocity impulse - which is what stops a dodge sailing off a ledge.
	 *  Set to AM_Dodge in the Blueprint.
	 */
	UPROPERTY(EditAnywhere, Category="Movement")
	UAnimMontage* DodgeMontage;

	/**
	 *  Multiplier on how far the dodge montage's root motion carries us. MM_Dash is a
	 *  long leap; a dodge is a short hop. Scaling the motion is cheaper than finding a
	 *  new animation, and the distance is tunable while playing.
	 */
	UPROPERTY(EditAnywhere, Category="Movement")
	float DodgeRootMotionScale = 0.35f;

	/** True from the moment the dodge montage starts until it ends. Runtime state. */
	bool bIsDodging = false;

	/** Seconds before the character can dodge again */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeCooldown = 0.6f;

	/** When the last dodge happened. Runtime state, not a setting, so no UPROPERTY. */
	float LastDodgeTime = -1000.0f;

	/** True from the moment an attack montage starts until it ends. Runtime state. */
	bool bIsAttacking = false;

	/**
	 *  Who this swing has already hit. Cleared when an attack starts, not when a trace
	 *  runs - a heavy attack in 3.7 will have two notifies in one montage, and the second
	 *  trace must not re-hit whoever the first one caught.
	 *  UPROPERTY so the garbage collector keeps these entries honest.
	 */
	UPROPERTY()
	TSet<AActor*> HitActorsThisSwing;

	/** Stamina spent per dodge. A tuning value, so it lives in the Blueprint too. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeStaminaCost = 25.0f;

	/** Which HUD to put on screen. Set to WBP_PlayerHUD in the Blueprint. */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	/** The live HUD instance. UPROPERTY so the garbage collector does not eat it. */
	UPROPERTY()
	UUserWidget* PlayerHUD;

	/** Stamina drained per second while sprinting */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintStaminaDrainRate = 15.0f;

	/** How often the drain timer fires. Same reasoning as the component's RegenInterval. */
	float SprintDrainInterval = 0.1f;

	FTimerHandle SprintDrainTimer;

	/** Called by the sprint timer. Pays for one interval of sprinting. */
	void SprintDrainTick();

	// --- noise ---

	/**
	 *  How often movement noise is reported, in seconds. Not a stealth dial - it is the
	 *  resolution of the reporting. Too long and you can dash between two reports; too
	 *  short and every enemy re-evaluates constantly for nothing.
	 */
	UPROPERTY(EditAnywhere, Category = "Stealth", meta = (ClampMin = "0.05"))
	float NoiseInterval = 0.35f;

	/** Below this speed, in cm/s, movement makes no sound at all. */
	UPROPERTY(EditAnywhere, Category = "Stealth", meta = (ClampMin = "0.0"))
	float SilentSpeedThreshold = 20.0f;

	FTimerHandle NoiseTimer;

	/**
	 *  Tells the world how much noise we are making. Called by NoiseTimer.
	 *
	 *  The character does not know that AI exists - it announces a fact about itself and
	 *  anything with ears may or may not pick it up. Same shape as the stats component
	 *  broadcasting rather than calling the HUD.
	 */
	void ReportMovementNoise();

public:

	/** Constructor */
	ALearningUECharacter();

protected:

	/** Subscribe to the stats component's events once we are live in the world */
	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 *  Runs when the Stats component announces death. UFUNCTION is mandatory: dynamic
	 *  delegates bind by function NAME at runtime, and only UFUNCTION registers a name.
	 */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	/**
	 *  Runs whenever we take damage. Cancels an attack in progress - poise: being hit
	 *  mid-swing costs you the swing, and the stamina, which is what makes trading
	 *  blows a decision rather than a race.
	 */
	UFUNCTION()
	void HandleDamaged(float Amount, AActor* Causer);
	/**
	 *  Runs when ANY montage on this character finishes - so it must check which one.
	 *  bInterrupted is true when the montage was cut short rather than played to the end.
	 */
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** How much the debug self-damage key deals */
	UPROPERTY(EditAnywhere, Category="Combat|Debug")
	float DebugSelfDamage = 10.0f;

	/** Bound to a raw key press, not an Input Action. Debug builds only. */
	void DebugDamageSelf();

	/**
	 *  Debug only: type "DamageMe 200" in the console (~) to hurt yourself.
	 *  Exec exposes a function to the console. Nothing damages us yet, so this is
	 *  how death gets tested before Phase 3 exists.
	 */
	UFUNCTION(Exec)
	void DamageMe(float Amount);

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called when the sprint input starts */
	void SprintStart();

	/** Called when the sprint input ends */
	void SprintEnd();

	/** Called when the dodge input fires */
	void Dodge();

	/** Called when the attack input is tapped */
	void Attack();

	/** Called when the attack input has been held long enough */
	void HeavyAttack();

	/**
	 *  Shared attack machinery. Returns false and changes nothing if the attack was
	 *  refused. Light and heavy differ only in the definition handed in - one argument
	 *  now instead of three, and adding a fourth value to an attack changes no
	 *  signature here at all.
	 *
	 *  const& because FAttackDefinition is a struct: passing it plainly would COPY all
	 *  its fields, and we only need to read them.
	 */
	bool StartAttack(const FAttackDefinition& Attack);

public:

	/**
	 *  Sweeps for targets in front of the given bone. Called by the Attack Hit anim
	 *  notify at the frame the blow lands - never on a schedule, and never by the
	 *  input code, because only the animation knows when the fist is actually out there.
	 */
	void DoAttackTrace(FName BoneName);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns the Stats component **/
	FORCEINLINE class UStatsComponent* GetStats() const { return Stats; }
};

