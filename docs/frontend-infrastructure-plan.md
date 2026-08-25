# People Party Frontend — Infrastructure Optimization Plan

*Created: 2026-08-21 — from a dedicated frontend-optimization planning session (backend is
already optimized for efficiency; this plan does the same audit for the Vue 3 frontend).*

## How to use this document

This plan is meant to be implemented **phase by phase, each in its own fresh session**, to keep
context small and focused instead of one huge implementation effort. At the start of a session:

1. Read this whole file (it's the only context you should need to pick up work).
2. Check the status tracker immediately below for the next phase marked "Not started."
3. Implement only that phase.
4. Before ending the session, update that phase's row — status, plus a short note (what was done,
   what's left, any deviation from the plan and why) — so the next fresh session picks up cleanly.

The recommendations, decisions, and rejected alternatives below were already researched and
decided deliberately — don't re-litigate them from scratch unless new evidence genuinely
contradicts them. If that happens, note the change in the relevant phase's row rather than
silently diverging from what's written.

## Status tracker

| Phase | Work | Status | Notes |
|---|---|---|---|
| 0a | Recommendation A — CSS-ify the background, vendor chunking + bundle analyzer, naive-ui packaging fix, reuse existing `isMobile()` in `main.ts` | Done | Implemented 2026-08-21. `BackgroundComponent.vue` rewritten as pure CSS (`mask-image` + `background-color: var(--color-secondary-dark)`, tint verified against decoded pixel data — no store import needed). `vite.config.ts` got `manualChunks` (function form) splitting `pixi`/`naive-ui` into named chunks, plus an on-demand-only bundle analyzer (`npm run build:analyze`, `--mode analyze`, decided with user rather than always-on — no new dependency needed). `isMobile()` also patched with an iPad feature-detection fallback (decided with user, beyond the plan's literal text — the plan's own justification for the swap didn't actually hold, see below). **Deviation/finding not anticipated by the plan**: `manualChunks` initially caused Rollup to hoist Vite's own tiny dynamic-import preload helper into the `pixi` chunk, which made the entry chunk eagerly import all 814KB of it on every page load — silently defeating the point. Caught by actually inspecting the built `dist/index.html`'s `modulepreload` hints and the entry chunk's own import statements, not by assuming the config was correct. Fixed by explicitly bucketing `id.includes('vite/preload-helper')` into its own tiny chunk in `manualChunks`. Confirmed via production build: `index.html` no longer preloads the `pixi` chunk on any route, only `naive-ui` (genuinely eager, unrelated to this fix) and the 1KB helper chunk. **Anyone touching `manualChunks` again should rebuild and check `dist/index.html`'s preload links directly — don't assume a chunk being "separate" means it isn't eagerly loaded.** |
| 0b | CI gate — PR-triggered `type-check` + `lint` + `prettier --check` (part of Recommendation C) | Done | Implemented 2026-08-21, same session as 0a. Two pre-existing breakages had to be fixed before the gate could work at all (neither was anticipated by the plan, both found by actually running the checks, not by inspecting config): (1) `.eslintrc.cjs` had a dead `overrides` block extending `plugin:cypress/recommended` — the plugin isn't installed and zero files match its glob, but ESLint eagerly resolves every configured plugin at startup regardless, so `npm run lint` crashed outright for every file, unconditionally, before this session. Removed the dead block. (2) `prettier --check src/` failed on 148 files: 101 auto-generated FlatBuffers files (now excluded via new `frontend/.prettierignore`) and 47 real hand-written files with genuine pre-existing debt — reformatted all 47 via one `prettier --write` pass so the new gate starts green rather than red-by-default for every future PR regardless of that PR's own diff. New `.github/workflows/frontend-checks.yml` also needed an explicit FlatBuffers-codegen step (`npm install && npm run build` at repo root, copied from the already-proven pattern in `publish-docker.yml`/`staging-publish.yml`) before the frontend `type-check` step, since `frontend/src/flatbuffers/` is gitignored and won't exist on a fresh checkout otherwise. Added non-mutating `lint:check`/`format:check` npm scripts (the existing `lint`/`format` scripts run `--fix`/`--write`, which would silently pass in CI instead of failing). All three checks (`type-check`, `lint:check`, `format:check`) verified clean against the final tree. Nothing committed — implementer should review `git status` and commit (suggested structure: tooling-config fix → mechanical reformat → feature changes → CI workflow, each as its own commit) before opening a PR. |
| 1 | Build the PixiJS composable layer (`useGameCanvas` / `createEntityLayer` / `useSnapshotBuffer`), manually verified (part of Recommendation B) | Done | Implemented 2026-08-25. New `frontend/src/composables/pixi/{useGameCanvas,createEntityLayer,useSnapshotBuffer,index}.ts` — no existing minigame touched, migration is Phase 2. Design verified directly against the installed `pixi.js@8.10.2` source (not just `.d.ts`/memory): `Application.init()` confirmed genuinely async, `autoStart:false` + a hand-rolled shared rAF loop replace Pixi's own Ticker, legacy `beginFill()`-style Graphics calls confirmed real `@deprecated` working methods (safe for Phase 2 to lift forward if it wants, though new code here uses only the modern fluent API). `useGameCanvas` exposes a synchronous `root: Container` (added to the real `Application.stage` once init resolves) rather than the `Application` itself, since the latter can't exist synchronously — `root`'s own transform doubles as the letterbox scale/offset, so draw code is written once in world units instead of every current minigame's per-coordinate `x * scale` pattern. **Real bug caught by live verification, not by code review (mine or an independent design-review pass both missed it):** the shared rAF loop's `invalidate()` guard used `rafHandle===null` to mean "loop is idle," but that's also true *while a frame is executing* (deliberately nulled at the top of each tick) — so an `animate` callback calling `invalidate()` (an expected, normal thing to do, e.g. `useSnapshotBuffer.push()`) scheduled a second frame on top of the tail's own scheduling, doubling every frame. Confirmed live via a Playwright-driven headless-Chrome smoke test: 8191 renders in 500ms (≈2¹³) against a measured ~62-in-500ms baseline on a blank page in the same browser — not a headless-mode artifact, a real exponential runaway. Fixed with an explicit `frameInProgress` flag so mid-frame `invalidate()` calls are inert (the frame's own tail (re)schedules once, after finishing) plus per-entry try/catch so one canvas's throwing callback can't wedge the shared loop for every other registered canvas. Re-verified after the fix: renders during interpolation dropped to 62/500ms, matching the baseline exactly. Verification harness: `frontend/src/routes/PixiPlaygroundRoute.vue` + one dev-only-gated route added to `frontend/src/router/index.ts` (`import.meta.env.DEV`, tree-shaken out of production builds) — exercises all three composables against synthetic bouncing balls with on-screen counters (renders-since-last-tick, canvas-element-count) making the plan's three verification targets objectively checkable rather than eyeballed. Kept permanently (user's call) as a standing dev playground/reference for Phase 2. `chromium-cli` wasn't available in this environment; drove a headless local Chrome install via a scratch `playwright-core` script instead (not committed to the repo). Nothing committed to git. **Anyone touching the shared rAF loop again: re-run the playground's interpolation check and confirm the render count roughly matches a plain blank-page rAF baseline — a silent runaway like this one won't show up in type-check/lint/format, only in an actual running page.** |
| 2 | Migrate `shellShuffle` (reference), then `marbleMania` as go/no-go gate (part of Recommendation B) | Not started | |
| 3 | Rewrite `create-minigame-frontend` skill + `CLAUDE.md` PixiJS section to match the new layer (part of B/C) | Not started | |
| 4 | Backfill remaining 5 minigames (`highwayHustle`, `teambasedPong`, `rpsBracket`, `businessBailout`, `crazyCounting`); remove `vue3-pixi`; add ESLint import ban | Not started | |
| C-rest | Typed payload helper (`readPayload`), `MinigameView` compiler contract, staged `strict` rollout in `tsconfig.app.json` | Not started | |
| C-docs | Make `CLAUDE.md` sole source of truth; delete `.github/copilot-instructions.md` (and mirrored `.github/skills/` if present); CI check for backend/frontend name coupling | Not started | |
| E | WebSocket reconnect-with-backoff; fix `GameManager.vue` `ResizeObserver` leak | Not started | |
| D | Dependency version currency (Tailwind v4, Pinia 3, ESLint 9, flatbuffers sync, TS bump) + stray dependency cleanup | Not started | ongoing/opportunistic, no fixed phase |

---

## Context

The backend was built for efficiency from day one (C++, Box2D, FlatBuffers binary protocol,
~100–800 byte payloads). The frontend never got the same scrutiny. The goal of this session was
to determine whether the Vue 3 frontend is already "good enough," needs targeted fixes, or
warrants a bigger infrastructure change (different framework, different rendering engine,
different component libraries) — considered with an open mind, no option ruled out in advance.

Two scope boundaries were set during research:
- **Mobile (player) experience is the priority; host (PC screen) is explicitly lower priority** —
  though still important; host-side PixiJS quality is a first-class part of this plan, not an
  afterthought. This matters below because it turns out the two devices have almost entirely
  different performance profiles today.
- **One-off content/asset transfer issues are out of scope** (e.g. the uncompressed 6.9MB
  soundtrack file) — a dedicated sound infrastructure will be designed later once more audio is
  added. This plan is scoped to architecture: rendering, realtime state flow, build tooling, and
  AI-agent/developer maintainability.

Research method: three parallel codebase-exploration passes (build tooling/deps, PixiJS rendering
patterns, core app architecture) followed by three parallel, independent design/critique passes
(one steelmanning a full framework/engine rewrite, one designing a concrete rendering-architecture
fix, one evaluating everything through an AI-agent-maintainability lens). All three design passes
converged on the same conclusion from different angles, and the most important factual claims were
independently verified twice more via direct file reads and git history. The plan below reflects
that converged, verified picture.

---

## Headline recommendation

**Keep Vue 3 + Vite. Do not rewrite the framework. Keep 2D canvas rendering, but delete the
`vue3-pixi` wrapper library and rebuild the rendering layer on raw PixiJS v8 behind one shared,
disciplined composable.** In parallel, close the gap that makes this a project where an AI coding
agent writes a meaningful share of the code: enforce (not just document) TypeScript strictness,
typed payload boundaries, and CI checks.

The single most important discovery of this research: **the frontend currently ships two entire
copies of PixiJS (versions 7 and 8) in one bundle, and forces every mobile player to download,
parse and run both — permanently, on every screen, including the pre-game join/lobby flow — purely
to render a decorative scrolling background that could be four lines of CSS.** Fixing that one
architectural mistake is worth more to the stated goal ("optimize the player's experience,
mobile-first") than any framework or rendering-engine choice discussed below.

---

## Finding 1: the app runs a broken, duplicated rendering stack (the centerpiece issue)

`vue3-pixi` (the Vue binding library used for all canvas-based minigames, currently `0.9.6`,
pre-1.0) declares PixiJS as a **hard dependency, not a peer dependency**:

```json
// frontend/node_modules/vue3-pixi/package.json:32-38
"dependencies": {
  "pixi.js": "^7.4.2",
  ...
}
```

The project's own top-level pin is `pixi.js: ^8.10.1`. Because of the mismatch, npm nests a
private copy: `node_modules/vue3-pixi/node_modules/pixi.js` resolves to **7.4.3**, alongside the
top-level **8.10.2**. Both ship in the production bundle — confirmed by finding v7-only symbols
(`DisplayObject`, `BaseTexture` — both removed in v8) inside the built `dist/assets/index-*.js`
chunk. Every `<Application>`, `<Graphics>`, `<Sprite>`, `<Text>` element in every minigame
instantiates a **v7** object; the v8 imports in a couple of files (for `TextStyle`,
`CanvasTextMetrics`) are cross-version calls that happen to work today by accident and are marked
with `@ts-expect-error` where they don't quite line up
(`marbleMania/HostView.vue:585`, `highwayHustle/HostView.vue:187`).

This is worse than a stale dependency — it actively misleads. `CLAUDE.md` and
`.github/copilot-instructions.md` both currently state "PixiJS (`vue3-pixi` + `pixi.js` v8)" as
fact. An AI agent (or a human) reading the project's own documentation, or reading the v8 package
that's genuinely installed at the top level, will write PixiJS v8 API calls against a v7 runtime.

**This single fact also explains bugs found independently while auditing every canvas minigame:**
- `vue3-pixi`'s `@render` prop is not a per-frame ticker callback — it wires the handler into a Vue
  `watchEffect` (`vue3-pixi/dist/index.js:528`, confirmed by direct inspection). Redraws fire only
  when a reactive value the draw function happened to read last time changes — an invisible,
  implicit dependency graph.
- `teambasedPong/HostView.vue:295-301` has a `watch(payloadData, () => {/* nothing */}, { deep:
  true })` — a cargo-culted attempt to force a redraw by a developer who (reasonably) assumed a
  normal per-frame ticker model.
- `marbleMania/HostView.vue:573-578,634` and `highwayHustle/HostView.vue:136-153,228` each destroy
  and recreate the **entire PixiJS Application** (canvas + WebGL context + renderer) via a Vue
  `:key` binding whenever the entity count changes — a sledgehammer workaround for the same
  invisible-dependency problem, and a genuine WebGL-context-exhaustion hazard (browsers cap
  concurrent contexts) tied to routine gameplay (a player's marble finishing).
- `highwayHustle/HostView.vue:228` hardcodes `<Application :width="800" :height="530">` with zero
  resize handling at all — inconsistent with the ResizeObserver+scale pattern used (with varying
  quality) everywhere else.
- No shared composable for canvas setup exists anywhere, so all 7 canvas-based minigames
  reimplement resize handling and render wiring from scratch, independently, with wide quality
  variance — which is the root cause of the inconsistency above, not any single bug.

## Finding 2: the original performance hypothesis needed correcting — say so plainly

Initial investigation assumed PixiJS redraws on an uncapped, display-refresh-rate ticker,
fully decoupled from the backend's 20–30Hz push cadence — a "2–3x redundant redraw" story. Deeper
inspection of `vue3-pixi`'s internals (above) showed the actual draw *logic* already runs at data
cadence via `watchEffect`, not at 60Hz+. The real waste is different and still real:
- PixiJS's own `Ticker` (`autoStart: true` by default) calls `renderer.render()` — reissuing GPU
  draw calls for whatever is currently in the scene graph — at full display refresh regardless of
  whether anything changed.
- The decorative background (Finding 3) runs **two** permanent `requestAnimationFrame` loops
  (its own Application's ticker, plus a write to `Ticker.shared` for position updates) on **every
  screen, on every device, for the entire party** — this is the one continuously-running cost that
  affects mobile, and it has nothing to do with gameplay.
- Per-frame allocation smells found in the canvas code (fresh `Set`/`.map()`/`.sort()`/`.filter()`
  inside draw callbacks in teambasedPong, shellShuffle, marbleMania) are real but minor at this
  cadence and these entity counts (≤16 players/marbles) — worth cleaning up as part of the
  refactor below, not a standalone justification for one.

## Finding 3: mobile devices carry zero canvas rendering today — the host carries all of it

Confirmed by grep across every `PlayerView.vue`: **none of the 11 minigames render PixiJS/canvas
content on the player's phone.** All 7 canvas-based minigames render exclusively in `HostView.vue`.
Mobile `PlayerView`s are plain DOM/CSS/Naive UI plus the on-screen joystick. All game physics
(including Box2D for 2 games) is computed server-side; the frontend — host or mobile — only ever
renders scalar positions it's told.

This means: the rendering-loop discipline problems in Finding 1 affect the **host**. The bundle
problem in Finding 1 (two PixiJS copies loading on every screen) affects **mobile**, which is the
higher priority. These are two different fixes serving two different devices — don't conflate them
when sequencing work, and don't treat the host-side fix as unimportant just because mobile is the
priority; both matter.

---

## Recommendation A — Get PixiJS off the mobile critical path (do this first)

**Replace `BackgroundComponent.vue`'s PixiJS-based scrolling background with CSS.** The current
implementation (`BackgroundComponent.vue`) is a 512px tiling PNG, scrolled by writing a fresh
`{x, y}` object into a Vue ref every tick (`bgPos.value = { x: xPosition, y: 0 }`,
line 36) and tinted from the color store. That is exactly `background-image` +
`background-repeat` + one `@keyframes` animating `background-position-x`, GPU-composited by the
browser with zero JS and zero WebGL context. `App.vue:3,35` currently mounts this component
unconditionally behind every route (`<RouterView>` + `<Background>` siblings, no host/mobile
branch) — confirmed by direct read — so every mobile player pays for it from the moment they load
the join page, before a party even starts.

Concrete wins from this one change: removes ~150–200KB gzip of duplicated PixiJS from the mobile
critical path, removes a permanent WebGL context and two always-on animation loops from every
player's phone for the entire party (meaningful for battery/thermals given the app also holds a
screen wake lock the whole session), and incidentally fixes an existing bug where the scroll speed
is accidentally tied to display refresh rate (twice as fast on a 120Hz phone as a 60Hz one).
Bonus, previously impossible: respect `prefers-reduced-motion`, and pause via
`animation-play-state` when the tab is hidden.

Do alongside this, same low effort/high value bucket:
- Add `build.rollupOptions.output.manualChunks` in `vite.config.ts` to split PixiJS and Naive UI
  into named vendor chunks, plus a bundle analyzer (`rollup-plugin-visualizer`) as a dev dependency.
  There is currently no visibility into bundle composition at all — this instruments every
  subsequent change in this plan so impact is measured, not assumed.
- Move `naive-ui` from `devDependencies` to `dependencies` in `package.json` — it's used at
  runtime in production (`App.vue:4`); it currently only works by accident of how Vite bundles.
- In `main.ts`, replace the mobile/orientation-handling gate (`/Mobi|Android/i.test(navigator
  .userAgent)`) with the project's own existing, more complete `isMobile()` util
  (`src/util/detectmobilebrowser.ts`, already used correctly by the router). The regex check
  silently excludes iPadOS Safari (default UA doesn't match "Mobi"/"Android"), meaning iPads
  currently get none of the viewport-height/orientation-lock handling every other mobile browser
  gets. This is a one-line fix reusing code that already exists.

## Recommendation B — Rebuild the rendering layer on raw PixiJS v8, drop `vue3-pixi`

**Why replace `vue3-pixi` with a custom layer, rather than keep using it or wait for it to support
v8?** Two separate reasons:
1. It isn't just outdated, it's structurally stuck. It hard-depends on PixiJS v7 (Finding 1), and
   even a hypothetical future update wouldn't be a small bump — PixiJS v8 changed `Application`
   initialization from a synchronous constructor to an async `app.init()` and removed classes
   (`DisplayObject`) the wrapper's custom renderer is built around internally. There's no cheap path
   from here to a v8-compatible version of this specific wrapper.
2. Even independent of the version problem, the *style* of abstraction is the wrong shape for this
   app. `vue3-pixi`'s declarative `<Graphics @render>` model wires redraws to an implicit, invisible
   Vue reactivity dependency graph — confirmed by reading its source, it's a `watchEffect` under the
   hood, not a per-frame callback. That's a defensible design for a generic "make Pixi feel like
   Vue" library, but it's the direct, traceable cause of the two worst bugs found anywhere in this
   codebase: a dead no-op deep-watcher and two independent `:key`-driven full-Application-recreation
   hacks, all written by developers fighting a redraw trigger they couldn't see. In a real-time
   multiplayer game — where "exactly when does this redraw" is a meaningful question tied to a
   20–30Hz server cadence, not an incidental detail — that trigger is worth making explicit and
   visible in code that lives in this repo, instead of implicit inside a third-party reconciler.

A hand-rolled layer should be *shared*, not duplicated per minigame — that is the entire point of
the composable design below. One small internal library (`useGameCanvas`, `createEntityLayer`,
`useSnapshotBuffer`) that every canvas-based `HostView` imports, so Application setup, resizing,
and redraw-cadence logic exist in exactly one place instead of being reimplemented 7 times with
inconsistent quality — which is the current state regardless of whether a third-party wrapper sits
underneath. It's a small, purpose-built, in-house replacement for what `vue3-pixi` was supposed to
provide: fully visible and debuggable by whoever — human or AI agent — needs to work on it next,
with no `node_modules` archaeology required to understand what it does or why a redraw happened.

**Engine choice — decided: PixiJS v8, used directly, no wrapper.** Auditing every draw call across
all 7 canvas minigames showed the app currently uses zero WebGL-differentiating features anywhere
(no filters, masks, blend modes, meshes, particles, shaders, render textures — just rects, circles,
lines, polygons, sprites, and text, on ≤16 entities); that workload maps 1:1 to plain Canvas2D,
which would have been simpler and carried zero version-skew risk ever again. Weighed against
PixiJS v8 directly (smaller migration delta from today's state since v8 is already a top-level
dependency, headroom for flashier celebratory effects — particle bursts, glow/blur on wins — as the
game grows, and real sprite-batching benefit for `highwayHustle`'s cars/obstacles), **the project
owner chose PixiJS v8** for the effects headroom, not because Canvas2D was a weaker option. The
composable boundary below keeps this low-regret — a specific minigame could still move to Canvas2D
later at low cost if that ever looks better for it.

**The composable layer** (new directory, e.g. `frontend/src/composables/pixi/`):

- **`useGameCanvas(options)`** — owns the entire Application lifecycle: creates the canvas and
  calls `await app.init(...)` once in `onMounted`, destroys it once in `onScopeDispose`, and is
  never recreated by a `:key`. Owns sizing: accepts an explicit `size()` getter (preferring
  `GameManager`'s already-computed width/height over a second redundant `ResizeObserver`) or falls
  back to installing exactly one `ResizeObserver` itself — never both, which is what happens today
  in games that ignore the `width`/`height` props they're handed while also running their own
  observer. Owns cadence: redraws are gated by an explicit `invalidate()` call (a dirty flag on one
  shared, module-level `requestAnimationFrame` loop), not a `maxFPS` cap — a cap still renders at
  the cap when nothing changed; a dirty flag renders nothing while idle. An opt-in `animate(ctx)`
  hook with a supplied `dt` exists for games that want frame-rate-independent interpolation between
  network ticks (see the teambasedPong smoothing bug below).
- **`createEntityLayer(parent, { key, create, update, release })`** — a small object pool keyed by
  entity id, diffed against each new snapshot: present entities get `update()`, new ones get
  created-or-reused-from-pool, removed ones return to the pool instead of being destroyed. This is
  the direct, non-hacky replacement for the `:key`-driven Application recreation in `marbleMania`
  and `highwayHustle` — an entity-count change becomes a `Map` diff against a stable WebGL context,
  not a renderer teardown.
- **`useSnapshotBuffer()`** — holds Pixi-consumed game state as a plain mutable object (not a deep
  `ref`), since nothing needs Vue to react to it — the draw callback reads it directly, and
  `push(next)` is the only way to update it, which also calls `invalidate()` internally so a
  forgotten-invalidate class of bug can't exist. A small explicit `hud` `shallowRef` carries the
  handful of fields a DOM overlay (score, timer, phase label) actually needs, replacing the
  discipline-free approach of dumping everything into one deeply-reactive object and hoping the
  right things re-render.

This design directly fixes, not just works around: `marbleMania`'s Application-recreation-on-
marble-count-change; `highwayHustle`'s non-responsive hardcoded canvas; the dead no-op deep watcher
in `teambasedPong`; and a real latent gameplay bug in `teambasedPong/HostView.vue:104-114`, where a
fixed-factor (`0.2`) smoothing calculation is commented "avoid marker teleporting between network
ticks" but — because draws only ever fire once per snapshot today — never actually interpolates
between ticks at all; it's a mislabeled lag filter. The new `animate(ctx)` + `dt` hook is what would
make that comment true.

**Migration sequence:**
1. Ship Recommendation A, delete the dead deep-watcher in teambasedPong, add the vendor-chunk
   split — all independently valuable and non-blocking.
2. Build the composable (`useGameCanvas` / `createEntityLayer` / `useSnapshotBuffer`). No automated
   test suite runs for this frontend today (Cypress is scaffolded but unused, and standing up real
   test infrastructure is a deliberately separate decision for later — out of scope here). Verify
   manually instead that: redraws stop while idle, entity-count changes never recreate the
   `Application`/canvas, and `dt`-based smoothing looks consistent regardless of display refresh
   rate. The first reference migration (next step) is a good place to confirm all three concretely.
3. Migrate `shellShuffle` first — `<Graphics>`-only, 3 entities, already has the best existing
   resize handling of the seven, so it's a clean proof of the sizing/overlay-alignment API.
4. Migrate `marbleMania` second, as an explicit **go/no-go gate** — it's the hardest case
   (pooled entities + a static obstacle layer + camera-as-transform) and the biggest win. If the
   pooled-entity API reads worse than the `v-for` it replaces, revise the composable before
   continuing rather than backfilling a design that isn't earning its keep.
5. **Rewrite `create-minigame-frontend` (and the CLAUDE.md PixiJS section) before backfilling the
   rest.** This matters more than it sounds: the skill currently hands an AI agent the hand-rolled
   `ResizeObserver` pattern as copy-paste code and explicitly says to match the existing
   (inconsistent) games — i.e. it teaches the exact anti-pattern this recommendation removes. Fix
   the false "v8" claim in `CLAUDE.md` at the same time. Every new minigame built after this step
   should be structurally unable to reintroduce the bugs in Finding 1.
6. Backfill the remaining games (`highwayHustle`, `teambasedPong`, `rpsBracket`, `businessBailout`,
   `crazyCounting`) opportunistically, one PR each.
7. Remove `vue3-pixi` from `package.json` once nothing imports it. Add an ESLint
   `no-restricted-imports` rule banning `vue3-pixi` entirely and banning `pixi.js` outside
   `src/composables/pixi/**`, so the boundary is mechanically enforced rather than a convention
   that drifts the way the current one has.

Two adjacent issues the new composable should absorb rather than leave as separate bugs:
`GameManager.vue:41` calls `container.scrollIntoView({ behavior: 'smooth' })` from inside its own
`ResizeObserver` callback — risky on mobile where an on-screen keyboard opening/closing fires
resize events — and `GameManager.vue`'s 500ms debounce before swapping minigame components means
`update()` is silently a no-op for the first ~12–15 incoming snapshots after a round starts, so
`useSnapshotBuffer`'s `initial` value must be treated as a real, expected first state rather than a
placeholder.

## Recommendation C — Close the AI-agent / type-safety gap

This project explicitly plans to lean on AI coding agents for a meaningful share of future
minigame development (there's already a `CLAUDE.md` and three dedicated skills for exactly this).
Research surfaced a specific, fixable reason that workflow is currently riskier than it should be:
the code that exists today, and the compiler configuration, don't actually enforce what the
documentation describes.

- **Flip `strict: false` to `strict: true` in `tsconfig.app.json`**, staged in two steps: first
  `strict: true` with `strictNullChecks: false` (cheap — mostly catches implicit-`any`), then
  `strictNullChecks: true` once the Pixi migration lands (do it after, not before, so the rewrite
  happens under whichever rule set is simpler, then the rest of the codebase catches up). This is
  currently an explicit override of the stricter default the project's own base config
  (`@vue/tsconfig`) sets — nothing about FlatBuffers interop actually requires it off; the
  generated code already targets strict mode cleanly. Payoff is concrete: FlatBuffer vector
  accessors return `T|null`, and at least one existing processor
  (`highwayHustle/HighwayHustleProcessor.ts`) calls them with no null guard while a sibling
  processor does guard correctly — exactly the class of bug (`Cannot read properties of null`,
  live, on the host TV, mid-round) `strictNullChecks` exists to catch before it ships.
- **Fix the untyped payload-read pattern.** The generated `gamestatepayload<T>(obj): any` accessor
  is called ~65 times across the codebase as
  `const payload: SomePayload = data.gamestatepayload(new SomePayload())` — every one of those
  type annotations is decorative; because the return type is `any`, mismatching the annotation and
  the constructor produces no error. A five-line hand-written helper
  (`readPayload<T>(data, Ctor)`, kept outside the generated-code directory) that returns the
  constructed type via inference closes all 65 sites at once and gives real autocomplete, for
  near-zero effort.
- **Add a compiler-enforced contract for the minigame `update()` method.** Today every minigame
  independently writes `defineExpose({ update })`, with no shared interface — drift is already
  visible (different function-declaration styles, some omitting the documented `return []`). A
  two-line addition per file (`defineExpose<MinigameView>({ update })` against a shared
  `MinigameView` interface) turns a missing/misnamed `update` into a compile error, and typing
  `GameManager.vue`'s `gameViewRef` against the same interface closes the loop end to end.
- **Add an actual CI gate.** Today, nothing runs automatically on a PR: the only GitHub Actions
  workflow with a test job is manually triggered and runs a backend test binary that (per
  `CLAUDE.md` itself) is never built; `type-check` only executes inside the Docker build at
  release time. This is also why documentation and reality have already drifted — `CLAUDE.md`
  states Prettier formatting is "exactly enforced," but there is no pre-commit hook installed and
  several files already violate the configured style. Add a PR-triggered job running
  `type-check`, `lint`, and `prettier --check`. This is the single change that turns every other
  convention in this plan from prose an agent can skip into a gate it must pass — and because the
  agent can run the same commands locally, it self-corrects before a human ever reviews the diff.
- **`CLAUDE.md` is the single, primary source of truth — remove the Copilot duplication outright.**
  `CLAUDE.md` and `.github/copilot-instructions.md` currently duplicate the same guidance (including
  the same false "PixiJS v8" claim). GitHub Copilot is no longer used on this project, so
  `.github/copilot-instructions.md` should simply be deleted rather than kept as a redirect stub —
  check `.github/skills/` for the same Copilot-era duplication of `.claude/skills/` content and
  remove that too if it's just a mirrored copy. Update the "reference minigame" guidance in the
  `create-minigame-frontend` skill at the same time as the Recommendation B rewrite: it currently
  points to `teambasedPong` as the primary reference despite it containing the dead deep-watcher
  bug, and lists `highwayHustle`/`marbleMania` as required/recommended reading despite them
  containing the worst offenders found in this research. Point instead at the new composable plus
  whichever minigame is migrated onto it first.
- **Add a cheap CI check for the backend/frontend name-coupling footgun.** The camelCase directory
  name under `frontend/src/components/` must exactly match the backend's `get_camel_case_name()`
  string, with no compiler link between the two languages — and this exact mismatch has already
  caused two renames in this project's git history (`buisinessBailout`→`businessBailout`,
  `shellGame`→`shellShuffle`). A short script comparing the two lists and failing CI on a mismatch
  turns a silent runtime failure into a loud PR-time one.

## Recommendation D — Dependency and build hygiene (routine, lower urgency)

Nothing here is urgent on its own; bundle it into normal maintenance rather than a dedicated push.

| Package | Current | Notes |
|---|---|---|
| Tailwind CSS | 3.4.17 | v4 exists (new "Oxide" engine) — faster builds, smaller output; worth the upgrade eventually, not a rewrite |
| Pinia | 2.3.1 | 3.0 exists (ESM-only rewrite) — minor footprint win, no urgency |
| ESLint | 8.57 (frozen legacy major) | v9's flat config is upstream; do alongside Recommendation C's rule-set changes |
| flatbuffers (frontend) | 24.12.23 | ~20 months stale relative to the backend's `flatc` pin (24.3.25) — benign today, worth keeping in sync so a future generator/runtime skew doesn't produce a silent bug |
| TypeScript | 5.4.5 | several minors behind; fine to bump alongside the `strict` migration in Recommendation C |

Also worth a quick cleanup pass, unrelated to any of the above: an unused `e` dependency, an
unused `vue-joystick-component` dependency (the actual joystick UI is a hand-forked local copy),
an undeclared `axios` import in `releasesStore.ts` that currently only resolves because a dev
tool happens to pull it in transitively, and the `test:e2e`/`test:unit` npm scripts, which
currently invoke `cypress` even though it isn't installed and isn't in use. Since testing
infrastructure is being deliberately deferred to a separate future decision, these dead scripts
should either be removed or left as an explicit placeholder rather than quietly broken. None of
these affect runtime behavior today, but each is a dependency-graph correctness issue that could
break silently on a future `npm install`.

## Recommendation E — Realtime-update robustness

Two issues surfaced that sit squarely in "the gameplay/realtime-update path," not the
one-off-asset category this plan otherwise excludes:

- **No WebSocket reconnection logic exists anywhere.** `confettiStore.ts` has no `onerror`
  handler, and the only recovery from a dropped connection is a hard `location.reload()` on
  `onclose`. For a game played entirely over one persistent socket per client, a momentary WiFi or
  cellular blip mid-minigame currently costs a player their session state rather than a
  transparent reconnect. Worth adding a minimal reconnect-with-backoff layer around the existing
  `host()`/`join()` calls — though note this was researched from the frontend side only: whether a
  reconnect can transparently resume a player's existing spot in an in-progress minigame, or can
  only rejoin fresh, depends on whether the backend's party/client handling supports
  re-associating a new socket with an already-known client. That's a joint frontend/backend
  question to scope properly when this phase is actually implemented, not assumed away now.
- **`GameManager.vue` leaks a `ResizeObserver` on every single round transition.** It sets one up
  in `onMounted` and returns a cleanup closure — a pattern that only works for `watchEffect`/
  `watch`, not `onMounted` — with no `onUnmounted` hook at all, while the component itself is
  conditionally mounted and unmounted every `PartyPrep → MiniGame → Leaderboard` cycle
  (`v-if="viewState === ViewState.MiniGame"` in both route files). This is best understood as a
  symptom of the same gap Recommendation B closes (no shared, disciplined lifecycle/composable
  pattern) rather than a one-off fix — but flag it explicitly so it isn't lost if `GameManager`
  isn't touched during that work.

---

## What was considered and explicitly rejected

The brief asked for an open-minded, full-spectrum look. Here's what was seriously evaluated and
why each was set aside, so the reasoning is on record rather than silently dropped:

**A full framework rewrite (React, Svelte/SvelteKit, SolidJS, vanilla TS).** Rejected with high
confidence. The measured bottlenecks (Finding 1 and 2) live entirely in imperative canvas code and
a mis-scoped app-shell component — none of them are attributable to Vue's own reactivity/diffing
overhead, and none would be fixed by changing frameworks. The bundle math doesn't support it
either: swapping Vue for the lightest realistic alternative saves on the order of 10–15KB gzip of
runtime weight, versus ~150–200KB gzip currently wasted on the duplicated PixiJS problem alone —
the framework is responsible for a small fraction of the actual waste found. Every realistic
alternative is neutral-to-worse than Vue 3 on the project's own "AI agent must be able to work in
it productively" requirement: React's ecosystem is larger but far more heterogeneous (an agent is
prone to emitting Next.js/RSC-flavored code into a plain Vite SPA) and its component model
encourages state-per-message patterns that fit this app's 20–30Hz push model poorly; Svelte 5's
runes migration currently produces the sharpest version-confusion in AI-generated Svelte code of
any mainstream framework; SolidJS's characteristic AI-agent failure mode (destructuring props,
which silently severs its fine-grained reactivity with no error) is exactly the kind of unreviewable
silent failure this project's owner is trying to avoid by requiring good documentation in the first
place. Vue 3's `<script setup>` Composition API is also, concretely, the only pattern used anywhere
in this 46-component codebase — a full, consistent in-repo example corpus for an agent to pattern-
match against, which is worth more than any cross-framework documentation-depth difference at this
project's scale. Separately, and decisively on its own: this project's frontend has averaged
roughly 2–3 commits/month over the last year (confirmed via `git log`), with several months of zero
frontend activity. An ~11,000-line rewrite is a multi-year undertaking at that pace, during which
no user-facing improvement would ship.

**A different rendering engine wholesale (Konva.js, WebGPU-first libraries, Phaser, Three.js,
plain Canvas2D).** Konva is a lateral move (similar capability, smaller ecosystem, not worth a
migration on its own). WebGPU-first directly contradicts the "support all mobile
browsers/devices" requirement — coverage across older iOS/Android is still too inconsistent, and
since canvas rendering is host-only today (Finding 3), it wouldn't even help the prioritized mobile
side. Phaser/Three.js are the wrong shape entirely — full game engines with their own physics/scene
ownership, when this app's physics is already correctly owned by the C++ backend; adopting one
would create a second, competing source of truth for game state. Plain Canvas2D was the one
genuinely close call (see Recommendation B) — decided against in favor of PixiJS v8's smaller
migration delta and headroom for future celebratory visual effects, not because Canvas2D is a bad
idea; it remains available per-minigame later behind the same composable boundary.

**Swapping component/CSS libraries (Naive UI → something else, Tailwind → Bootstrap).** Naive UI
is used in under half the codebase, concentrated in generic, easily-substitutable components
(`NCard`, `NScrollbar`, `NButton`) — not shown to be a performance bottleneck anywhere in this
research, so ripping it out wouldn't move the needle and isn't worth the churn. Bootstrap would be
a straightforward downgrade from Tailwind — heavier, more JS-dependent for interactive components,
and less tree-shakeable — there's no version of "switch to Bootstrap" that improves on the current
setup. Recommend keeping both, upgrading Tailwind to v4 opportunistically (Recommendation D).

---

## Suggested sequencing

| Phase | Work | Effort | Depends on |
|---|---|---|---|
| 0 | Recommendation A (CSS background, vendor chunking, naive-ui packaging fix, `isMobile()` reuse) | ~1–2 days | nothing — do first, makes everything else measurable |
| 0 | CI gate: type-check + lint + prettier on PRs (part of C) | hours | nothing — do early so later phases are guarded |
| 1 | Build the composable layer, manually verified (part of B) | ~2–3 days | Phase 0 |
| 2 | Migrate `shellShuffle`, then `marbleMania` as go/no-go gate (part of B) | ~3–4 days | Phase 1 |
| 3 | Rewrite `create-minigame-frontend` skill + `CLAUDE.md` PixiJS section (part of B/C) | ~1 day | Phase 2 |
| 4 | Backfill remaining 5 minigames, one PR each; remove `vue3-pixi`; add lint import ban | opportunistic | Phase 3 |
| — | Typed payload helper, `MinigameView` contract, staged `strict` rollout (rest of C) | ~3–5 days, can overlap with B | Phase 0 |
| — | WebSocket reconnect logic, `GameManager` cleanup fix (E) | ~1–2 days, can overlap | independent |
| — | Dependency version currency (D) | ongoing/opportunistic | independent |

## How to verify this plan's diagnosis and, later, its results

- **Before touching code:** run a production build and inspect it with the new bundle analyzer to
  confirm the current dual-PixiJS bundle composition and get a real baseline number (this plan's
  ~840KB/~262KB gzip figures were read from an existing local build, not a tool-verified one).
- **After Recommendation A:** rebuild, confirm via the analyzer that PixiJS no longer appears in
  any chunk loaded by the `/join` route; manually load the join flow on a throttled mobile network
  profile in Chrome DevTools and compare time-to-interactive before/after.
- **After each Recommendation B migration step:** use Chrome/Safari performance profiling on the
  host during actual gameplay to confirm GPU/render work drops and no longer runs while the game is
  idle (e.g. during the `shellShuffle` GUESS phase where nothing on screen changes).
- **Mobile device pass:** test the join/lobby flow and at least one full party on iOS Safari
  (including an iPad, since the current `isMobile()` gap specifically affects it), Android Chrome,
  and one older/lower-end device, before and after Recommendation A.
- **CI gate:** confirm it actually blocks by opening a throwaway PR with a deliberate type error
  and an unformatted file, and checking the check fails.

## Critical files

- `frontend/src/App.vue`, `frontend/src/components/BackgroundComponent.vue` — Recommendation A
- `frontend/vite.config.ts`, `frontend/package.json` — vendor chunking, `vue3-pixi` removal, dep
  hygiene (A, B, D)
- `frontend/src/components/marbleMania/HostView.vue`, `frontend/src/components/highwayHustle/HostView.vue`,
  `frontend/src/components/shellShuffle/HostView.vue`, `frontend/src/components/teambasedPong/HostView.vue`
  — the 4 minigames with the concrete bugs cited; `shellShuffle` and `marbleMania` are the two
  reference migrations (B)
- `frontend/src/components/GameManager.vue` — `MinigameView` typing, ResizeObserver leak (C, E)
- `frontend/src/stores/confettiStore.ts` — websocket reconnect logic (E)
- `frontend/tsconfig.app.json` — staged `strict` rollout (C)
- `frontend/src/flatbuffers/` (generated, read-only) plus a new hand-written `readPayload` helper
  alongside it — typed payload boundary (C)
- `.claude/skills/create-minigame-frontend/SKILL.md`, `CLAUDE.md`, `.github/copilot-instructions.md`
  — documentation consolidation and the false "v8" claim (C, and part of B's migration sequence)
- `.github/workflows/` — new PR-triggered CI job (C)
