# ARCANE_BOOTSTRAP.md — starting Project Arcane

Two parts. Part 1 is for Artem, done by hand before the first session. Part 2 is for
the Claude session that finds this file in the new folder — it executes the bootstrap.

---

## Part 1 — Artem, before the first session

1. **Have the ideas document ready.** Everything in your head about the game:
   mechanics, fragments, worries, things you have NOT decided. Any structure, any
   length. Name it `IDEAS.md`.

2. **Create the GitHub repository.** On github.com: New repository → name **Arcane**
   → **Private** → do NOT add a README, .gitignore, or license (the repo must be
   completely empty).

3. **Create the local folder** `C:\Projects\Arcane`.

4. **Copy three files into it:** this file, `DECISIONS.md`, and `IDEAS.md`.

5. **Open a terminal in that folder and run `claude`.** First message:
   *"Read ARCANE_BOOTSTRAP.md and execute Part 2."*

That's all. Everything else is Claude's job.

---

## Part 2 — Claude, in the bootstrap session

Read everything before doing anything.

1. **Read the constitution.** Read `DECISIONS.md` in this folder — every decision
   in it is binding. Highlights: GAS · UE 5.8 · four-rule C++/Blueprint boundary ·
   GitHub private + LFS from commit zero · monorepo layout · standard naming
   prefixes · one C++ module `Arcane` · pain-first tooling, free/local only ·
   division of labour — Claude does everything possible, Artem handles what Claude
   cannot (editor lifecycle, builds, installs, logins) · pull-based teaching ·
   phase-based work · pre-production before development.

2. **Initialize git — LFS before anything else.** `git init` (branch `main`), then
   `.gitattributes` with LFS patterns for `*.uasset`, `*.umap`, textures, audio,
   meshes, `*.blend`, archives — BEFORE any other file is committed. Add a
   UE-ready `.gitignore` (Binaries, DerivedDataCache, Intermediate, Saved, .vs,
   *.sln — dormant until Game/ gets a project). Remote:
   `https://github.com/artemfrolov/Arcane.git`. **Pin the credential account:**
   `git config credential.https://github.com.username artemfrolov` — this machine
   has two GitHub accounts in the credential manager and pushes hang on an
   invisible account-picker without it.

3. **Build the monorepo skeleton** per the repo-layout decision: `Docs/` (move the
   local `DECISIONS.md` there, plus `Docs/systems/`, empty), `Game/` (empty — stays
   empty until pre-production ends; no UE project yet), `Assets/` (empty — Blender
   sources later). Root: `README.md`, `SESSION_LOG.md`, `CLAUDE.md`. This
   bootstrap file itself can be deleted once executed.

4. **Write CLAUDE.md** from the "How we work" section of DECISIONS.md: the
   division of labour, working rhythm, and standing rules, phrased as operating
   instructions for every future session.

5. **Ingest IDEAS.md.** Read it fully — it is the sole source of truth for the
   game's design intent. Do NOT decide anything, do NOT start designing in the
   bootstrap session. Deliverable: a proposed **design-phase agenda** — the list
   of design areas (whatever IDEAS.md surfaces: attributes/skills, combat, magic,
   world, AI/simulation, economy, …), ordered, one session each, for Artem's
   approval. The first agenda item should be distilling `Docs/VISION.md` out of
   IDEAS.md — a short statement of what the game is, drafted by Claude, decided
   by Artem. Design sessions then produce `Docs/systems/*.md` one at a time.

6. **Commit and push** (small commits per step, push after each — standing rule),
   update `SESSION_LOG.md`, and end with the agenda proposal.

Not in this session: no UE project, no MCP servers, no C++, no asset imports.
Pre-production is documents.
