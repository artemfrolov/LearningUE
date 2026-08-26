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
	void HandleDeath();

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
};

