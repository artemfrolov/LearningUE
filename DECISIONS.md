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

### Engine version and UE5→UE6 timing ⏳

### Where C++ ends and Blueprint begins (written policy) ⏳

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
