#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CombatTypes.generated.h"

/**
 *  Shared combat vocabulary.
 *
 *  This header exists because two unrelated things need the same words: a WEAPON must
 *  be able to say what kind of blow it deals, and a DEFENDER must be able to say what
 *  it resists. Neither should have to include the other's header to find out - a stats
 *  component has no business knowing that weapons exist.
 *
 *  Header-only, no .cpp: enums and a small struct with an inline function need no
 *  translation unit of their own.
 */

/**
 *  What a blow physically does. NOT what the weapon is called - a halberd's stab and a
 *  halberd's swing are different damage types on the same weapon, which is why this
 *  lives on the ATTACK rather than on the weapon.
 *
 *  uint8 keeps it one byte. Every UENUM must fit in a byte to be Blueprint-visible.
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	/** Swords, sabres, most axe swings. Devastating on flesh, turned aside by plate. */
	Slash		UMETA(DisplayName = "Slash"),

	/** Spears, daggers, arrows. Finds the gaps armour cannot cover. */
	Pierce		UMETA(DisplayName = "Pierce"),

	/** Maces, hammers, staves, fists. Does not care what you are wearing. */
	Bludgeon	UMETA(DisplayName = "Bludgeon")
};

/**
 *  What a defender is wearing. Deliberately coarse - three kinds, not a material list.
 *  Adding "Chain" later is one line here plus one row in the matchup table.
 */
UENUM(BlueprintType)
enum class EArmourType : uint8
{
	Unarmoured	UMETA(DisplayName = "Unarmoured"),
	Light		UMETA(DisplayName = "Light Armour"),
	Heavy		UMETA(DisplayName = "Heavy Armour")
};

/**
 *  One row of the damage matchup table: how one kind of armour responds to each kind of
 *  blow. One row per armour type, so the whole table is three rows and nine numbers.
 *
 *  FTableRowBase is what makes a struct usable as a DataTable row - it is the marker
 *  that lets the editor offer this shape when you create the table, and it is what
 *  gives every row its RowName. Inheriting it is the only requirement.
 *
 *  Note where these numbers do NOT live: not on any weapon, not on any enemy. A new
 *  weapon declares its damage type and inherits every interaction here for free.
 */
USTRUCT(BlueprintType)
struct FArmourMatchupRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Multiplier applied to slashing damage. Below 1 resists, above 1 is vulnerable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchup", meta = (ClampMin = "0.0"))
	float VsSlash = 1.0f;

	/** Multiplier applied to piercing damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchup", meta = (ClampMin = "0.0"))
	float VsPierce = 1.0f;

	/** Multiplier applied to bludgeoning damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchup", meta = (ClampMin = "0.0"))
	float VsBludgeon = 1.0f;

	/**
	 *  Picks the right column for a damage type. Defined inside the struct, so it is
	 *  inline and this header still needs no .cpp.
	 *
	 *  The default case is not dead code: it is what runs if someone adds a fourth
	 *  damage type to the enum and forgets this switch. Returning 1.0 means the new
	 *  type simply has no armour interaction yet, rather than dealing zero damage.
	 */
	float GetMultiplierFor(EDamageType DamageType) const
	{
		switch (DamageType)
		{
		case EDamageType::Slash:	return VsSlash;
		case EDamageType::Pierce:	return VsPierce;
		case EDamageType::Bludgeon:	return VsBludgeon;
		default:					return 1.0f;
		}
	}
};
