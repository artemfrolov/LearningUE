// Learning project - written by hand, not from the template.

#include "MeleeAttackComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "StatsComponent.h"
#include "CombatTypes.h"
#include "LearningUE.h"
#include "DrawDebugHelpers.h"

UMeleeAttackComponent::UMeleeAttackComponent()
{
	// Nothing to do per frame. The animation drives the blow and a delegate reports the
	// end, so there is no state to poll.
	PrimaryComponentTick.bCanEverTick = false;
}

ACharacter* UMeleeAttackComponent::GetOwningCharacter() const
{
	// Cast<ACharacter> rather than storing a typed pointer: this component is legal on
	// any actor, and simply does nothing useful on one without a skeletal mesh.
	return Cast<ACharacter>(GetOwner());
}

void UMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwningCharacter = GetOwningCharacter();

	if (!OwningCharacter)
	{
		UE_LOG(LogLearningUE, Error, TEXT("%s has a MeleeAttackComponent but is not a Character."), *GetNameSafe(GetOwner()));
		return;
	}

	// The component subscribes for itself rather than being told when a montage ended.
	// That is the point of moving this out of the character: the owner does not have to
	// remember to forward anything.
	if (UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &UMeleeAttackComponent::HandleMontageEnded);
	}

	// The empty-dropdown failure, caught at spawn rather than on the first swing.
	if (!EquippedWeapon)
	{
		UE_LOG(LogLearningUE, Error, TEXT("%s has no EquippedWeapon set - it cannot attack."), *GetNameSafe(GetOwner()));
	}
}

void UMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unsubscribe on the way out. NativeDestruct's lesson from the HUD in Phase 2: a
	// listener that outlives its interest is a bug waiting for the next respawn.
	if (const ACharacter* OwningCharacter = GetOwningCharacter())
	{
		if (UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UMeleeAttackComponent::HandleMontageEnded);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UMeleeAttackComponent::StartLightAttack()
{
	return EquippedWeapon ? StartAttack(EquippedWeapon->LightAttack) : false;
}

bool UMeleeAttackComponent::StartHeavyAttack()
{
	return EquippedWeapon ? StartAttack(EquippedWeapon->HeavyAttack) : false;
}

bool UMeleeAttackComponent::StartAttack(const FAttackDefinition& Attack)
{
	ACharacter* OwningCharacter = GetOwningCharacter();

	// --- free refusals first. None of these change anything. ---

	if (!OwningCharacter || bIsAttacking)
	{
		return false;
	}

	// Validate the montage BEFORE spending stamina. PlayAnimMontage can only fail for
	// these two reasons, so ruling them out here means the committing call below cannot
	// fail after we have already been charged for it.
	if (!Attack.Montage || !OwningCharacter->GetMesh()->GetAnimInstance())
	{
		return false;
	}

	// --- the committing check: asking costs stamina ---

	// Ask what the owner HAS. An actor with an attack component but no stats can still
	// swing - it simply has no resource to spend, which is right for a trap or a turret.
	if (UStatsComponent* Stats = GetOwner()->FindComponentByClass<UStatsComponent>())
	{
		if (!Stats->TryConsumeStamina(Attack.StaminaCost))
		{
			return false;
		}
	}

	// --- nothing below this line may fail ---

	OwningCharacter->PlayAnimMontage(Attack.Montage, Attack.PlayRate);

	CurrentAttack = Attack;
	bIsAttacking = true;

	// fresh swing, so nobody has been hit by it yet
	HitActorsThisSwing.Reset();

	return true;
}

void UMeleeAttackComponent::CancelAttack()
{
	if (!bIsAttacking)
	{
		return;
	}

	if (ACharacter* OwningCharacter = GetOwningCharacter())
	{
		// Stopping a montage ends it as INTERRUPTED, which fires HandleMontageEnded,
		// which clears the flag. No need to clear it here, and clearing it here would
		// make the two paths disagree.
		OwningCharacter->StopAnimMontage(CurrentAttack.Montage);
	}
}

void UMeleeAttackComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// this fires for EVERY montage on the owner - dodges, hit reactions, deaths - so it
	// must check whether the one that ended is ours
	if (Montage != CurrentAttack.Montage)
	{
		return;
	}

	// the attack is over whether it finished cleanly or was cut short
	CurrentAttack.Montage = nullptr;
	bIsAttacking = false;
}

void UMeleeAttackComponent::DoAttackTrace(FName BoneName)
{
	const ACharacter* OwningCharacter = GetOwningCharacter();

	// The anim notify fires straight off the animation timeline and knows nothing about
	// weapons. With no weapon there is no reach and no radius, so there is no blow to
	// resolve - bail before touching the world.
	if (!OwningCharacter || !EquippedWeapon)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();

	// pulled into locals once: read four times below, and it keeps the geometry lines
	// readable rather than three-deep in arrows
	const float TraceDistance = EquippedWeapon->TraceDistance;
	const float TraceRadius = EquippedWeapon->TraceRadius;

	// start at the fist, reach forward. Note the direction comes from the ACTOR, not
	// from the fist's motion - the same simplification Epic made. Predictable and cheap;
	// the cost is that a target directly beside you during a wide swing can be missed.
	const FVector TraceStart = OwningCharacter->GetMesh()->GetSocketLocation(BoneName);
	const FVector TraceEnd = TraceStart + (OwnerActor->GetActorForwardVector() * TraceDistance);

	// which KINDS of thing can be punched. Pawn covers characters; WorldDynamic covers
	// movable props like a training dummy or a crate. Static world geometry is absent on
	// purpose - punching a wall should find nothing.
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// a sphere dragged from start to end, rather than a hairline ray - a thin ray between
	// two frames of a fast animation slips straight through people
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, Sphere, QueryParams);

	if (bShowAttackTrace)
	{
		// Draw what is actually tested: a sweep is one continuous CAPSULE, not two
		// separate spheres. Half height covers the travel plus a radius at each cap.
		const FVector Direction = (TraceEnd - TraceStart).GetSafeNormal();
		const FVector Centre = (TraceStart + TraceEnd) * 0.5f;
		const float HalfHeight = (TraceDistance * 0.5f) + TraceRadius;

		// MakeFromZ because a capsule's axis is its local Z
		const FQuat Orientation = FRotationMatrix::MakeFromZ(Direction).ToQuat();

		// 2 seconds so it can be studied after the swing is over
		DrawDebugCapsule(GetWorld(), Centre, HalfHeight, TraceRadius, Orientation, FColor::Yellow, false, 2.0f);
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

		// Ask the VICTIM what this blow is worth against it. The attacker states the raw
		// number and the kind of blow; armour is none of its business.
		const float FinalDamage = HitStats->CalculateMitigatedDamage(CurrentAttack.Damage, CurrentAttack.DamageType);

		// Logged BEFORE the damage is applied, so it reads in causal order: the blow,
		// then the health it left behind.
		UE_LOG(LogLearningUE, Warning, TEXT("%s hit %s for %.1f (raw %.1f, %s vs %s)"),
			*GetNameSafe(OwnerActor),
			*GetNameSafe(HitActor),
			FinalDamage,
			CurrentAttack.Damage,
			*StaticEnum<EDamageType>()->GetNameStringByValue(static_cast<int64>(CurrentAttack.DamageType)),
			*StaticEnum<EArmourType>()->GetNameStringByValue(static_cast<int64>(HitStats->GetArmourType())));

		// pass the OWNER as the causer, not the component - the victim works out which
		// way it was hit from the attacker's position, and a component has none
		HitStats->ApplyDamage(FinalDamage, OwnerActor);
	}
}
