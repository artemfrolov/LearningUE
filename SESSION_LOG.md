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

---

## Session 4 — 2026-08-25 — Phase 2 (part): stats as a component

Branch: `phase2-stats`. Commits `ea7ce18`, `eb4033f`, `8f7f83b`, `979832b`.

### 2.0 first — reading the code I actually stand on

Before building anything, we read the files my game really uses. The decision that
made it tractable: **own the 480 lines I inherit, treat `Variant_Combat`'s 3,661 as
reference.** `ACombatCharacter` is a *sibling* sample — nothing I have inherits a line
of it. The variants are compiled into my binary but never referenced: dead weight in
the .exe, not dormant features waiting to be switched on.

### What exists now that I built

| Thing | Where | Mine? |
|---|---|---|
| `UStatsComponent` — health + stamina | `Source/LearningUE/StatsComponent.*` | **yes** |
| `TryConsumeStamina` (all-or-nothing) | written by me | **yes** |
| Stamina regen on a looping timer, with a spend delay | mentor-written, my design questions | shared |
| Dodge costs stamina | written by me | **yes** |
| Sprint drains stamina while held | written by me | **yes** |
| `Stats` attached to `ALearningUECharacter` | constructor, 1 line | **yes** |

Tunable in the Blueprint with no rebuild: `MaxHealth`, `MaxStamina`, `StaminaRegenRate`,
`StaminaRegenDelay`, `DodgeStaminaCost`, `SprintStaminaDrainRate`.

### What I can explain now

- **Why a component and not a parent class.** `ACombatCharacter` is an `ACharacter`,
  `ACombatDamageableBox` is an `AActor`. Nearest shared ancestor is `AActor`, which
  Epic cannot put game stats on — so inheritance *cannot* express "these unrelated
  things share a trait." Four copies of `CurrentHP -= Damage; if (<=0) HandleDeath();`
  is the result. Composition attaches instead of inheriting. **`UActorComponent` is
  Roblox's Humanoid** — you drop it in, you do not extend a base class.
- **Why Epic wrote `ICombatDamageable` when `AActor::TakeDamage` already exists.**
  Every actor has `TakeDamage`, so it distinguishes nothing — swing at a wall and the
  wall "takes damage." `Cast<ICombatDamageable>` is the filter: *did this thing opt
  into combat?* It also let them invent a signature carrying hit location and impulse.
- **Pawn vs Controller.** Pawn = the body (Roblox Character model). Controller = the
  driver (Roblox Player object). The controller survives death, the pawn does not —
  which is the rule for where state goes: **health on the pawn, score on the
  PlayerState.** Put health on the controller and you respawn at 3 HP.
- **Contexts vs bindings.** Mapping contexts live on the PlayerController (follow the
  human); action bindings live on the pawn (follow the body). The test: get in a car —
  same keybinds, different meaning, because `SetupPlayerInputComponent` runs on
  possession.
- **Constructor vs BeginPlay.** Constructor: no world, no other actors, no local player
  — defaults and subobjects only. BeginPlay: everything exists. `constructor()` vs
  `componentDidMount()`. This is *why* Live Coding cannot do constructor changes — the
  **CDO** is built once at startup and every spawn is cloned from it. A component's
  `BeginPlay` runs *inside* its owner's `Super::BeginPlay()`.
- **BeginPlay's boundary.** Actors and components have it. Plain `UObject`s do not —
  they do not live in the World.
- **Why GameMode's .cpp is empty.** `AGameModeBase` already implements spawning; what
  it needs from me is five class *properties*. Configuration, not logic — so it lives
  in the Blueprint. And `UCLASS(abstract)` means `GlobalDefaultGameMode` *cannot* point
  at the C++ class.
- **UBT scans `Source/`; the .sln is a stale bookmark list.** New files build fine but
  stay invisible in Solution Explorer until project files are regenerated. Ctrl+click
  still works because IntelliSense follows includes on disk. **Ritual: any file added
  outside the editor → regenerate project files.**
- **Tick vs Timer.** Tick when the value must be right *this frame* (movement, camera);
  Timer when a few times a second is plenty. A timer can be stopped; a tick runs
  forever. **DeltaTime**: per-frame changes multiply by it or they run 5x faster on a
  144fps machine than a 30fps one — which matters, because the target machine is my
  son's PC.
- **Timers start where the thing starts.** Regen → `BeginPlay` (lives as long as the
  actor). Sprint drain → `SprintStart` (exists only while the key is held). I was stuck
  for an hour looking for a permanent home for something temporary.
- **`SetTimer` needs a `void` function** — nobody is waiting for a return value. A
  `bool` operation gets a small void wrapper, and the wrapper is where the result is
  acted on.
- **`FTimerHandle` is a plain member**, not a pointer — no `*`, no `->`. `ClearTimer`
  on an unset handle is safe.
- **`#include` when you call a method on a type.** `GetTimerManager()` returns a
  reference a forward declaration covers, but `.SetTimer` needs `TimerManager.h`.

### Design rules I earned rather than was told

1. **Check-and-act must be one operation.** `TryConsumeStamina` returns `bool` and
   spends only on success, so no caller can spend without asking or ask without
   spending. TOCTOU. I chose this over `if (stamina >= cost)` myself — then reverted to
   check-then-act in `SprintStart` and had to be caught doing it.
2. **`Try` + `bool` in the name**, so a function that can decline says so.
3. **Free checks first, the committing check last.** In `Dodge`: falling → cooldown →
   stamina. Nothing below the stamina gate may fail. I got this wrong twice — first
   `LastDodgeTime` reset on a refused dodge, then stamina was spent before the cooldown
   check. The fix was moving one line down, not moving the gate up.
4. **One exit path.** `SprintEnd` is the only way sprinting stops — called by the key
   release *and* by the drain running out, so the two cannot drift apart.
5. **Asking must never be free.** `SprintStart` pays the first interval up front;
   otherwise Shift-mashing at low stamina is a 100ms speed boost per press.
6. **Prove the invariant instead of clamping.** Once affordability is checked,
   `CurrentStamina -= Amount` cannot go negative, so the `Clamp` came out. A clamp that
   can never fire hides whether the invariant is real.

### The question I asked that taught the most

*Regen is 20/sec and sprint drains 15/sec — why do I run out?* Because every successful
drain stamps `LastStaminaSpendTime`, so the 1-second regen delay never elapses while
sprinting. Regen fires 10x/sec and bails on its first line. **Net is -15, not +5.**

The delay is load-bearing: without it, any drain below the regen rate is free, and the
two numbers fight — drain would have to exceed regen, making recovery brutally slow.
With it, drain controls sprint duration and regen controls recovery, independently.

### Deliberately not done

- **The duplication.** Stamina is a near-copy of health; mana will be a third. Left
  alone on purpose — three identical resources is the shape that becomes data
  (**Phase 4**) or GAS Attributes (**Phase 6**). Writing it by hand is what will make
  that decision informed instead of cargo-culted.
- **The regen timer never stops.** It fires 10x/sec forever, even on a full bar.
  Stopping it when full and restarting on spend is what the stored `FTimerHandle` is
  really for. Deferred, not forgotten.
- **`ApplyDamage` drops health to 0 and nothing happens.** Death needs the announcement
  mechanism — that is 2.4.
- **`IsAlive()` gates stamina.** Deliberate coupling: attach this to something with
  `MaxHealth = 0` and it can never spend stamina either.

### Phase progress

- [x] **Phase 0 — Orientation**
- [x] **Phase 1 — Input & movement** (sprint, dodge, packaged .exe runs standalone)
- [ ] **Phase 2 — Stats as a component** — 2.0 / 2.2 / 2.3 done; **2.4 delegates** and
      **2.5 UMG bars** remain. Right now the resources are invisible: dodge four times
      and the fourth press silently does nothing — no bar, no sound, no reason given.
      That is the argument for 2.5, felt rather than explained.
- [ ] Phase 3 — Melee combat
- [ ] Phase 4 — Data-driven design
- [ ] Phase 5 — Enemy AI & perception
- [ ] Phase 6 — Where GAS fits (decision session)
- [ ] **Phase 7 — Ship it** (menu, packaging, install on son's PC)

---

## Session 5 — 2026-08-26 — Phase 2 complete: delegates and the HUD

Branch: `phase2-stats`. Commits `c1c4b6a`, `21d64c9`, `2de3182`.

### What exists now that I built

| Thing | Where | Mine? |
|---|---|---|
| `OnHealthChanged` / `OnStaminaChanged` / `OnDied` | `StatsComponent.h` | mentor-written, my design summary |
| `SetHealth` / `SetStamina` — the only writers | `StatsComponent.cpp` | mentor-written |
| Character subscribes to `OnDied` in `BeginPlay` | `LearningUECharacter.cpp` | mentor-written |
| `UFUNCTION(Exec) DamageMe` — console damage for testing | `LearningUECharacter.cpp` | mentor-written |
| `UPlayerHUDWidget` — subscribes, never polls | `PlayerHUDWidget.*` | mentor-written |
| `WBP_PlayerHUD` — two progress bars, layout and colours | `Content/UI/` | **yes, in the editor** |

### What I can explain now

- **Delegate = `addEventListener`.** The component broadcasts; listeners subscribe. It
  never learns who showed up, so it stays usable on a crate. The alternative —
  `Cast<ALearningUECharacter>(GetOwner())->HandleDeath()` — would rebuild the exact
  coupling the whole phase removed.
- **Multicast** = many listeners. **Dynamic** = bindable from Blueprint, slower, binds
  by *name*. UMG is Blueprint, so the HUD events must be dynamic.
- **A delegate declaration is a signature, not an event.** One
  `FOnStatChanged` type, two properties (`OnHealthChanged`, `OnStaminaChanged`).
  `BlueprintAssignable` is what makes them appear as red event nodes in Blueprint.
- **`UFUNCTION()` is mandatory on anything passed to `AddDynamic`.** Dynamic delegates
  resolve by name at runtime, and only `UFUNCTION` puts a function in the name table.
  Forgetting it gives an unhelpful `FindFunctionChecked` error.
- **One writer per field.** `SetHealth`/`SetStamina` are private and are the only code
  that assigns. Clamping and broadcasting live there, so no call site can forget either.
  Same family as "one exit path" from Session 4.
- **Death is a crossing, not a state.** `SetHealth` compares `bWasAlive` before to
  `IsAlive()` after, so hitting a corpse does not re-fire `OnDied`. Same bug family as
  the dodge cooldown resetting on a refused dodge.
- **Never `==` on floats.** `FMath::IsNearlyEqual`. Rounding makes exact equality a lie.
- **Subscribers miss the opening value.** A component's `BeginPlay` runs inside its
  owner's `Super::BeginPlay()`, so anything that binds afterwards was not listening yet.
  The HUD reads current values once in `NativeConstruct`, then reacts to changes. Skip
  that and the bars sit at zero until the first dodge.
- **Push vs subscribe, read against Epic.** `CombatCharacter` calls
  `LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP)` by hand inside `TakeDamage`.
  Works, but every future thing that changes health is a new call site to remember.
  Mine cannot fall out of sync because nothing pushes.
- **`BlueprintImplementableEvent`** — declared in C++, implemented in Blueprint, no
  `.cpp` body. The mirror of "C++ says *that*, Blueprint says *how it looks*".
- **`meta=(BindWidget)`** — a compile-time contract: the widget Blueprint must contain a
  Progress Bar named exactly `HealthBar`, or it refuses to compile. Verified by
  deliberately misspelling it. That is why the handler needs no null check.
- **`FindComponentByClass<UStatsComponent>()`** — ask what an actor *has*, not what it
  *is*. Same move as `Cast<ICombatDamageable>`. The HUD will work on any future pawn.
- **`TSubclassOf`** = a class, not an instance. C++ says a HUD is needed, the Blueprint
  says which one. Same shape as the `UInputAction*` pointers, same failure mode: leave
  the dropdown empty and correct code does nothing.
- **`NativeConstruct` / `NativeDestruct`** = UMG's BeginPlay / EndPlay. Unsubscribe in
  the second, or a widget removed and re-added is subscribed twice.
- **`IsLocallyControlled()`** guards HUD creation, so an AI-possessed pawn in Phase 5
  cannot draw a HUD on my screen.
- **`UFUNCTION(Exec)`** exposes a function to the `~` console. `DamageMe 200` is how
  death got tested before anything could deal damage. Wrap it out of Shipping in Phase 7.
- **DisableMovement is not disabled input.** The dead character stands in idle because
  the anim blueprint still runs and sees speed 0. Phase 3's death montage needs both.

### Red squiggles in Visual Studio are not errors

**Build output is truth; IntelliSense is a guess.** Unreal breaks IntelliSense
specifically because `.generated.h` files do not exist until **UnrealHeaderTool** runs
at build time — and `GENERATED_BODY()` is a macro defined inside the file being
generated. One unresolved include poisons every type below it, which is why adding a
single line can turn a whole file red.

Fix, in order: ignore it; build first and let it re-parse; close VS and delete `.vs/`
(the regenerable 1.5 GB IntelliSense database from the Session 2 audit).

UHT is also what generates the runtime name table that `AddDynamic` looks names up in —
the macros are input to a code generator, not decoration.

### Phase 2 is complete

Health, stamina, spending, regeneration, events, and a HUD that cannot fall out of sync.

### Still deliberately deferred

- **The duplication.** Mana will be a third near-copy. Left for Phase 4 (data) / Phase 6
  (GAS) to answer.
- **The regen timer never stops** — 10x/sec forever, even on a full bar. What the stored
  `FTimerHandle` is really for.
- **`DamageMe` ships.** Strip it from Shipping builds in Phase 7.
- **The HUD lives on the pawn, not the controller.** Conceptually the HUD belongs to the
  human; it is created in the character's `BeginPlay` because that is where the stats
  component is guaranteed to exist. Revisit when respawn arrives.

### Phase progress

- [x] **Phase 0 — Orientation**
- [x] **Phase 1 — Input & movement**
- [x] **Phase 2 — Stats as a component** (component, spending, regen, delegates, HUD)
- [ ] Phase 3 — Melee combat
- [ ] Phase 4 — Data-driven design
- [ ] Phase 5 — Enemy AI & perception
- [ ] Phase 6 — Where GAS fits (decision session)
- [ ] **Phase 7 — Ship it** (menu, packaging, install on son's PC)
