# frontend

This template should help get you started developing with Vue 3 in Vite.

## Project Setup

```sh
npm install
```

### Compile and Hot-Reload for Development

```sh
npm dev
```

### Type-Check, Compile and Minify for Production

```sh
npm build
```

### Run Headed Component Tests with [Cypress Component Testing](https://on.cypress.io/component)

```sh
npm test:unit:dev # or `bun test:unit` for headless testing
```

### Run End-to-End Tests with [Cypress](https://www.cypress.io/)

```sh
npm test:e2e:dev
```

This runs the end-to-end tests against the Vite development server.
It is much faster than the production build.

But it's still recommended to test the production build with `test:e2e` before deploying (e.g. in CI environments):

```sh
npm build
npm test:e2e
```

### Lint with [ESLint](https://eslint.org/)

```sh
npm lint
```

## PixiJS composable playground (dev only)

A live reference and regression check for the shared PixiJS composables at
`src/composables/pixi/` (`useGameCanvas`, `createEntityLayer`, `useSnapshotBuffer`) — built while
verifying Phase 1 of `docs/frontend-infrastructure-plan.md`, kept as a standing dev tool.

**Reach it:** with the dev server running (`npm run dev`), open
`http://localhost:5173/dev/pixi-playground` (port may differ if 5173 is taken). The route only
exists in dev builds (`import.meta.env.DEV`-gated in `src/router/index.ts`) — it's never part of
a production bundle.

**What it shows:** a handful of synthetic colored balls on a dark canvas, plus controls and live
counters:

| Control / readout | What it does |
|---|---|
| `Add ball` / `Remove ball` | Adds/removes a synthetic entity via `createEntityLayer`. Watch **canvas elements in DOM** — it should always read `1`. If it ever changes, the `Application`/WebGL context got recreated, which is exactly the bug this composable layer exists to prevent. |
| `Start/Stop smooth interpolation` | Toggles `useGameCanvas`'s continuous `animate`/`stopAnimating` mode. **Off**: balls only jump to a new position on each simulated network tick (every 1.2s) — discrete steps, like a real minigame snapshot. **On**: positions interpolate smoothly toward the latest target every frame using `dt`, independent of display refresh rate. |
| `renders since last tick` | The most useful readout. With interpolation **off**, this should sit at `1` between ticks — proof the render loop is truly idle, not just visually static, while nothing has changed. It only climbs while interpolation is **on**. |
| `total renders` / `network ticks` | Running counters — useful for eyeballing rate (e.g. renders climbing by roughly your monitor's refresh rate while interpolating). |

**When to use it:**
- As a working example when wiring a new minigame's `HostView.vue` onto these composables (Phase
  2 onward of the infrastructure plan) — it's a complete, runnable usage of all three together.
- After changing anything in `src/composables/pixi/` — re-check all three behaviors above. A
  broken render loop won't show up in `type-check`/`lint`/`format`, only on an actually-running
  page — that's exactly how a real exponential-rendering bug was caught while building this (see
  the Phase 1 row in `docs/frontend-infrastructure-plan.md` for details).

**Source:** `src/routes/PixiPlaygroundRoute.vue`, route registered in `src/router/index.ts`.
