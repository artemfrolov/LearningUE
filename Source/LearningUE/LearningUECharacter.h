// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LearningUECharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UStatsComponent;
class UMeleeAttackComponent;
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

	/**
	 *  Swinging, hit traces, and the weapon. Attached, not inherited - the enemy needs
	 *  exactly this and shares no game class with us.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UMeleeAttackComponent* MeleeAttack;

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

	/**
	 *  Movement speed while sneaking, in cm/s. The stealth number: loudness is speed
	 *  divided by SprintSpeed, so 200 reports about 0.22 and is heard from roughly 4.4m
	 *  instead of the 11m a normal walk carries.
	 */
	UPROPERTY(EditAnywhere, Category="Movement")
	float SneakSpeed = 200.0f;

	/** Sneak Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SneakAction;

	/** True while the sneak key is held. */
	bool bIsSneaking = false;

	/** True while sprinting. Was implicit in MaxWalkSpeed until a third speed existed. */
	bool bIsSprinting = false;

	/**
	 *  The single place that decides how fast we move.
	 *
	 *  Three states now want to set MaxWalkSpeed, and three callers each setting it
	 *  themselves is how "attacking while sneaking silently cancels the sneak" gets in.
	 *  This derives the speed from the flags instead, so no caller can disagree.
	 */
	void UpdateMaxWalkSpeed();

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AttackAction;

	/**
	 *  Played when this character is killed. Set to one of the AM_Death_* montages.
	 *
	 *  One montage, not the four directional ones the enemy picks between. Choosing by
	 *  the direction of the killing blow is real logic that already exists on
	 *  AEnemyCharacter, and copying it here would be the second copy - the point at
	 *  which it should become a shared component instead. Logged as debt rather than
	 *  duplicated.
	 */
	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	UAnimMontage* DeathMontage;

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
	 *  How often we SAMPLE our own movement, in seconds. Not how often noise is made -
	 *  this is only the clock that measures distance travelled. Shorter is more accurate
	 *  and costs almost nothing.
	 */
	UPROPERTY(EditAnywhere, Category = "Stealth", meta = (ClampMin = "0.02"))
	float NoiseInterval = 0.1f;

	/**
	 *  Centimetres of travel between noise events - a stride.
	 *
	 *  Noise is emitted per DISTANCE, not per second, because that is how footsteps
	 *  actually work. Emitting on a fixed clock meant slow movement laid down noise
	 *  events more densely than fast movement, so sneaking was noisier per metre than
	 *  walking. Distance-based, speed changes the RATE of footsteps as well as their
	 *  loudness, and both push the same way.
	 */
	UPROPERTY(EditAnywhere, Category = "Stealth", meta = (ClampMin = "10.0"))
	float NoiseStrideDistance = 200.0f;

	/** Where we were at the last sample, for measuring how far we have come. */
	FVector LastNoiseSampleLocation = FVector::ZeroVector;

	/** Travel accumulated since the last footstep. Spends down to zero on each step. */
	float DistanceSinceLastNoise = 0.0f;

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

	/** Called when the sneak input starts */
	void SneakStart();

	/** Called when the sneak input ends */
	void SneakEnd();

	/** Called when the dodge input fires */
	void Dodge();

	/** Called when the attack input is tapped */
	void Attack();

	/** Called when the attack input has been held long enough */
	void HeavyAttack();

	/**
	 *  Shared by both attack inputs. Asks the component to swing, and if it agrees,
	 *  applies the things that are OUR business rather than the component's: stopping a
	 *  sprint and turning to face the camera.
	 */
	void TryAttack(bool bHeavy);

public:

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

	/** Returns the melee attack component **/
	FORCEINLINE class UMeleeAttackComponent* GetMeleeAttack() const { return MeleeAttack; }
};

