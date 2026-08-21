# SESSION_LOG.md — What exists, and why

> The ownership rule: if I can't answer "what is in this project and why,"
> we pause and review before adding anything new.

---

## Session 1 — 2026-08-20 — Orientation & project intent

### What exists now

Nothing built by me yet. The project is the **UE 5.8 Third Person template with all
variants included**, plus mentor scaffolding:

| Thing | What it is | Mine? |
|---|---|---|
| `Source/LearningUE/` (9 files) | Plain third-person base: `LearningUECharacter`, `LearningUEGameMode`, `LearningUEPlayerController` | No — template |
| `Source/LearningUE/Variant_Combat/` (42 files) | Epic's melee combat sample: combos, charged attack, StateTree AI | No — **reference only** |
| `Source/LearningUE/Variant_Platforming/` (8), `Variant_SideScrolling/` (26) | Unused template variants | No — noise |
| `Content/ThirdPerson/Lvl_ThirdPerson.umap` | Our working level (72 actors, stock) | No — template |
| `CLAUDE.md` | Mentor instructions, destination, roadmap | Mine |
| `.mcp.json` | Unreal MCP server config (http://127.0.0.1:8000/mcp) | Mine |

### Decisions made

**1. `Variant_Combat` is a reference, not a foundation.**
We build the vertical slice ourselves on the plain `LearningUECharacter` base. When I
get stuck, we open Epic's version and compare. Rejected: building on top of it (fast,
but I'd own ~30% of the code) and deleting it (clean, but throws away a good reference).
*Why:* the point of this project is what I understand, not how fast a slice appears.

**2. Working level is `Lvl_ThirdPerson`,** not `Lvl_Combat`.

**3. Method is read → strip → rebuild → compare.** Starting from a template was
correct — nobody hand-builds a character controller. Where Epic already solved
something on the roadmap, we dissect their version, then I write my own simpler one
from a blank class, then we diff. The diff is the lesson.

**4. The project ships.** It ends as a packaged Windows build installable on my son's
PC — not an editor-only demo. Throwaway package at the end of Phase 1 to de-risk the
pipeline; the real one in Phase 7.

### What the template gives us free (do NOT re-derive, but do NOT copy either)

`Variant_Combat` already contains working: combo attacks, press-and-hold charged
attack (the same click-vs-long-press pattern the dream game's alternate-cast spells
need), weapon traces via anim notifies, damage/death/respawn, a UMG life bar,
and StateTree enemy AI with EQS.

### What is genuinely absent (verified by grep, not assumed)

- **No Stamina, no Mana anywhere.** Only `MaxHP = 5.0f` on `ACombatCharacter` — a hit
  *count*, not a resource bar with regen.
- **HP is a field on the character, not a component.** This is the exact anti-pattern
  Phase 2 exists to fix, so Phase 2 becomes a refactor I can feel the reason for.
- No sidestep dodge.
- **No AIPerception.** The only sight cone in the code is `DangerSightConeAngle`
  (enemies avoiding *hazards*, not detecting *me*). The four detection states are unbuilt.
- No DataTable-driven weapons. No GAS.

### Open question, deferred

CLAUDE.md Phase 5 says "Behavior Trees, Blackboard," but UE 5.8 and this template use
**StateTree**, which is where Epic is steering. Revisit when we reach Phase 5.

### What I should be able to explain after this session

- What LearningUE is *for*, and why it's disposable.
- The difference between the template's plain third-person base and `Variant_Combat`.
- Why we chose to rebuild rather than extend.


---

## Session 2 — 2026-08-21 — Phase 0 complete

### What I did

- **0.1 Project anatomy.** Discovered the repo tracked only 4 files — `Source/`,
  `Content/`, `Config/`, `.uproject` were untracked. Fixed it. 4.8 GB on disk, of
  which 4.6 GB is regenerable (`Intermediate/` 2.7 GB, `.vs/` 1.5 GB).
- **0.2 Actor / Component / World.** Took the character apart live via MCP. Six
  components; four inherited from `ACharacter`, two added by our C++.
- **0.3 Blueprint vs C++.** C++ declares *that* something exists; Blueprint says
  *which* asset and *what* value.
- **0.4 The walk speed exercise.** Done, both ways, with the override clash.

### What I can explain now

- **Regenerable vs real.** `Binaries/`, `Intermediate/`, `DerivedDataCache/`,
  `Saved/` all rebuild themselves. The fix-it ritual: close editor, delete them,
  regenerate project files, rebuild.
- **Two kinds of component.** `UActorComponent` = behaviour, no position
  (`CharMoveComp`). `USceneComponent` = has a transform, can attach to others
  (`CameraBoom`). My Phase 2 Stats component will be a `UActorComponent`.
- **Sockets.** The mannequin is ONE mesh with a 161-bone skeleton, not separate
  body parts like Roblox. Attach via named sockets — `HandGrip_R` on bone `hand_r`.
  Manny and Quinn share one skeleton (`SK_Mannequin`), which is why animations swap
  freely between them.
- **Why a C++ project still has Blueprints.** "Blueprint" means two things: a saved
  pre-configured child class (unavoidable), and visual scripting (mostly optional).
  `UCLASS(abstract)` on `ALearningUECharacter` makes the Blueprint child *mandatory*.
- **The override rule.** A Blueprint value is either inherited (follows the parent)
  or pinned (ignores the parent). They look identical on screen. The grey arrow on
  the RIGHT of the row in UE5 unpins it.
- **Live Coding limits.** Hot-patches function bodies. Cannot handle header changes
  or constructor defaults — those need the editor closed and a full rebuild.
- **Build configuration.** Must be `Development Editor`, not `Development`.

### Corrections I made to the mentor

- The reset arrow is grey and on the RIGHT in UE5, not yellow on the left (UE4).

### Pace calibration (important)

Lesson 0.3 failed on first delivery — ~20 undefined terms in one message. Rewritten
smaller. CLAUDE.md rule 4 now caps new terms at ~3 per message and requires every
concept to be anchored to Roblox Studio or web development first.

### State of the code

`MaxWalkSpeed = 200.f` in C++ (was 500 in the template). The Blueprint override was
added, then reset, so it now inherits. Set it back to 500 if 200 feels sluggish.

### Still open

- **0.5 (optional):** delete `Variant_Platforming` and `Variant_SideScrolling`
  (34 source files, 2 maps). Teaches `Build.cs` include paths. Can break the build.

### Phase progress

- [x] **Phase 0 — Orientation** (0.5 cleanup optional, not done)
- [ ] Phase 1 — Input & movement (Enhanced Input) + throwaway package
- [ ] Phase 2 — Stats as a component
- [ ] Phase 3 — Melee combat
- [ ] Phase 4 — Data-driven design
- [ ] Phase 5 — Enemy AI & perception
- [ ] Phase 6 — Where GAS fits (decision session)
- [ ] **Phase 7 — Ship it** (menu, packaging, install on son's PC)
