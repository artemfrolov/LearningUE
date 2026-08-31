#include "WeaponData.h"

// Deliberately empty.
//
// UWeaponData is data with no behaviour - it holds numbers and answers no questions, so
// there is nothing to implement. The .cpp exists only so the build system has a
// translation unit for this class; the header carries everything real.
//
// If a weapon ever needs to COMPUTE something (say, damage after a durability penalty),
// that function's body goes here.
