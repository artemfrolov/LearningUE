// Copyright Epic Games, Inc. All Rights Reserved.

#include "LearningUECharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "LearningUE.h"
#include "StatsComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ALearningUECharacter::ALearningUECharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create the stats component. No SetupAttachment call: a UActorComponent has no
	// transform, so there is nothing to attach it to - it just belongs to this actor.
	Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ALearningUECharacter::BeginPlay()
{
	// Super first: a component's BeginPlay runs inside this call, so the Stats component
	// is fully initialised by the time the next line subscribes to it.
	Super::BeginPlay();

	// UE idiom: AddDynamic takes the listener and the function to call on it. The
	// component never learns who subscribed - it only broadcasts.
	Stats->OnDied.AddDynamic(this, &ALearningUECharacter::HandleDeath);
	Stats->OnDamaged.AddDynamic(this, &ALearningUECharacter::HandleDamaged);

	// The AnimInstance is the running instance of the Anim Blueprint on our mesh.
	// It announces when any montage finishes; that is how an attack learns it is over.
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ALearningUECharacter::HandleMontageEnded);
	}

	// Only the local player gets a HUD. An AI-possessed copy of this character must not
	// draw one, and in multiplayer neither must the other players' pawns.
	if (IsLocallyControlled() && PlayerHUDClass)
	{
		// created with the PlayerController as owner, which is how the widget later
		// answers GetOwningPlayerPawn()
		PlayerHUD = CreateWidget<UUserWidget>(GetController<APlayerController>(), PlayerHUDClass);

		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
		}
		else
		{
			UE_LOG(LogLearningUE, Error, TEXT("Could not create the player HUD widget."));
		}
	}
}

void ALearningUECharacter::HandleDeath(AActor* Killer)
{
	UE_LOG(LogLearningUE, Warning, TEXT("%s died"), *GetName());

	// route through SprintEnd rather than clearing the timer here: it is still the one
	// place that knows how to stop sprinting, and a corpse must not keep draining stamina
	SprintEnd();

	// stop the character where it stands. A real death gets a montage and a ragdoll in Phase 3.
	GetCharacterMovement()->DisableMovement();
}

void ALearningUECharacter::HandleDamaged(float Amount, AActor* Causer)
{
	if (!Stats->IsAlive())
	{
		return;
	}
	// poise: an attack in progress dies here. No need to clear bIsAttacking - stopping a
	// montage ends it as INTERRUPTED, which fires HandleMontageEnded, which clears it.
	// That branch was written in 3.3 for a case that did not exist yet.
	if (bIsAttacking)
	{
		StopAnimMontage(CurrentAttackMontage);
	}

	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage);
		return;
	}
	if (Causer)
	{
		const FVector AwayFromAttacker = (GetActorLocation() - Causer->GetActorLocation()).GetSafeNormal2D();

		LaunchCharacter(AwayFromAttacker * HitKnockbackImpulse, true, false);
	}
}

void ALearningUECharacter::DamageMe(float Amount)
{
	Stats->ApplyDamage(Amount);
}

void ALearningUECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::Look);

		// Sprinting
		// UE idiom: one action, two events - key down and key up drive separate functions
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ALearningUECharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALearningUECharacter::SprintEnd);

		// Dodging
		// only Started - a dodge is a one-shot, not something you hold
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ALearningUECharacter::Dodge);

		// Attacking
		// IA_Attack carries a Hold trigger, which splits one key into two intents:
		//   Canceled  = released BEFORE the hold threshold, i.e. a tap
		//   Triggered = the hold threshold was reached
		// This is the click-vs-long-press pattern the dream game's alternate casts need.
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Canceled, this, &ALearningUECharacter::Attack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ALearningUECharacter::HeavyAttack);
	}
	else
	{
		UE_LOG(LogLearningUE, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ALearningUECharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ALearningUECharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ALearningUECharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ALearningUECharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ALearningUECharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ALearningUECharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ALearningUECharacter::SprintStart()
{
	// no sprinting out of an attack - attacking is a commitment. Free check, so it goes
	// above the one that spends stamina.
	if (bIsAttacking)
	{
		return;
	}

	// asking must never be free, or Shift-mashing is a speed boost
	if (!Stats->TryConsumeStamina(SprintStaminaDrainRate * SprintDrainInterval))
	{
		return;
	}

	// raise the movement component's speed cap for as long as the key is held
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	GetWorld()->GetTimerManager().SetTimer(
		SprintDrainTimer,
		this,
		&ALearningUECharacter::SprintDrainTick,
		SprintDrainInterval,
		true);
}

void ALearningUECharacter::SprintEnd()
{
	// restore the normal cap
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetWorld()->GetTimerManager().ClearTimer(SprintDrainTimer);
}

void ALearningUECharacter::DoAttackTrace(FName BoneName)
{
	// start at the fist, reach forward. Note the direction comes from the CHARACTER, not
	// from the fist's motion - the same simplification Epic made. Predictable and cheap;
	// the cost is that a target directly beside you during a wide swing can be missed.
	const FVector TraceStart = GetMesh()->GetSocketLocation(BoneName);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * AttackTraceDistance);

	// which KINDS of thing can be punched. Pawn covers characters; WorldDynamic covers
	// movable props like a training dummy or a crate. Static world geometry is absent on
	// purpose - punching a wall should find nothing.
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// a sphere dragged from start to end, rather than a hairline ray - a thin ray between
	// two frames of a fast animation slips straight through people
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackTraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, Sphere, QueryParams);

	if (bShowAttackTrace)
	{
		// Draw what is actually tested: a sweep is one continuous CAPSULE, not two
		// separate spheres. Half height covers the travel plus a radius at each cap.
		const FVector Direction = (TraceEnd - TraceStart).GetSafeNormal();
		const FVector Centre = (TraceStart + TraceEnd) * 0.5f;
		const float HalfHeight = (AttackTraceDistance * 0.5f) + AttackTraceRadius;

		// MakeFromZ because a capsule's axis is its local Z
		const FQuat Orientation = FRotationMatrix::MakeFromZ(Direction).ToQuat();

		// 2 seconds so it can be studied after the swing is over
		DrawDebugCapsule(GetWorld(), Centre, HalfHeight, AttackTraceRadius, Orientation, FColor::Yellow, false, 2.0f);
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();

		if (!HitActor)
		{
			continue;
		}

		// one sweep can report the same actor once per component it touched, so without
		// this a two-collider dummy takes double damage from a single punch
		if (HitActorsThisSwing.Contains(HitActor))
		{
			continue;
		}

		// ask what it HAS, not what it IS - anything carrying stats can be hurt,
		// anything else is scenery
		UStatsComponent* HitStats = HitActor->FindComponentByClass<UStatsComponent>();

		if (!HitStats)
		{
			continue;
		}

		HitActorsThisSwing.Add(HitActor);
		// pass ourselves as the causer so the victim can work out which way it was hit
		HitStats->ApplyDamage(CurrentAttackDamage, this);

		UE_LOG(LogLearningUE, Warning, TEXT("Punch connected with %s"), *GetNameSafe(HitActor));
	}
}

void ALearningUECharacter::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// this fires for EVERY montage, so ignore any that is not the swing we started
	if (Montage != CurrentAttackMontage)
	{
		return;
	}

	CurrentAttackMontage = nullptr;

	// the attack is over whether it finished cleanly or was interrupted - either way we
	// are no longer attacking, so the flag clears in both cases
	bIsAttacking = false;
}

void ALearningUECharacter::Attack()
{
	StartAttack(LightAttackMontage, LightAttackDamage, LightAttackStaminaCost);
}

void ALearningUECharacter::HeavyAttack()
{
	// same machinery, three different numbers - that is the whole point of the refactor
	StartAttack(HeavyAttackMontage, HeavyAttackDamage, HeavyAttackStaminaCost);
}

bool ALearningUECharacter::StartAttack(UAnimMontage* Montage, float Damage, float StaminaCost)
{
	// --- free refusals first. None of these change anything. ---

	// one attack at a time: without this, every click restarts the montage from frame zero
	if (bIsAttacking)
	{
		return false;
	}

	// Validate the montage BEFORE spending stamina. PlayAnimMontage can only fail for
	// these two reasons, so ruling them out here means the committing call below cannot
	// fail after we have already been charged for it.
	if (!Montage || !GetMesh()->GetAnimInstance())
	{
		return false;
	}

	// --- the committing check: asking costs stamina ---

	if (!Stats->TryConsumeStamina(StaminaCost))
	{
		return false;
	}

	// --- nothing below this line may fail ---

	// UE idiom: ACharacter::PlayAnimMontage finds the mesh's AnimInstance for us. The
	// montage only reaches the screen because the Anim Blueprint has a Slot node.
	PlayAnimMontage(Montage);

	CurrentAttackMontage = Montage;
	CurrentAttackDamage = Damage;

	// stop sprinting, since the drain timer has no other reason to stop
	SprintEnd();
	bIsAttacking = true;

	// fresh swing, so nobody has been hit by it yet
	HitActorsThisSwing.Reset();

	// Aim the attack where the player is looking. Yaw only - copying the camera's pitch
	// would tip the character over when you look up. Done after the montage is confirmed
	// so a failed attack never turns you, and before root motion moves anything this
	// frame, so the step forward goes the new way.
	if (bFaceCameraOnAttack)
	{
		if (const AController* OwningController = GetController())
		{
			SetActorRotation(FRotator(0.0f, OwningController->GetControlRotation().Yaw, 0.0f));
		}
	}

	return true;
}

void ALearningUECharacter::Dodge()
{
	// a dodge is a grounded move: in the air there is no ground friction to decay
	// the burst, and zeroing vertical velocity turns a jump into a glide
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// refuse if the cooldown has not elapsed yet
	const float Now = GetWorld()->GetTimeSeconds();

	if (Now - LastDodgeTime < DodgeCooldown)
	{
		return;
	}

	// last of the three refusals, and the only one that costs something to ask:
	// TryConsume spends on success, so nothing below this line may fail
	if (!Stats->TryConsumeStamina(DodgeStaminaCost))
	{
		return;
	}

	// the dodge is committed now - record it and go
	LastDodgeTime = Now;

	// dodge where the player is steering; sidestep right when standing still
	FVector Direction = GetLastMovementInputVector();

	if (Direction.IsNearlyZero())
	{
		Direction = GetActorRightVector();
	}

	// UE idiom: normalise before scaling, so a half-pushed stick dodges as far as a key press
	// the two trues override existing velocity instead of adding to it, so dodges don't compound
	LaunchCharacter(Direction.GetSafeNormal() * DodgeImpulse, true, true);
}

void ALearningUECharacter::SprintDrainTick()
{
	if (!Stats->TryConsumeStamina(SprintStaminaDrainRate * SprintDrainInterval))
	{
		SprintEnd();
	}
}
