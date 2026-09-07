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

4. **Copy two files into it:** this file, and your `IDEAS.md`.

5. **Open a terminal in that folder and run `claude`.** First message:
   *"Read ARCANE_BOOTSTRAP.md and execute Part 2."*

That's all. Everything else is Claude's job.

---

## Part 2 — Claude, in the bootstrap session

Read everything before doing anything.

1. **Read the constitution.** Read `DECISIONS.md` from the learning repo at
   `C:\Projects\UE\LearningUE\DECISIONS.md` — all decisions 14.1–14.5 are binding.
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

3. **Build the monorepo skeleton** per decision 14.2: `Docs/` (move `DECISIONS.md`
   there — copy it from the learning repo — plus `Docs/systems/`, empty), `Game/`
   (empty — stays empty until pre-production ends; no UE project yet), `Assets/`
   (empty — Blender sources later). Root: `README.md`, `SESSION_LOG.md`,
   `CLAUDE.md`.

4. **Write the new CLAUDE.md** implementing decision 14.4 — the working agreement,
   not the mentor-mode rules of the learning project. Port what survives (git
   discipline, answerable questions, free-assets-first, honesty about experimental
   tools, English everywhere, session log); drop what died (exercises, term caps,
   editor-work-reserved-for-Artem, per-change go-aheads).

5. **Write Docs/VISION.md** — port "The Destination" from the learning repo's
   `C:\Projects\UE\LearningUE\CLAUDE.md`, cleaned up as a standalone document.
   `SESSION_LOG.md` there holds project history if context helps.

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
