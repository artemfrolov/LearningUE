# DECISIONS.md — Founding decisions for Project Arcane

Each entry records the decision, the reasoning, and the rejected alternatives.
Status marks: ✅ decided · ⏳ under discussion · 🅿 parked.

---

## Foundations

### Stats and abilities: Gameplay Ability System ✅ (2026-09-07)

**Decision:** attributes, skills, abilities, and effects (damage, buffs,
enchantments, costs, cooldowns) are built on GAS.

**Why:** a systems-first RPG is GAS's home genre — 6 attributes, 24 skills,
enchanting, six magic schools, and concentration are all "something modifies a stat
under conditions, for a duration, with stacking rules," which is precisely what
GameplayEffects are. GAS is the industry-standard answer, so the ecosystem
(tutorials, forums, plugins) assumes it.

**Lock-in assessment:** accepted. GAS ships inside the engine with full source;
extending it is the intended usage, not a workaround; and its blast radius is
bounded to the stats/abilities/effects domain — inventory, quests, journal, AI
detection, and economy stay plain C++/Blueprint. Residual risk: GAS is a spine, not
a swappable module; abandoning it mid-project would mean rewriting the stat/ability
layer. Judged low for a single-player systems RPG — the projects that regret GAS
are tiny games (overkill) or multiplayer games (prediction complexity).

**Rejected:** a hand-built stats/abilities layer — it would reinvent GAS piece by
piece.

### Engine: UE 5.8 ✅ (2026-09-07)

**Decision:** built on UE 5.8. No waiting for UE6, no mid-project engine upgrades
by default.

**Why:** 5.8 is the final UE5 release and gets bug fixes only — the most stable
possible target, with zero engine churn for the project's lifetime. UE6 (announced
May 2026) targets Early Access end of 2027 and full release 12–18 months later —
realistically 2029, and a fresh major version is the worst thing to build on.
Skills and code (GAS included — Fortnite runs on it) carry forward if a migration
ever becomes worth it; that call belongs to a future with something shipped.

**Rejected:** waiting for UE6; starting on 5.8 with a planned mid-project upgrade.

### Where C++ ends and Blueprint begins ✅ four-rule policy (2026-09-07)

The industry-standard split (Epic's Lyra sample follows the same shape):

1. **Systems and state live in C++.** Components, GAS attributes and damage math,
   save data, AI logic, anything with an algorithm. An `if` chain longer than three
   branches belongs in C++.
2. **Blueprint subclasses of C++ classes hold content.** Asset references, tuning
   numbers, animation/VFX/sound hookups — the game is balanced in the editor
   without compiling.
3. **Blueprint-only is allowed for** UI widgets (layout and light glue), one-off
   level scripting, and prototyping — with the standing rule that a prototype that
   survives gets promoted to C++ once its shape stabilizes.
4. **Logic exists in exactly one place.** Blueprint never reimplements what C++
   already does; if Blueprint needs it, C++ exposes it via BlueprintCallable /
   BlueprintImplementableEvent / BlueprintNativeEvent.

---

## Structure and conventions (2026-09-07)

### Source control ✅ GitHub private + Git LFS from commit zero

`.gitattributes` covering all binary types (`*.uasset`, `*.umap`, textures, audio,
meshes) exists before the first asset is committed. GitHub's free LFS tier is 10 GB
storage + 10 GB/month bandwidth (verified 2026-09-07 — the 1 GB figure in older
tutorials is outdated). Safety rails: Fab/Quixel imports are trimmed to used assets
before committing, and the GitHub spending limit stays at $0 so overage fails
instead of charging. **Fallback if 10 GB nears:** pay, or migrate LFS to Azure
DevOps (free, effectively unlimited). Rejected for now: Azure DevOps as primary
(unfamiliar, no current need).

### Repo layout ✅ monorepo

Repo root = Claude Code project root (CLAUDE.md, README, SESSION_LOG.md), with
`Game/` holding the .uproject and everything Unreal generates, and `Docs/` holding
DECISIONS.md and design documentation. The two never mix: no docs inside `Game/`,
nothing UE-generated outside it. Future siblings (`Assets/`, `Tools/`) as needed.

### Naming ✅ standard Unreal prefix conventions

`BP_`, `ABP_`, `GA_`, `GE_`, `DA_`, `WBP_`, `M_`/`MI_`, `T_`, `SM_`/`SK_`, `NS_`,
etc. Full table goes in the conventions doc. Enforced by Claude.

### C++ modules ✅ one runtime module

`Arcane` (`ARCANE_API`). Split only if a real need appears. Folders inside mirror
the content domains (by game feature, not asset type — Lyra-style).

### Project codename ✅ Arcane

Repo `Arcane` (private), module `Arcane`, content under `Content/Arcane/`. Chosen
because the game's core concept is Arcane-in-all-things — the crystals are one
expression of it, not the theme. Explicitly a codename, NOT the shipped title: the
Riot series of the same name only matters if "Arcane" is ever chosen as the public
title, which is a separate, later decision.

---

## Tooling ✅ (2026-09-07)

**The rule: pain-first.** A tool is added when a concrete, recurring pain appears
that it demonstrably solves — never preemptively. **Budget: free/local only.** No
paid or token-metered services beyond the existing Claude subscription.

### Day one
- **Unreal MCP** — read/inspect freely; destructive mutations are confirmed first;
  never bulk-delete. It is experimental: excluded from packaged builds, and the
  first suspect when builds or packaging misbehave.
- Rejected as redundant: GitHub MCP (git/gh CLI suffice), docs-mirror servers
  (engine facts are verified against the running editor).

### Earmarked — pre-approved, added when their phase arrives
- **Blender + Blender MCP** — for the custom-asset phase. `Assets/` root sibling
  holds .blend sources/exports; gitignore or LFS-review if it grows heavy.
- **Local image generation + open-source image-to-3D** (Stable-Diffusion-family;
  TRELLIS / Hunyuan3D) → Blender Decimate for triangle budgets. Props, not
  characters.

### Art strategy
Free assets (Fab / Quixel / Epic) while gameplay is being proven; custom style
pass later. Visual identity comes primarily from consistent
lighting/materials/post-processing over whatever meshes exist; custom meshes are
the second lever, not the first.

### Animations
Epic's free **Game Animation Sample** (500+ AAA locomotion clips) + **Mixamo**
(free library) via the IK Retargeter; **Cascadeur** free indie tier for custom
moves (parries, casts, techniques). Rejected: hand-keying from scratch; AI
text-to-animation (immature).

### Sound
Curated free libraries: Sonniss GDC packs, freesound.org, Zapsplat; trim and
layer. Rejected: generative SFX (current quality unsatisfying).

---

## How we work ✅ (2026-09-07)

### Division of labour
- **Claude does everything possible:** all code, and all editor work reachable via
  MCP. Destructive operations are always confirmed first; never bulk-delete;
  routine work is reported in briefings rather than announced per action.
- **Artem's hands:** whatever Claude cannot do — opening/closing the editor,
  regenerating project files and building, installs, logins, GPU-local pipelines —
  on Claude's request, plus review of everything via the session log.
- **Understanding is pull-based:** Artem asks when something is unclear; genuinely
  new concepts get a mapping-first explanation (anchored to web development or
  Roblox Studio) before Claude builds on them.
- **New tools/MCP hookups:** flagged by Claude; who-does-what decided jointly each
  time.

### Working rhythm
Feature-sized increments organized into phases and subphases. Game dev has more
moving pieces than web dev, so planning gets MORE time, not less — plans live in
documents (see The development plan) and are followed.

### Standing rules
- Claude runs all git: feature branches, small commits, push after every commit.
- Questions to Artem are answerable: one idea per question, plain wording, obvious
  answer type (a name, a number, a yes/no, a choice).
- Honesty about experimental tools — say plainly when something fails or is flaky.
- English for everything in the project.
- SESSION_LOG.md updated at the end of every session.

### Understanding target
Artem can explain every **system** in the game and why it exists — not every line.

---

## The development plan ✅ pre-production first (2026-09-07)

**Decision:** no development planning until the systems are designed on paper.
Systems are designed in documents; the development roadmap is then written against
those documents — not the other way around. Paper is the cheapest place to be
wrong.

**The sequence:**
1. **Bootstrap session** (see ARCANE_BOOTSTRAP.md): repo + LFS from commit zero,
   monorepo skeleton, CLAUDE.md, ingest IDEAS.md. No UE project yet — `Game/`
   stays empty through pre-production; the Unreal MCP hooks up when development
   starts (pain-first rule).
2. **Design phase:** one design area per session (attributes/skills, combat,
   magic, world, AI/simulation, economy, …), agenda proposed by Claude after
   reading IDEAS.md, approved by Artem. Output: one document per system in
   `Docs/systems/`.
3. **Only then:** ROADMAP.md with milestones, written against the finished
   designs.

**Docs structure:** VISION.md · DECISIONS.md (this file) · systems/ (one doc per
system, written before building it) · ROADMAP.md (later) · SESSION_LOG.md.

**Parked milestone draft 🅿 — revisit when the design phase ends:** M0 walking
skeleton (GAS plumbing, graybox island, attributes on screen) → M1 "the loop" (one
weapon skill + one spell, skill-by-use progression visible, one enemy, death both
sides) → M2 "choice matters" (second weapon, armor matchups, dodge family) → M3
"the island lives" (region blockout, first NPC routine, save/load). Scope rules
adopted in principle for whenever development starts: playable at every milestone
end; detailed planning one milestone ahead only; mid-milestone ideas go to the
backlog, never into the milestone; breadth (24 skills, 6 schools) built
data-driven at N=2, content scaled later.

---

## Parking lot 🅿

Ideas and concerns parked for later; reviewed when planning resumes.

- **Playtesting:** first playtester is Artem's son, once something is playable —
  the plan for that belongs in the roadmap phase.
