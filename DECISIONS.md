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

## 14.2 Structure and conventions (decided 2026-09-07, codename pending)

### Source control ✅ GitHub private + Git LFS from commit zero

`.gitattributes` covering all binary types (`*.uasset`, `*.umap`, textures, audio,
meshes) exists before the first asset is committed. GitHub's free LFS tier is 10 GB
storage + 10 GB/month bandwidth (verified 2026-09-07 — the old 1 GB figure in
tutorials is outdated). Safety rails: Fab/Quixel imports are trimmed to used assets
before committing, and the GitHub spending limit stays at $0 so overage fails instead
of charging. **Fallback if 10 GB nears:** pay, or migrate LFS to Azure DevOps (free,
effectively unlimited). Rejected for now: Azure DevOps as primary (unfamiliar, no
current need).

### Repo layout ✅ monorepo (Artem's proposal)

Repo root = Claude Code project root (CLAUDE.md, README, SESSION_LOG.md), with
`Game/` holding the .uproject and everything Unreal generates, and `Docs/` holding
DECISIONS.md and design documentation. The two never mix: no docs inside `Game/`,
nothing UE-generated outside it. Future siblings (`Tools/` etc.) as needed.

### Naming ✅ standard Unreal prefix conventions

`BP_`, `ABP_`, `GA_`, `GE_`, `DA_`, `WBP_`, `M_`/`MI_`, `T_`, `SM_`/`SK_`, `NS_`,
etc. Full table goes in the real repo's conventions doc. Claude enforces it.

### C++ modules ✅ one runtime module

Named after the codename. Split only if a real need appears. Folders inside mirror
the content domains (by game feature, not asset type — Lyra-style).

### Project codename ✅ Arcane (decided 2026-09-07)

Repo `Arcane` (private), module `Arcane` (`ARCANE_API`), content under
`Content/Arcane/`. Chosen because the game's core concept is Arcane-in-all-things —
the crystals are one expression of it, not the theme. Explicitly a codename, NOT the
shipped title: the Riot series of the same name only matters if "Arcane" is ever
chosen as the public title, which is a separate, later decision.

## 14.3 Tooling ✅ (decided 2026-09-07)

**The rule: pain-first.** A tool is added when a concrete, recurring pain appears that
it demonstrably solves — never preemptively. **Budget: free/local only.** No paid or
token-metered services beyond the existing Claude subscription.

### Day one
- **Unreal MCP** — same constitution as the learning project: read/inspect freely,
  announce mutations, never bulk-delete. Excluded from packaged builds; it broke
  packaging once here, so it is the first suspect when builds misbehave.
- Rejected as redundant: GitHub MCP (git/gh CLI suffice), docs-mirror servers (we
  verify against the running editor).

### Earmarked — pre-approved, added when their phase arrives
- **Blender + Blender MCP** — for the custom-asset phase. Monorepo gets an `Assets/`
  root sibling for .blend sources/exports; gitignore or LFS-review if it grows heavy.
- **Local image generation + open-source image-to-3D** (Stable-Diffusion-family;
  TRELLIS / Hunyuan3D) → Blender Decimate for triangle budgets. Props, not characters.

### Art strategy (attached decision)
Free assets (Fab / Quixel / Epic) while gameplay is being proven; custom style pass
later. Visual identity comes primarily from consistent lighting/materials/post over
whatever meshes exist — Artem's concern about "looks like every free-asset game" is
addressed there first, custom meshes second.

### Animations — not an MCP problem
Epic's free **Game Animation Sample** (500+ AAA locomotion clips) + **Mixamo** (free
library) via the IK Retargeter; **Cascadeur** free indie tier for custom moves
(parries, casts, techniques). Hand-keying from scratch rejected; AI
text-to-animation rejected as immature.

### Sound — download, don't generate
Curated free libraries: Sonniss GDC packs, freesound.org, Zapsplat; trim and layer.
Generative SFX rejected — tried by Artem across several services, consistently
unsatisfying.

## 14.4 How we work ✅ (decided 2026-09-07)

**The shift:** the learning project optimized for Artem's understanding; Arcane
optimizes for a game existing, with understanding as a strong second.

### Division of labour — option (c), chosen by Artem
- **Claude does everything possible:** all code, and all editor work reachable via
  MCP. Destructive operations are always confirmed first; never bulk-delete; routine
  work is reported in briefings rather than announced per-click.
- **Artem's hands:** whatever Claude cannot do — opening/closing the editor,
  REGENERATE & BUILD, installs, logins, GPU-local pipelines — on Claude's command,
  plus review of everything via the session log.
- **Understanding is pull-based:** Artem asks when something is unclear; genuinely
  new concepts still get the mapping-first explanation before Claude builds on them.
- **New tools/MCP hookups:** flagged by Claude, who-does-what decided jointly each time.

### Working rhythm — phases, planned properly
Feature-sized increments organized into phases/subphases, as in past Claude+Artem
projects. Acknowledged: game dev has more moving pieces than web dev, so planning
gets MORE time, not less — plans live in documents (see 14.5) and are followed.

### Rules that survive unchanged
Git discipline (Claude runs it all, feature branches, always push) · answerable
questions · free-assets-first per 14.3 · honesty about experimental tools · English
everywhere · SESSION_LOG.md habit.

### Rules that die
Exercises (no exercises in production) · the strict ~3-new-terms cap · editor work
reserved for Artem · wait-for-go-ahead on >2-file changes (superseded by phase plans
approved up front).

### Ownership rule, softened
Artem should be able to explain every **system** and why it exists — not every line.

## 14.5 The development plan ✅ reshaped: pre-production first (decided 2026-09-07)

**Decision (Artem's call, and the right one):** no development planning yet. A large
amount of design lives undecided in Artem's head; it gets onto paper first. Systems
are designed in documents, then a development roadmap is written against those
documents — not the other way around. This is industry-standard pre-production:
paper is the cheapest place to be wrong.

**The sequence:**
1. **Bootstrap session** in the new Arcane repo (see ARCANE_BOOTSTRAP.md in the
   learning repo): repo + LFS from commit zero, monorepo skeleton, new CLAUDE.md
   implementing 14.4, ingest Artem's IDEAS.md brain-dump. No UE project yet — Game/
   stays empty through pre-production; the Unreal MCP hooks up when development
   starts (14.3 pain-first).
2. **Design phase:** one design area per session (attributes/skills, combat, magic,
   world, AI/simulation, economy, …), agenda proposed by Claude after reading
   IDEAS.md, approved by Artem. Output: one document per system in Docs/systems/.
3. **Only then:** ROADMAP.md with milestones, written against the finished designs.

**Docs structure adopted:** VISION.md · DECISIONS.md (this file, moves there) ·
systems/ (one doc per system, written before building it) · ROADMAP.md (later) ·
SESSION_LOG.md.

**Parked draft — revisit when the design phase ends** (Claude's milestone sketch,
explicitly NOT adopted): M0 walking skeleton (GAS plumbing, graybox island, three
attributes on screen) → M1 "the loop" (one weapon skill + one spell, skill-by-use
progression visible, one enemy, death both sides) → M2 "choice matters" (second
weapon, armor matchups, dodge family) → M3 "the island lives" (region blockout,
first NPC routine, save/load). Scope rules adopted in principle for whenever
development starts: playable at every milestone end; detailed planning one milestone
ahead only; mid-milestone ideas go to backlog, never into the milestone; breadth
(24 skills, 6 schools) built data-driven at N=2, content scaled later.

---

## Parking lot 🅿

Topics the original 14.1–14.5 plan missed; we review this list before closing Phase 14.

- (empty)
