# DECISIONS.md — Founding decisions for the real project

Drafted during Phase 14 in this learning repo. When the real project's repository is
created (during 14.2), this file moves there and becomes a founding document.

Every decision gets: the decision itself, the alternatives we rejected, and why.
Status marks: ⏳ under discussion · ✅ decided · 🅿 parked.

---

## 14.1 Foundations

### GAS vs hand-built ✅ GAS (decided 2026-09-07)

**Decision:** The real game uses the Gameplay Ability System for attributes, skills,
abilities, and effects (damage, buffs, enchantments, costs, cooldowns).

**Why:** A systems-first RPG is GAS's home genre — 6 attributes, 24 skills, enchanting,
six magic schools, and concentration are all "something modifies a stat under conditions,
for a duration, with stacking rules," which is precisely what GameplayEffects are. The
hand-built alternative would reinvent GAS piece by piece. Claude writes the C++, which
blunts the learning curve; and GAS is the industry-standard answer, so the ecosystem
(tutorials, forums, plugins) assumes it.

**Lock-in concern (raised by Artem, from PM experience with libraries constraining
features):** assessed and accepted. Mitigations: GAS ships inside the engine with full
source; extending it is the intended usage, not a workaround; and its blast radius is
bounded to the stats/abilities/effects domain — inventory, quests, journal, AI
detection, economy, and stolen-item tracking stay plain C++/Blueprint. Residual risk:
GAS is a spine, not a swappable module; walking away mid-project would mean rewriting
the stat/ability layer. Judged low for a single-player systems RPG — the teams that
regret GAS are tiny games (overkill) or multiplayer games (prediction complexity).

**Rejected:** growing the Phase 0–5 hand-built components (StatsComponent,
TryConsumeStamina, DodgeCooldown, CalculateMitigatedDamage) into a homemade GAS.

### Engine version and UE5→UE6 timing ✅ UE 5.8 (decided 2026-09-07)

**Decision:** The real game is built on UE 5.8. No waiting for UE6, no mid-project
engine upgrades by default.

**Why:** 5.8 is the final UE5 release and gets bug fixes only — the most stable
possible target, with zero engine churn for the project's lifetime. UE6 (announced
May 2026) targets Early Access end of 2027 and full release 12–18 months later —
realistically 2029, and a fresh major version is the worst thing to build on. All
skills and code (GAS included — Fortnite runs on it) carry forward if a migration
ever becomes worth it; that call belongs to a future with something shipped.

**Rejected:** waiting for UE6; starting on 5.8 with a planned mid-project upgrade.

### Where C++ ends and Blueprint begins ✅ four-rule policy (decided 2026-09-07)

**Decision** — the written policy, adopted as the industry-standard split (Epic's Lyra
sample follows the same shape):

1. **Systems and state live in C++.** Components, GAS attributes and damage math, save
   data, AI logic, anything with an algorithm. An `if` chain longer than three
   branches belongs in C++.
2. **Blueprint subclasses of C++ classes hold content.** Asset references, tuning
   numbers, animation/VFX/sound hookups — the game is balanced in the editor without
   compiling.
3. **Blueprint-only is allowed for** UI widgets (layout and light glue), one-off level
   scripting, and prototyping — with the standing rule that a prototype that survives
   gets promoted to C++ once its shape stabilizes.
4. **Logic exists in exactly one place.** Blueprint never reimplements what C++ already
   does; if Blueprint needs it, C++ exposes it via BlueprintCallable /
   BlueprintImplementableEvent / BlueprintNativeEvent.

**Why:** it codifies what Phases 0–5 already practiced (C++ components, Blueprint
dropdowns), and it is the mainstream Unreal convention, so ecosystem examples and
future collaborators assume it. Adopted on Claude's recommendation; Artem's position:
industry standard is the default unless there is a reason to deviate.

---

## 14.2 Structure and conventions ⏳

Folders, naming, modules, source control incl. Git LFS.

## 14.3 Tooling ⏳

Which MCP servers earn their place, what each gives, known risks.

## 14.4 How we work ⏳

Which CLAUDE.md rules survive, session-log habit, Claude's work vs my review.

## 14.5 The development plan ⏳

Milestone order, first vertical slice of the real game, scope discipline.

---

## Parking lot 🅿

Topics the original 14.1–14.5 plan missed; we review this list before closing Phase 14.

- (empty)
