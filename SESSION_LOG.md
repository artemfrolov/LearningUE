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

### Decision: keep all template variants (0.5 cancelled)

Do NOT delete `Variant_Platforming` / `Variant_SideScrolling`. My call, and the
reasoning holds: deleting working code with no forcing reason is risk without
payoff, and a broken build mid-Phase-1 costs more than the ~25% rebuild time it
would save. They also stay useful as reading material — two different solutions to
the same problems as `Variant_Combat`.

Revisit when starting the real project, where a clean foundation is the point.

### State of the code

`MaxWalkSpeed` restored to `500.f` (template default). Base values belong in the
C++ base class; the Blueprint is for tuning, and pinning a base value there is the
trap 0.4 taught.


---

## Session 3 — 2026-08-22 — Phase 1: Enhanced Input, sprint, dodge

### What exists now that I built

| Thing | Where | Mine? |
|---|---|---|
| `IA_Sprint`, `IA_Dodge` | `Content/Input/Actions/` | **yes** |
| Their key bindings in `IMC_Default` | Shift + L3, Left Alt + gamepad B | **yes** |
| `SprintStart` / `SprintEnd` / `Dodge` | `LearningUECharacter.cpp` | **yes** |
| `WalkSpeed`, `SprintSpeed`, `DodgeImpulse`, `DodgeCooldown` | `LearningUECharacter.h` | **yes** |
| Cooldown logic | written by me, reviewed not rewritten | **yes** |

### What I can explain now

- **Enhanced Input is a router.** An *Input Action* is the intent ("Sprint") and knows
  no keys. An *Input Mapping Context* holds the bindings. C++ binds to the Action, so
  rebinding needs no recompile and one action can carry many keys. Whole control
  schemes swap by adding/removing a context at runtime.
- **`ETriggerEvent`.** `Started` = key down, `Completed` = key up. Sprint uses both,
  dodge only `Started` because it is a one-shot.
- **Input Actions are assets, not code.** Created in the Content Browser. C++ holds an
  empty `UInputAction*`; the Blueprint points it at the asset. Forgetting that last
  step is why correct code does nothing.
- **`.h` vs `.cpp`.** The header says *what exists* — **every member variable, always**,
  plus function names ending in `;`. The cpp holds the function bodies. There is no
  such thing as putting a variable in the cpp.
- **`UPROPERTY` or not.** `DodgeCooldown` is a setting, so it gets one and appears in
  the Blueprint. `LastDodgeTime` is runtime bookkeeping, so it does not.
- **Steer vs shove.** `AddMovementInput` accelerates toward a speed cap (walking).
  `LaunchCharacter` sets velocity outright (dodge, knockback). Roblox: `Humanoid:Move`
  vs applying an impulse to the root part.
- **Movement modes.** `MOVE_Walking` / `MOVE_Falling` are Unreal's version of Roblox
  `HumanoidStateType`. `LaunchCharacter` forces `MOVE_Falling`, which is why the dodge
  plays the in-air animation.
- **Normalise before scaling.** `GetSafeNormal()` makes a half-pushed stick dodge
  exactly as far as a key press.
- **Live Coding, confirmed by doing.** Worked for the `IsFalling` guard (a function
  body). Would not have worked for the header changes or a constructor value.

### Bugs I found by testing (not told to me)

1. **Standing dodge plays a crouch-like pose.** Diagnosed: it is the *falling*
   animation, because `LaunchCharacter` switches the movement mode to `MOVE_Falling`.
   Left as-is — that mode switch is also what makes the dodge feel crisp, and the
   Phase 3 montage replaces the pose properly.
2. **Air dodge travelled 3-4x too far.** Fixed with an `IsFalling()` guard.
3. **Dodging off a ledge still flies.** ← **STILL OPEN.** The guard only tests the
   moment of the keypress; the burst then carries me off the edge into air that has no
   friction. A guard cannot check where I will be in 200ms. The real fix is making the
   dodge a *timed state* rather than a one-shot shove — which is what Phase 3's
   root-motion montage gives us. Deliberately not hacked around.

### Packaging: it works, and the MCP plugin broke it first

The first package failed with "Unknown Cook Failure". The cook itself completed
(`Finalisation: End`, `Done!`) but the process exited 1. The only `Error` in 1436 log
lines was:

```
LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8000
```

Cooking spawns a *second* Unreal process, which loaded the `ModelContextProtocol`
plugin, which tried to open port 8000 — already held by my running editor. **Unreal
commandlets return exit code 1 if anything logged an Error, even when the work
succeeded.**

Fix: disable the `ModelContextProtocol` plugin ("Unreal MCP" in the plugin browser)
before packaging. Two reasons it is the right call, not a workaround:

  - it owns the HTTP listener that collided
  - its modules are `Type: Runtime`, so an MCP server would ship inside the game exe

`EditorToolset` stays enabled — it declares `EditorOnly: true` and depends on
`ToolsetRegistry`, not on MCP, so it is excluded from builds automatically. Good
contrast in how to declare a dev-only plugin.

Re-enabling MCP is one tick; UE removes the entry from the .uproject rather than
setting Enabled false, because the plugin is not enabled by default.

### Config that packaging wrote

- `ProjectName=Learning UE` (was "Third Person Game Template")
- `+MapsToCook=(FilePath="/Game/ThirdPerson/Lvl_ThirdPerson")` — only cook the map I use
- `BuildConfiguration=PPBC_Development` — keeps logging and the console; Shipping strips
  them, and that is for Phase 7

### On the EULA warning

`LogModelContextProtocol` warns that data sent through it is Epic's Licensed Technology
and that I am responsible for my LLM provider not training on it. Checked: "Help
improve our AI models" is **off** in my Anthropic privacy settings. Note the warning is
narrower than the real surface — engine source read directly from disk lands in the
conversation too, by a route the plugin never sees.

### Git hazard learned the hard way

`git checkout main` was run while the editor was open, onto a commit that predates
`Content/` being tracked. Git tried to delete 849 `.uasset` files, the editor had them
locked, and the checkout half-completed: `Source/`, `Config/`, `.uproject`,
`.gitattributes` and `.mcp.json` were deleted from disk while `Content/` survived as
untracked. Nothing was lost — every commit was intact — but the working tree was broken.

**Rule: close the Unreal Editor before any branch operation that touches `Content/`.**

Recovery, for next time. Two steps so git never has to rewrite a locked file:

```
git reset --mixed <branch>   # moves the branch pointer and rebuilds the INDEX only
git checkout -- .            # restores only what actually differs on disk
```

`reset --mixed` touches no files, so it cannot fight the editor's locks. The surviving
`Content/` files then match the commit, so step two has no reason to rewrite them.

Also: the `!` prefix for running a shell command is a Claude Code prompt feature, not
PowerShell syntax.

### Tuning I chose

Dodge on **Left Alt** rather than Q (thumb for dodge, three fingers free for WASD).
`DodgeImpulse` raised to **2400** in the Blueprint; C++ default still 1200, so the
Blueprint value is pinned — intended, since it is a tuning value.

### Phase progress

- [x] **Phase 0 — Orientation** (0.5 cleanup optional, not done)
- [x] **Phase 1 — Input & movement** (sprint, dodge, packaged .exe runs standalone)
- [ ] Phase 2 — Stats as a component
- [ ] Phase 3 — Melee combat
- [ ] Phase 4 — Data-driven design
- [ ] Phase 5 — Enemy AI & perception
- [ ] Phase 6 — Where GAS fits (decision session)
- [ ] **Phase 7 — Ship it** (menu, packaging, install on son's PC)
