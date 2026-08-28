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

	/** The light attack animation. Set to AM_LightAttack in the Blueprint. */
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* LightAttackMontage;

	/**
	 *  Turn to face the camera when an attack starts. The character normally faces where
	 *  it is RUNNING, not where you are LOOKING, so standing still it swings wherever it
	 *  last moved. Off = the old-school behaviour, for comparison.
	 */
	UPROPERTY(EditAnywhere, Category="Combat")
	bool bFaceCameraOnAttack = true;

	/** Damage a light attack deals on a clean hit */
	UPROPERTY(EditAnywhere, Category="Combat")
	float LightAttackDamage = 25.0f;

	/** How far in front of the fist the blow reaches, in cm */
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackTraceDistance = 75.0f;

	/** How wide the blow is, in cm. Forgiveness: bigger means easier to land. */
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackTraceRadius = 40.0f;

	/** Draw the trace shape in the world. Turn off before packaging. */
	UPROPERTY(EditAnywhere, Category="Combat|Debug")
	bool bShowAttackTrace = true;

	/** Dodge Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DodgeAction;

	/** Speed of the dodge burst, in cm/s. Decays through the movement component's braking. */
	UPROPERTY(EditAnywhere, Category="Movement")
	float DodgeImpulse = 1200.0f;

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
	 *  Runs when ANY montage on this character finishes - so it must check which one.
	 *  bInterrupted is true when the montage was cut short rather than played to the end.
	 */
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

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

	/** Called when the attack input fires */
	void Attack();

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

