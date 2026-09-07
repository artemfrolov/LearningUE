# ARCANE_BOOTSTRAP.md — starting Project Arcane

Two parts. Part 1 is for Artem, done by hand before the first session. Part 2 is for
the Claude session that finds this file in the new folder — it executes the bootstrap.

---

## Part 1 — Artem, before the first session

1. **Write the ideas document.** A brain dump of everything in your head about the
   game: mechanics, fragments, worries, things you have NOT decided. Any structure,
   any length, messy is fine — structuring it is Claude's job, deciding is yours.
   Name it `IDEAS.md` (plain .txt is also fine).

2. **Create the GitHub repository.** On github.com: New repository → name **Arcane**
   → **Private** → do NOT add a README, .gitignore, or license (the repo must be
   completely empty).

3. **Create the local folder** `C:\Projects\Arcane`.

4. **Copy three files into it:** this file, `DECISIONS.md` (from the learning repo
   root — the founding decisions), and your `IDEAS.md`. After these are copied, the
   learning repo is no longer needed by anything; delete it whenever you like
   (ideally after the bootstrap session succeeds).

5. **Open a terminal in that folder and run `claude`.** First message:
   *"Read ARCANE_BOOTSTRAP.md and execute Part 2."*

That's all. Everything else is Claude's job.

---

## Part 2 — Claude, in the bootstrap session

Read everything before doing anything.

1. **Read the constitution.** Read `DECISIONS.md` in this folder — all decisions
   14.1–14.5 are binding. (The learning repo it came from may no longer exist;
   nothing in this bootstrap depends on it.)
   Highlights: GAS · UE 5.8 · four-rule C++/Blueprint boundary · GitHub private +
   LFS from commit zero · monorepo layout · standard naming prefixes · one C++
   module `Arcane` · pain-first tooling, free/local only · division of labour
   option (c) — Claude does everything possible, Artem handles what Claude cannot
   (editor lifecycle, builds, installs, logins) · pull-based teaching · phase-based
   work · pre-production before development.

2. **Initialize git — LFS before anything else.** `git init` (branch `main`), then
   `.gitattributes` with LFS patterns for `*.uasset`, `*.umap`, textures, audio,
   meshes, `*.blend`, archives — BEFORE any other file is committed. Add a
   UE-ready `.gitignore` (Binaries, DerivedDataCache, Intermediate, Saved, .vs,
   *.sln — dormant until Game/ gets a project). Remote:
   `https://github.com/artemfrolov/Arcane.git`. **Pin the credential account:**
   `git config credential.https://github.com.username artemfrolov` — this machine
   has two GitHub accounts in the credential manager and pushes hang on an
   invisible account-picker without it.

3. **Build the monorepo skeleton** per decision 14.2: `Docs/` (move the local
   `DECISIONS.md` there, plus `Docs/systems/`, empty), `Game/` (empty — stays empty
   until pre-production ends; no UE project yet), `Assets/` (empty — Blender
   sources later). Root: `README.md`, `SESSION_LOG.md`, `CLAUDE.md`. This
   bootstrap file itself can be deleted once executed.

4. **Write the new CLAUDE.md** implementing decision 14.4 — the working agreement,
   not the mentor-mode rules of the learning project. Port what survives (git
   discipline, answerable questions, free-assets-first, honesty about experimental
   tools, English everywhere, session log); drop what died (exercises, term caps,
   editor-work-reserved-for-Artem, per-change go-aheads).

5. **Write Docs/VISION.md** from the source material in Part 3 below, cleaned up
   as a standalone document. Where IDEAS.md contradicts Part 3, IDEAS.md wins —
   it is newer; flag the contradiction to Artem rather than silently choosing.

6. **Ingest IDEAS.md.** Read it fully. Do NOT decide anything, do NOT start
   designing in the bootstrap session. Deliverable: a proposed **design-phase
   agenda** — the list of design areas (attributes/skills, combat, magic, world,
   AI/simulation, economy, and whatever IDEAS.md surfaces), ordered, one session
   each, for Artem's approval. Design sessions then produce `Docs/systems/*.md`
   one at a time.

7. **Commit and push** (small commits per step, push after each — standing rule),
   update `SESSION_LOG.md`, and end with the agenda proposal.

Not in this session: no UE project, no MCP servers, no C++, no asset imports.
Pre-production is documents.

---

## Part 3 — VISION.md source material

Carried over from the learning project so nothing depends on that repo existing.
This is the dream-game description as of 2026-09; IDEAS.md supersedes it wherever
they disagree.

**The game:** an island action RPG, Elder Scrolls-inspired but systems-first. The
core concept is **Arcane in all things** — Mana exists in all living things; the
deep-mined giant crystals are one expression of it (and the plot's hidden
catastrophe), not the theme itself.

- **6 attributes** (Strength, Endurance, Agility, Wits, Arcane, Will), each
  governing 4 of 24 skills in three families: Warrior / Rogue / Mage (8/8/8). No
  class at start; specialization emerges mid-game from actual play.
- **Skill-by-use progression** with anti-exploit design: XP scales with meaningful
  use (enemy threat, parry vs plain block, item complexity for crafting) — never
  with mindless repetition.
- **Combat with mechanical expression:** weapon types matter vs enemy types (axes
  vs armor, swords vs agile foes), light/heavy attacks, unlockable moves, active
  techniques costing Mana (Arcane-enhanced physical feats), sidestep dodge for
  all, dodge roll via Acrobatics (blocked by Heavy Armor), blink for mages.
- **Magic:** Arcane = pool/potency, Will = control/regen/concentration. Six
  schools: Evocation, Conjuration, Alteration, Malediction, Preservation,
  Psychomancy. Spells have meaningfully different click vs long-press alternate
  casts. Concentration can be broken by damage.
- **Enchanting via crystals** embedded in gear; crystal colors map to attributes.
- **Simulation flavor:** NPC daily routines; 4-state enemy detection
  (relaxed → alerted → searching → attacking) driven by sight cones, light, and
  noise; ownership-tracked stolen items that age out of "stolen" status by
  value/uniqueness; merchants with buying preferences.
- **Navigation without quest-marker chores:** landmark-based wayfinding,
  Morrowind-style dated journal, player map notes.

**What the learning project proved (context, not constraint):** a working UE 5.8
vertical slice was built — third-person character with health/stamina, HUD, sprint,
root-motion dodge, light/heavy melee driven by anim notifies, data-driven weapons
and armor matchups (DA_Fists/DA_Axe pattern), and an enemy with sight, hearing,
four detection states and its own attack. Hand-built equivalents of GAS concepts
were written and understood (StatsComponent≈AttributeSet, TryConsumeStamina≈Cost,
DodgeCooldown≈Cooldown, bIsAttacking≈GameplayTag,
CalculateMitigatedDamage≈GameplayEffect) — that experience is why the GAS decision
was made with open eyes. Tone for all lore/writing: grounded fantasy, no purple
prose. Playtester-in-waiting: Artem's son, when something is playable.
