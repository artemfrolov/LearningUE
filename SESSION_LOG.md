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

---

## Sessions 6–8 — 2026-08-27 → 2026-08-29 — Phase 3: Melee combat

Branch: `phase3-combat`.

### What exists now that I built

| Thing | Where | Mine? |
|---|---|---|
| `AM_LightAttack`, `AM_HeavyAttack`, `AM_HitReact`, 4 × `AM_Death_*`, `AM_Dodge` | `Content/Anims/` | **yes** |
| `IA_Attack` + Hold trigger, bound to LMB | `Content/Input/` | **yes** |
| `UAnimNotify_AttackHit` | `Source/LearningUE/` | mentor-written |
| `DoAttackTrace` — swept sphere, damage loop | `LearningUECharacter.cpp` | damage loop **mine** |
| `StartAttack` shared machinery, light + heavy | `LearningUECharacter.cpp` | mentor-written |
| Poise — being hit cancels the swing | `LearningUECharacter.cpp` | **mine** |
| `ATrainingDummy` — an AActor with stats | `Source/LearningUE/` | mentor-written |
| `AEnemyCharacter` — mesh, stats, stagger, directional death, ragdoll | `Source/LearningUE/` | mentor-written |
| `BP_TrainingDummy`, `BP_Enemy` | `Content/Blueprints/` | **yes** |
| Dodge as a root-motion state | `LearningUECharacter.cpp` | mentor-written |

### What I can explain now

- **Montage, section, notify.** A montage is an animation you trigger from code, with
  named sections you can jump between and event markers along the timeline. A notify is
  Roblox's `GetMarkerReachedSignal` — the animator decides *when* the fist is dangerous,
  not the programmer.
- **Montages need a Slot node.** Without `Slot 'DefaultSlot'` in the Anim Blueprint,
  `Montage_Play` runs, returns a real length, fires its notifies, and shows nothing.
- **How Epic's combo actually works.** Clicking does not attack — it writes down the
  time you clicked. The `CheckCombo` notify later asks "was there a click recently?"
  That is **input buffering**, and it is most of why combat feels responsive. Release is
  quantised to the notify grid, which is why the loop section has to be short.
- **The hit trace is one swept sphere at one instant**, from a bone, forward along the
  character's facing. Not a simulation of the weapon's arc. Radius is forgiveness.
- **Object types are the first filter.** `ECC_Pawn` and `ECC_WorldDynamic` are queried;
  `WorldStatic` is deliberately absent so punching a wall finds nothing. Collision setup
  is the most common reason a correct trace finds nothing.
- **Ask what an actor HAS, not what it IS.** `FindComponentByClass<UStatsComponent>()`
  is the composition version of `Cast<ICombatDamageable>`.
- **Pointers.** Every `UObject` and `AActor` is handled by pointer, always — engine
  objects are owned by the world and referred to by address, never copied. That is why
  `->` and not `.`, and why `UStatsComponent` alone does not compile.
- **Additive vs normal animation.** An additive animation stores the *difference* from a
  base pose so it can layer over anything. The montage editor previews it with no base
  pose, which is why it looks broken in isolation and correct in game.
- **Dot product = how much two directions agree.** +1 same, 0 perpendicular, −1
  opposite. Cosine similarity, exactly. Two dot products (forward, right) carve the
  space around an actor into four quadrants: magnitude picks the axis, sign picks the
  side. The Phase 5 sight cone is the same operation with a threshold.
- **Root motion vs impulse.** `LaunchCharacter` sets a ballistic velocity nothing owns.
  Root motion is swept against the world by the movement component, so it collides,
  follows the ground, and ends when the animation does.
- **`SetAnimRootMotionTranslationScale`** shrinks how far an animation carries you —
  and lives on the *character*, so forgetting to reset it silently shortens every later
  root-motion animation.
- **Enhanced Input Triggers.** A Hold trigger on one action splits a key into two
  intents: `Canceled` = released before the threshold = tap; `Triggered` = threshold
  reached = hold. No timing code. This is the click vs long-press pattern the dream
  game's alternate-cast spells reuse.
- **`BindKey` bypasses Enhanced Input** — right for a debug key that must never be
  rebindable, wrong for anything a player touches. Wrapped in `#if !UE_BUILD_SHIPPING`,
  so in a Shipping build it is absent from the binary rather than merely disabled.
- **`SetLifeSpan`** destroys an actor after N seconds with no timer to own.
- **Auto Blend Out.** Off = the montage holds its final pose instead of handing the pose
  back to the state machine. That is why a corpse stopped standing back up — and it
  means `OnMontageEnded` never fires for those montages.
- **Spring arms trace on the Camera channel** every frame. A corpse you walk into blocks
  that trace and springs the camera to your shoulders, so dead bodies ignore `ECC_Camera`.

### Design rules added this phase

7. **Make the committing call unfailable.** "Commit last" is not enough when something
   after the commit can still fail. `PlayAnimMontage` sits after the stamina spend and
   returns 0 for exactly two reasons, so both are ruled out *before* stamina is charged.
8. **`||` short-circuits, so order stops being cosmetic when a side has a side effect.**
   `if (!TryConsumeStamina(...) || bIsAttacking)` spends the stamina and *then* bails.
9. **No refunds — move the commit point instead.** If a cost feels unfair to keep, it
   was charged too early. (My call; the mentor agreed it was the sharper answer.)
10. **When a side quest costs more than the main quest, note it and walk away.**

### Bugs I found by playing

1. Sprint stamina kept draining through an attack — `SprintEnd` had only two exits and
   attacking was not one of them.
2. Sprint could be *started* mid-attack.
3. The attack fired where the character faced, not where the camera looked. Fixed by
   snapping yaw to the control rotation on attack; toggleable.
4. Root motion walks me around a rounded dummy instead of into it. Known; the industry
   answer is the Motion Warping plugin. Deferred.
5. The trace capsule lands in slightly different places each swing — frame rate, since a
   point notify samples a fast-moving bone once. Radius absorbs it.
6. The camera got stuck inside a corpse.

### Where I corrected the mentor

- The template hit reactions are **not** unusable as plain montages. They are additive,
  and only the isolated *preview* looks broken.
- The attack animations are unarmed, not weapon animations, so "the blade is live" was
  wrong — though the mechanism is bone-agnostic and unchanged.
- The legs genuinely were not animating during attacks. Twice told otherwise; the cause
  was the Control Rig, found by muting one node.
- `MM_Dash` is a nine-metre leap, not a dodge.

### Known debt

- **Foot IK is off.** `ABP_Unarmed`'s Control Rig Alpha = 0, because its foot IK ran
  after the Slot node and rewrote the leg bones during montages. Free on a flat level.
  Proper fix: drive Alpha from a variable that goes to 0 while a montage plays.
- **One dodge animation**, so a dodge turns the character — a roll, not a sidestep. A
  real sidestep needs four directional animations on the Epic skeleton. When shopping:
  "Epic Skeleton" / "UE5 Mannequin" in the description means no retargeting; anything
  else needs an IK Retargeter. Engine version listed on an *animation* pack barely
  matters — animations are data, not compiled code.
- **`DamageMe` and the `K` key** are debug-only and compiled out of Shipping already.
- **Point notify, not a notify state.** One instant is sampled per swing. Epic shipped
  zero notify states across all three variants, so this is the shipping-quality answer
  for a fast punch — revisit if a slow heavy swing starts passing through people.
- **Damage reaction logic is duplicated** between the player and the enemy. Second copy.
  When it is a third, it becomes a component or an interface.

### Phase progress

- [x] **Phase 0 — Orientation**
- [x] **Phase 1 — Input & movement**
- [x] **Phase 2 — Stats as a component**
- [x] **Phase 3 — Melee combat**
- [ ] Phase 4 — Data-driven design
- [ ] Phase 5 — Enemy AI & perception
- [ ] Phase 6 — Where GAS fits (decision session)
- [ ] **Phase 7 — Ship it** (menu, packaging, install on son's PC)

---

## Session 9 — 2026-08-30 → 2026-08-31 — Phase 4: Data-driven design

Branch: `phase4-data`. First phase with **no Epic version to read** — a grep over the
whole `Source/` tree found zero uses of `DataTable`, `UDataAsset` or `FTableRowBase`.
Epic's Combat variant hardcodes everything. So the read → strip → rebuild → compare loop
did not apply; this phase was build-first.

### How the sessions changed

Mid-phase I rewrote the mentor rules in CLAUDE.md. The short version:

- **Claude writes all the C++ and explains it.** No exercises unless we have done the
  *exact* thing before. No "as you already did before" — forgetting is normal.
- **I do all the editor work**, with exhaustive step-by-step instructions. That is where
  my gap actually is; typing out C++ I already understand teaches me nothing.
- **`BUILD` / `REGENERATE & BUILD` on its own line**, never buried mid-sentence.
- **Questions must be answerable**: one idea, plain wording, obvious what kind of answer
  is wanted. English is my second language and vague framings cost me 10+ minutes.
- **End-of-phase questions may exceed three.**

Also settled a question left open since Phase 0: **the MCP plugin can create and modify
assets**, not just read. It exposes 19 toolsets including `DataAssetTools`,
`DataTableTools`, `BlueprintTools` and `SceneTools`. The division of labour is a
deliberate choice, not a limitation — Claude uses MCP read-only, to verify my work.

### What exists now

| Thing | Where | Whose |
|---|---|---|
| `FAttackDefinition` — montage, damage, stamina cost, play rate, damage type | `WeaponData.h` | mentor-written |
| `UWeaponData` — two attacks + reach, as a DataAsset | `WeaponData.h` | mentor-written |
| `EDamageType`, `EArmourType`, `FArmourMatchupRow` | `CombatTypes.h` | mentor-written |
| `CalculateMitigatedDamage` — the armour formula | `StatsComponent.cpp` | mentor-written |
| `DA_Fists`, `DA_Axe` | `Content/Data/` | **mine** |
| `DT_ArmourMatchups` — 3 rows, 9 numbers | `Content/Data/` | **mine** |
| Two enemies with different armour, one Blueprint | level, per-instance overrides | **mine** |
| The damage-type model itself (Slash/Pierce/Bludgeon) | design | **mine** |

Eight hardcoded fields left `LearningUECharacter.h`. What replaced them is one pointer.

### What I can explain now

- **DataAsset vs DataTable, and when each wins.** A **DataAsset** is one file per thing,
  holding references to other assets, and can carry fields a sibling does not. A
  **DataTable** is many rows of identical shape, keyed by name, editable like a
  spreadsheet and exportable to CSV. Weapons became assets; the 3x3 matchup grid became
  a table. Both are correct; they answer different questions.
- **Declare the shape once.** `FAttackDefinition` describes an attack, and a weapon uses
  it twice. Every field added since — `PlayRate`, `DamageType` — reached both attacks on
  every weapon for the price of one line. That decision paid for itself twice in one
  phase.
- **`USTRUCT` vs `UCLASS`.** A struct has no pointer, no lifetime, no garbage
  collection. It lives inside whatever owns it and is copied on assignment — hence `.`
  and not `->`. Always initialise its members: a property absent from a saved asset
  falls back to the C++ default, which is why adding `PlayRate = 1.0f` did not break two
  already-saved weapons.
- **`UENUM` must be `: uint8`** to be visible to Blueprints and the Details panel.
  `UMETA(DisplayName=...)` sets the label in the dropdown — which is why the enum value
  `Light` displays as "Light Armour" but the DataTable row must still be named `Light`.
- **`FTableRowBase` is the whole contract** for a DataTable row. Inheriting it is what
  makes a struct appear in the editor's row-structure picker.
- **`StaticEnum<T>()->GetNameStringByValue(...)`** turns an enum value into its name at
  runtime. That is how row names are derived from the enum, so there is no second list
  to keep in sync — add `Chain` to the enum and the row it looks for is `"Chain"`.
- **`FindRow` returns a pointer INTO the table**, not a copy. Read it; do not store it.
  Its context-string argument exists only for the error message, so make it useful.
- **Diminishing returns beat subtraction.** `Damage - Armour` zeroes out light attacks,
  barely troubles heavy ones, and goes negative without a clamp. `K / (K + Armour)`
  approaches zero without reaching it, so no immunity and no clamping. `K` is named
  `ArmourHalvingPoint` because that is literally what it is.
- **Per-instance overrides.** Two enemies of one Blueprint can carry different armour by
  overriding the property on the placed actor. A **yellow arrow** marks any property
  changed from its Blueprint default, and resets it.
- **The failure I keep hitting has a shape: an empty dropdown.** Correct code, silently
  doing nothing. Third time this project. `BeginPlay` now logs an error if
  `EquippedWeapon` is unset.

### The numbers, and what they proved

Same two weapons, three targets, light attacks only:

| Target | Axe (Slash) | Fists (Bludgeon) |
|---|---|---|
| Dummy — Unarmoured, 0 | **2 hits** | 5 hits |
| Enemy 1 — Light, 30 | **4 hits** | 6 hits |
| Enemy 2 — Heavy, 60 | 8 hits | **6 hits** |

Against plate, bare hands beat the axe. Weapon choice became a read of the enemy rather
than an upgrade path — and nothing in that table is written in code.

### Design rules added this phase

11. **Declare the shape once, use it twice.** The second copy is where the drift starts.
12. **Choose fallbacks so a mistake degrades to "no effect", never to "no damage".** A
    missing matchup table returns 1.0, not 0 — a missing asset must never make an actor
    invulnerable, because that failure is nearly invisible in play.
13. **Defence belongs to the defender.** The attacker states a raw number and a damage
    type and stops. Every future damage source — spell, trap, falling rock — inherits
    the whole armour system by calling one function.
14. **Data, not subclasses, for things that differ only by value.** A Blueprint subclass
    is for different *behaviour*. One class per armour type is class explosion.
15. **A system can be correct and still be inert.** The fists log was arithmetically
    perfect and changed nothing about the fight. Correct is not the same as felt.

### Where I corrected the mentor

- **Weapon type was the wrong axis; damage type is the right one.** A halberd interacts
  with armour like an axe, a quarterstaff like a club. Armour responds to what the blow
  *does*, not what the object is called. Weapon *category* is a separate axis, useful
  for skill XP and animation sets — not for this.
- **Damage type belongs on the attack, not the weapon**, so one weapon can stab on its
  light and swing on its heavy. The mentor called this "maybe too much"; it cost one
  field in a struct that already existed.
- **Three damage types, not four.** Slash and Chop are near-synonyms and would be
  second-guessed forever. The "axes are sharp but heavy" problem is better solved with
  armour penetration as a weapon passive, which composes with everything else instead of
  multiplying the matrix.
- **PlayRate as a per-weapon constant is mostly redundant** — a real axe has its own
  animations with their own wind-up. Its honest home is as a *runtime* multiplier:
  encumbrance, haste, fatigue, an Agility-derived attack speed.
- **Skipped the skills DataTable (4.5).** It would have taught exactly one new technique
  (CSV round-trip) wrapped in 24 invented skill names, used by no later phase and absent
  from the vertical slice. The mentor proposed it because the roadmap said so; the
  roadmap's stated purpose — making the big game feel feasible — was already met by a
  table that actually does something.

### Known debt

- **`ArmourValue` currently drowns out `ArmourType`.** At 30/60 against a halving point
  of 100, the value contributes more than the type does. Three dials fix it, all in the
  editor: widen the type table, lower the values, or raise `ArmourHalvingPoint`.
- **The `ArmourMatchups` pointer is set in three Blueprints.** The pointer duplicates;
  the numbers do not. Correct home is project settings (`UDeveloperSettings`), deferred
  as a whole extra concept.
- **`PlayRate` scales the entire montage uniformly.** No separate wind-up and recovery —
  that needs notify states and montage sections.
- **DataTable `.uasset` files are binary, so git cannot diff them.** A CSV source
  alongside would be reviewable. Deliberately deferred; will matter at hundreds of rows.
- **No armour penetration, and no weapon-category axis.** Both designed, neither built.
- **Damage reaction logic is still duplicated** between the player and the enemy. Second
  copy. Third copy makes it a component.
- **`bShowAttackTrace` still defaults to true.** Turn off before packaging in Phase 7.

---

## Sessions 10–12 — 2026-08-31 → 2026-09-03 — Phase 5: Enemy AI & perception

Branch: `phase5-ai`.

### The finding that shaped the phase

Epic shipped a complete StateTree AI in `Variant_Combat` — a controller and eight
hand-written C++ tasks. **But their enemy has no senses at all.** `FStateTreeGetPlayerInfoTask`
calls `UGameplayStatics::GetPlayerPawn()` directly: it always knows exactly where the
player is, through walls, from any distance, facing any direction. Across the whole
`Source/` tree there are **zero** uses of AIPerception, BehaviorTree or Blackboard.

So the phase split in two: **senses**, which had to be built from nothing, and
**behaviour**, which Epic had already solved and we could read. All the value was in the
first half — the four detection states are the design doc's sneaking system, and they
cannot be copied from anywhere in this template.

### Resolved: the StateTree vs Behavior Tree question left open in Session 1

**StateTree.** The plugins are already enabled and in `Build.cs`, Epic's own 5.8 combat
sample uses it, and StateTree left beta in 5.8. Behavior Trees are not deprecated and
have far more tutorials — which is the honest downside: searching "Unreal AI tutorial"
returns Behavior Trees overwhelmingly, and that mismatch is expected, not a wrong turn.

In the event the four states were built as a **plain C++ state machine**, because
perception, states and hearing are all independent of how behaviour is orchestrated.
Reading Epic's StateTree and deciding whether to port is deferred; it was never reached.

### What exists now

| Thing | Where | Whose |
|---|---|---|
| `AEnemyAIController` — perception, four states, hearing | `EnemyAIController.h/.cpp` | mentor-written |
| `EEnemyAlertState` — Relaxed / Alerted / Searching / Attacking | `EnemyAIController.h` | mentor-written |
| Sight sense: cone, hysteresis, memory, affiliation | `EnemyAIController.cpp` | mentor-written |
| Hearing sense + distance-based footstep noise | both characters | mentor-written; the model is **mine** |
| Sneak (Left Ctrl) and `UpdateMaxWalkSpeed` | `LearningUECharacter.cpp` | mentor-written |
| `UMeleeAttackComponent` — the third-copy extraction | `MeleeAttackComponent.h/.cpp` | mentor-written |
| Enemy attacks, cooldown, commitment | `EnemyAIController.cpp` | mentor-written |
| Player death: montage, input disabled | `LearningUECharacter.cpp` | mentor-written |
| NavMeshBoundsVolume, cover columns | level | **mine** |
| `IA_Sneak` + Left Ctrl binding | `Content/Input/` | **mine** |
| `DA_EnemyFists` | `Content/Data/` | **mine** |
| `Use Acceleration for Paths` on BP_Enemy | `BP_Enemy` | **mine** |

### What I can explain now

- **The brain is a separate actor from the body.** A Controller possesses a Pawn, exactly
  as a PlayerController possesses my character. Swap the controller and the same body
  behaves differently.
- **Perception is a subscription, not a poll.** Configure a sense — radius, cone, memory —
  and the engine calls back on CHANGE. Never "can I see him yet?" on a timer.
- **One component, many senses.** The perception component is machinery; each sense is a
  config object handed to it. Both report through one delegate, told apart by
  `FAIStimulus::Type`.
- **The affiliation trap.** Sight reports only ENEMIES by default, but "enemy" needs a
  team system, and without one every actor is NEUTRAL — so the stock configuration
  detects nothing, silently. The single most common reason people think AIPerception is
  broken.
- **`PeripheralVisionAngleDegrees` is HALF the cone.** 60 means a 120° field of view.
- **`AutoSuccessRangeFromLastSeenLocation` can only prevent LOSING a target, never find
  one** — the name says it. Proven by two experiments: touching an unaware enemy from
  behind stays undetected (a backstab window), but circling close around one that has
  already seen me does not break its sight.
- **Hearing has no "stopped hearing" event.** Sight reports both directions; ears only
  ever report success.
- **`LastKnownLocation`, not the target's position.** On a sight LOSS the stimulus
  location is where the target was last sensed. That one variable is the difference
  between an enemy that searches and an enemy that cheats.
- **Entry actions once, repeated work on the tick.** `SetAlertState` holds everything that
  happens on arriving; `Think` holds everything that repeats. Mixing them is how state
  machines rot — a move order re-issued five times a second never gets anywhere.
- **`MoveToActor` tracks a moving goal; `MoveToLocation` does not.** Using the wrong one
  is why chasing AI sometimes follows your ghost.
- **A NavMesh is pre-computed walkable surface.** No mesh, no path, and `MoveTo` fails
  silently. Press **P** to see it.
- **AI movement and keyboard movement come through different doors.** Input produces
  Acceleration, which produces velocity. Path following sets velocity directly and leaves
  Acceleration at zero. That is a real behavioural difference, not a detail.
- **Nothing links "moving" to "run animation".** Unlike Roblox's Humanoid, a skeletal mesh
  plays only what its Anim Blueprint decides. `ABP_Unarmed` gates it on
  `GroundSpeed > 0 AND Acceleration != 0`, so every AI in the engine slides until told
  otherwise.
- **`OnPossess`, not the constructor**, for anything belonging to the pawn — a controller
  is built before it has a body.
- **"Unresolved external symbol" always means declared-but-no-body.** The compiler was
  happy; the linker could not find the code.
- **Live Coding cannot create new classes.** New files or header changes mean closing the
  editor and a full rebuild, and the build fails with exit code 6 rather than saying so
  loudly.

### Design rules added this phase

16. **Two thresholds, not one.** `LoseSightRadius` (1800) is deliberately larger than
    `SightRadius` (1500), and `CombatApproachDistance` (110) deliberately smaller than
    `AttackRange` (150). One number for both means anything sitting on the boundary
    oscillates — seen/lost/seen, or walk/arrive/drift/walk. Hysteresis is free.
17. **Entry actions once, repeated work on the tick.** And one function owns every state
    change, so one place logs them and nothing can bypass it.
18. **Emit per the unit the player experiences.** Footsteps happen per metre, not per
    second. Getting that backwards made sneaking noisier per metre than walking — the
    exact opposite of the mechanic.
19. **A default argument can carry policy.** `MoveToActor`'s `bStopOnOverlap` defaults to
    true and silently adds both capsule radii to the acceptance radius. Asking for 150
    stopped the enemy at ~234, permanently outside its own reach.
20. **Choose fallbacks so a missing thing degrades to "no effect".** A controller whose
    pawn has no attack component chases and never swings, logged as a warning. Not an
    error, because that is a valid unarmed enemy.
21. **When it is the third copy, extract it.** Written as debt in Phase 3, executed here.

### Bugs I found by playing

1. **A column with a 5cm gap under it** let enemies see through the floor. Diagnosed by
   me from the AI's behaviour alone; fixed by sinking the geometry. AI sight is a single
   line trace, so any gap is a perfect sightline — a real production problem, and
   "sink it into the floor" is the professional answer.
2. **The enemy stopped outside its own reach** and could never attack (rule 19).
3. **A living enemy's fist collapsed the camera** when it passed near my head — the mesh
   blocked the Camera channel. Corpses were exempted in Phase 3; living bodies were not.
4. **Sneaking produced MORE noise events than walking** (rule 18), and each one reset a
   searching enemy's give-up timer, so sneaking made escape harder.
5. **Enemies slid with no run animation** — acceleration was zero.
6. **Player death was DisableMovement and nothing else**: standing in idle, still able to
   punch.

### Where I corrected the mentor

- **The footstep model.** Told that noise fired on a timer, I asked whether it fired "when
  I step with a specific foot" — which is what it should do. It now fires per distance.
- **`AutoSuccessRangeFromLastSeenLocation`.** The mentor described it as "you cannot sneak
  up on someone by standing on their toes." My two experiments showed the opposite: it is
  anti-loss only, and sneaking up to touch an unaware enemy works fine.
- **Unusable editor instructions.** The Anim Blueprint debug-object walkthrough was too
  compressed to follow. Rewritten, and the actual diagnosis was done with a log line
  instead — the right call, since a log line costs me nothing.
- **The mentor's own mistakes this phase:** two blind range deletes in the source (one
  duplicated a block, one deleted `HandleMontageEnded` and broke the link); "yellow arrow
  on the left" for a reset indicator that is grey and on the right; "Create Advanced
  Asset" for a menu heading that has not existed since UE 5.6.

### The course correction — 2026-09-02

Combat works and feels unpolished, and that is not a systems problem: no hit stop, no
sound, no animation blending, no invincibility frames, a roll standing in for a sidestep,
and montages authored for something else. That layer is animation and audio work.

More importantly, the sessions had become **write C++ → build → set a dropdown →
repeat**. The remaining gap is the editor, not the code. Phases 6+ are reorganised around
one editor domain per phase; packaging is dropped; the final phase becomes planning the
real project. See CLAUDE.md.

### Known debt

- **No team system.** `IsPlayerControlled()` is a stand-in in two places. Enemies also
  friendly-fire each other, which was accepted deliberately.
- **Damage reaction is still duplicated** between player and enemy, and death montage
  selection now is too — the player has one montage where the enemy picks directionally.
  This is the next extraction.
- **Footsteps should be anim notifies** on the actual contact frames, not a distance
  approximation. That is also where the sound goes in Phase 7.
- **`Use Acceleration for Paths` is a Blueprint tick**, not C++ — losable, and invisible
  to anyone reading the source.
- **No patrol.** `Relaxed` stands still.
- **The enemy never throws its heavy attack**, and has no dodge or block.
- **The dodge still has no invincibility frames**, so it is travel rather than defence.
- **`bShowAttackTrace` and `bShowStateDebug` both default to true.**

### Phase progress

- [x] **Phase 0 — Orientation**
- [x] **Phase 1 — Input & movement**
- [x] **Phase 2 — Stats as a component**
- [x] **Phase 3 — Melee combat**
- [x] **Phase 4 — Data-driven design**
- [x] **Phase 5 — Enemy AI & perception** (AIController, sight, hearing, four detection
      states, sneaking, the melee attack component, enemy attacks, player death)

Roadmap reorganised 2026-09-02 — phases below are one editor domain each. See CLAUDE.md.

- [ ] Phase 6 — Blueprint, properly
- [ ] Phase 7 — Audio
- [ ] Phase 8 — Materials and post-process
- [ ] Phase 9 — UI and menus
- [ ] Phase 10 — The world: level and lighting
- [ ] Phase 11 — VFX with Niagara
- [ ] Phase 12 — Animation, deeper
- [ ] Phase 13 — Framework and persistence
- [ ] **Phase 14 — Planning the real project**

---

## Session 13 — 2026-09-03 → 2026-09-04 — Phase 6.1: the Blueprint editor, mapped

Branch `phase6-blueprint`. **Nothing was built.** A guided tour of
`BP_ThirdPersonCharacter` with the editor open, verifying every UI claim against the
running editor rather than against tutorials — which caught two errors in my own
roadmap before we got past the second panel.

### What exists now

Nothing new. No files changed except this log and CLAUDE.md.

### What I can explain now

- **A Blueprint asset is one class, edited through three tabs.** In Roblox, a Model holds
  the Parts and a Script inside it holds the behaviour — two places. Unreal folds the
  parts list, the code, and the default property values into a single asset. The tabs
  (**Viewport / Construction Script / EventGraph**) are views of one class, not files.
- **`BP_ThirdPersonCharacter` is itself a class**, a subclass of `ALearningUECharacter`
  authored in an editor instead of a text file. At runtime its name carries a `_C`
  suffix — `BP_ThirdPersonCharacter_C` — Unreal's marker for "class generated from a
  Blueprint." The same suffix appears on `ABP_Unarmed_C` in the Mesh's Anim Class field.
- **In the Components panel, indentation is the contract and vertical order is
  decoration.** Indentation means attachment, exactly like Roblox parenting; it is a
  drawing of the `SetupAttachment()` calls in the C++ constructor. `SetupAttachment` is
  `instance.Parent = other`.
- **The root is the collision capsule, not the body.** The skeletal mesh hangs off it at
  `Z = -89` so the feet reach the bottom of the capsule instead of the hips sitting at
  its centre. Movement moves the capsule; the mesh follows.
- **The divider line in the Components panel** is inserted immediately after the root
  scene component's whole subtree (verified in `SSubobjectEditor.cpp:2652`). It means
  "above: the root and everything attached to it; below: components attached to
  nothing." It is *not* a C++-vs-Blueprint line and not an engine-vs-my-code line.
- **What decides which side something lands on is the TYPE, not my code.** A
  `USceneComponent` has a transform, so it must hang off something — if it were left
  unattached the editor would still parent it to the root. A `UActorComponent` has no
  transform, so there is nothing to hang.
- **A movement component is a brain, not an object in the world.** It holds an
  `UpdatedComponent` pointer aimed at the capsule and writes a new position into it
  every frame. Asking where it is located is like asking where a `for` loop is located.
- **Blueprint-added components are unnamed to C++, not invisible.** No compile-time
  pointer exists, so `PointLight->SetIntensity()` cannot compile — but
  `FindComponentByClass<UPointLightComponent>()` finds it at runtime. Not in the C++
  constructor, though: that runs before the Blueprint's own components exist. `BeginPlay`
  is the earliest point that works.
- **My Blueprint → VARIABLES lists only variables this Blueprint declares itself.**
  Inherited C++ properties never appear there; they appear in Class Defaults and the
  Details panel. An empty VARIABLES section means "added none of its own", not "has none".
- **Class Settings vs Class Defaults.** Settings = facts about the class — parent class,
  interfaces, abstract. Defaults = starting values for every property, i.e. editing the
  CDO. *If it changes what the class **is**, it is Settings; what it **starts with**,
  Defaults.*
- **A Blueprint Interface is a contract** — function signatures with no bodies. The
  declared-up-front version of the `FindComponentByClass` move: ask what a thing can do,
  not what it is. Web anchor: a TypeScript `interface`.
- **The Override dropdown's right-hand column is an ORIGIN column, not a history
  column.** `AnyDamage → Actor` means `AActor` declares it, not that anyone overrode it.
  Read down the column and the inheritance chain appears:
  `AActor → APawn → ACharacter → ALearningUECharacter → BP_ThirdPersonCharacter`.
- **Construction Script is not the C++ constructor.** The constructor runs once at engine
  startup building the CDO, with no world to touch. The Construction Script runs on every
  placement, move, or property edit *in the editor with the game stopped*, and again at
  spawn before `BeginPlay`. It re-runs constantly and discards what it made last time, so
  it must be pure.

### What the tour revealed about this project

The player character's **entire Blueprint contribution** is: asset pointers
(`SKM_Quinn_Simple`, `ABP_Unarmed_C`, `MI_Quinn_01/02`) plus a four-function adapter
implementing the `Touch` interface, each forwarding straight into C++ (`Do Look`,
`Do Move`, `Do Jump Start`, `Do Jump End`). Zero variables, zero event dispatchers, zero
components of its own — every row in the Components panel reads `Edit in C++`.

The boundary policy of this project is not written down anywhere. It is **visible as two
empty lists**.

### Roadmap corrections made this session

Both found by reading the running editor, exactly as the Phase 6 method requires:

1. **My Blueprint has SIX sections, not eight.** GRAPHS, FUNCTIONS, INTERFACES, MACROS,
   VARIABLES, EVENT DISPATCHERS.
2. **The tab is `EventGraph`, one word** in 5.8.

### Where I corrected the mentor

- Asked why Character Movement sits above the divider when it is the same flat kind as
  Stats. It does not fit the rule I had been given. It turned out the panel's vertical
  order is decided by a display-sort step with no structural meaning — the honest answer
  was "the layout lies about its own grouping; trust the indent."
- The Components panel had been scrolled, hiding the `BP_ThirdPersonCharacter (Self)`
  root row. Without that row the indent levels cannot be read at all.

### Phase progress

- [x] **Phases 0–5** — see above
- [ ] **Phase 6 — Blueprint, properly** — 6.1 done (anatomy tour)
- [ ] Phase 7 — Audio
- [ ] Phase 8 — Materials and post-process
- [ ] Phase 9 — UI and menus
- [ ] Phase 10 — The world: level and lighting
- [ ] Phase 11 — VFX with Niagara
- [ ] Phase 12 — Animation, deeper
- [ ] Phase 13 — Framework and persistence
- [ ] **Phase 14 — Planning the real project**

---

## Next session starts here

**State:** Phases 0–5 merged to `main`. On branch `phase6-blueprint`, one commit in:
documentation only, no code and no assets touched. The game is unchanged and runs.

**Next:** **Phase 6.2 — EventGraph grammar.** Events, nodes, execution wires vs data
wires, variables. Roadmap order unchanged: 6.3 pickup (Blueprint-only overlap/heal/
destroy) · 6.4 Construction Script, the self-configuring actor · 6.5 functions, macros,
interfaces · 6.6 event dispatchers · 6.7 the C++/Blueprint boundary keywords · 6.8 the
5.8 Create menu.

At the end of 6.1 I asked, out of curiosity, whether a Construction Script actor could be
built purely in the editor with no C++. The answer is yes and that is 6.4 — it was a
question, not a request to reorder anything.

**Read first:** CLAUDE.md — the UE 5.8 facts table. Two more entries earned this session:
My Blueprint has six sections, and the tab is `EventGraph`.

**Do not:** propose combat polish, or re-add packaging. Both were ruled out deliberately.
