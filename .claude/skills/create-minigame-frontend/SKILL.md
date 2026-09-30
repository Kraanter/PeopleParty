---
name: create-minigame-frontend
description: "Implement responsive Vue 3 frontend components (HostView + PlayerView) for a new PeopleParty minigame. Use when: creating minigame frontend components, building host/player views, adding a minigame UI, or working on minigame Vue components."
---

# Create Minigame Frontend

Implement the Vue 3 + TypeScript frontend components for a new PeopleParty minigame — responsive
HostView (big screen) and PlayerView (mobile phone controller), with optional Models, Processor,
and SpriteMap files.

## When to Use

- User asks to create minigame frontend components
- User asks to build host or player views for a game
- User asks to add a minigame UI / Vue components
- User asks to wire a minigame into the frontend

## Procedure

### Step 1 — Gather Context

Before writing any code, read existing implementations to match established patterns. Read BOTH
`<script>` and `<template>` sections for each.

**Migration status, read this first**: `shellShuffle` and `marbleMania` have been migrated onto the
new `@/composables/pixi` layer (`useGameCanvas`/`createEntityLayer`/`useSnapshotBuffer`) and are the
primary references below for any new canvas-based minigame. `teambasedPong`, `highwayHustle`,
`rpsBracket`, `businessBailout`, and `crazyCounting` still use the older `vue3-pixi` library — real,
currently-shipping code, still worth reading for non-canvas concerns, but **not** the canvas-setup
pattern to copy (full backfill is tracked separately in `docs/frontend-infrastructure-plan.md`,
Phase 4). Build every new minigame on the composable layer regardless of how many existing
minigames still look like the old pattern.

**Required reading — reference games** (read ALL files in each directory):
- `frontend/src/components/shellShuffle/` — **primary reference for simple/single-entity
  canvases.** Migrated onto `@/composables/pixi` (`useGameCanvas` + `useSnapshotBuffer`, no
  `createEntityLayer`): one shared `Graphics` redrawn on every `invalidate()`, `worldSize`
  letterboxing, HTML overlay positioning synced to `canvas.scale`/`offsetX`/`offsetY` (the
  cup-number labels), `defineExpose({ push })` on the canvas-owning child with a template-ref call
  from `HostView.vue` (fine here — one always-present child, no mount-timing race)
- `frontend/src/components/marbleMania/` — **primary reference for pooled/variable-count
  entities.** Also migrated onto `@/composables/pixi`: three `createEntityLayer` pools across
  z-ordered sub-`Container`s, `canvas.startAnimating()`/`stopAnimating()` for camera easing, and a
  prop + `watch(..., { immediate: true })` pattern (not a template-ref push) for getting live host
  data into the canvas-owning child — the pattern to default to for new minigames, see the
  Responsive Design section below
- `frontend/src/components/highwayHustle/` — **old pattern, not yet migrated** (still
  `vue3-pixi`/`<Application>`/`<Graphics @render>`). Still worth reading for its sprite-map
  approach (`Application` + `Graphics` + `Sprite` + `Text`) and joystick input — don't copy its
  canvas setup or `ResizeObserver` handling into a new minigame
- `frontend/src/components/memoryMixer/` — Grid-based layout (non-PixiJS), GridView subcomponent,
  card flipping, `NCard`/`NScrollbar` from Naive UI, `sendPlayerAction` inline pattern
- `frontend/src/components/crazyCounting/` — **old pattern, not yet migrated.** PixiJS with entity
  rendering, `appSize = Math.min(width, height)` canvas sizing — don't copy the canvas setup; see
  the Responsive Design section below for the new-pattern equivalent (the `size:` sizing mode)

**Recommended reading — additional patterns** (scan files):
- `frontend/src/components/teambasedPong/` — **old pattern, not yet migrated** (still
  `vue3-pixi`). Its multi-phase `ViewState` (RoundPrep, MiniGame, RoundResult, Results) and
  joystick input are still good references; ignore its `canvasWidth`/`canvasHeight`/`MAP_ASPECT`/
  `ResizeObserver` sizing code and its `watch(payloadData, () => {}, { deep: true })` — the latter
  is a known dead no-op left over from fighting `vue3-pixi`'s implicit redraw trigger, not a
  pattern to learn from
- `frontend/src/components/businessBailout/` — **old pattern, not yet migrated** (`vue3-pixi`).
  PixiJS for graphs, parser pattern, multiple subcomponents — don't copy its canvas setup
- `frontend/src/components/rpsBracket/` — **old pattern, not yet migrated** (`vue3-pixi`). PixiJS
  for bracket visualization — don't copy its canvas setup
- `frontend/src/components/unscrambled/` — text-based, Models + Processor pattern (unaffected by
  the PixiJS changes)
- `frontend/src/components/rightOnTime/` — FlipClock subcomponent, Models + Processor, custom
  input sending (unaffected by the PixiJS changes)
- `frontend/src/components/launchParty/` — LightsComponent subcomponent, simple button
  interaction, `<style scoped>` for CSS animations (one of several components that do this — see
  the styling note below; unaffected by the PixiJS changes)

**Required reading — infrastructure**:
- `frontend/src/components/GameManager.vue` — dynamic component loading by `gameName`,
  `width`/`height` from `ResizeObserver`, prop passing, `update()` call forwarding
- `frontend/src/components/introduction/Introduction.vue` — shared Introduction component (props:
  `logoSVG`, `data: IntroductionData`)
- `frontend/src/components/shared/JoystickComponent.vue` — shared joystick (if game uses joystick)
- `frontend/src/components/TimeComponent.vue` — shared time display (prop: `timeLeft`)
- `frontend/src/util/flatbufferMessageBuilder.ts` — `buildMessage(builder, payload, messageType,
  payloadType)` utility
- `frontend/src/util/joystickMessageBuilder.ts` — `sendPlayerAction(gameName, x, y)` and
  `sendPlayerEvent(gameName, event)`
- `frontend/src/stores/confettiStore.ts` — WebSocket store: `sendMessage()`, `clientName`,
  `isHosting`
- `frontend/src/assets/main.css` — global styles, CSS variables, color tokens, `.text-body`/
  `.text-title` responsive classes

### Step 2 — Confirm Prerequisites

Before creating frontend files, verify:
1. FlatBuffer schemas exist in `schemes/minigamedata/{GameName}/` — if not, create them first
   (use the `create-minigame-schemas` skill)
2. Schemas have been built (`npm run build:macos` or platform variant) so that
   `frontend/src/flatbuffers/` contains the generated TypeScript files
3. Read `frontend/src/flatbuffers/messageClass.ts` to find the exact `GameStateType` enum values
   for the new game's payloads
4. Know the `get_camel_case_name()` value from the backend — the frontend directory name MUST
   match this exactly

### Step 3 — Create Directory and Files

**Directory**: `frontend/src/components/{gameName}/` — camelCase, MUST match
`get_camel_case_name()` from backend

**Required files**:

| File | Purpose |
|------|---------|
| `HostView.vue` | Big screen display — rendered on host device |
| `PlayerView.vue` | Mobile phone controller — rendered on player devices |

**Optional files** (based on game complexity):

| File | When to add |
|------|-------------|
| `{GameName}Models.ts` | Game has >2 data shapes or complex nested data |
| `{GameName}Processor.ts` | Parsing logic is non-trivial (vectors, nested tables, many fields) |
| `{GameName}SpriteMap.ts` | Game uses sprite assets with lookup functions |
| Additional `.vue` components | Reusable sub-UI elements (grids, clocks, cards, etc.) |

### Step 4 — Implement Models (if needed)

Define TypeScript interfaces for all data shapes:

```ts
// {GameName}Models.ts

export interface {GameName}HostData {
  time_left: number
  // ... game-specific host fields
}

export interface {GameName}PlayerData {
  // ... per-player state fields
}

export interface {GameName}ResultPair {
  name: string
  placement: number
}

export interface {GameName}Result {
  results: {GameName}ResultPair[]
}
```

Rules:
- Use `interface` for object shapes, not `type`
- `number` for all numeric fields (FlatBuffer bigints wrapped with `Number()`)
- `string` for all text fields (FlatBuffer strings decoded with `decodeURI()`)
- Enum values: define a TypeScript `enum` mirroring the FlatBuffer enum

### Step 5 — Implement Processor (if needed)

Parse functions that convert `MiniGamePayloadType` into typed model data:

```ts
// {GameName}Processor.ts
import type { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { {GameName}HostPayload } from '@/flatbuffers/{generated-file}'
import type { {GameName}HostData } from './{GameName}Models'

export function parse{GameName}HostPayload(data: MiniGamePayloadType): {GameName}HostData {
  const payload: {GameName}HostPayload = data.gamestatepayload(
    new {GameName}HostPayload()
  )

  return {
    time_left: payload.timeLeft()
    // ... map fields
  }
}
```

Rules:
- `Number()` wrap ALL FlatBuffer `bigint` fields (timestamps, large counters)
- `decodeURI()` on ALL FlatBuffer string fields (player names are URI-encoded on backend)
- Iterate vectors with `for (let i = 0; i < payload.xxxLength(); i++)` pattern
- Null-check vector elements: `const item = payload.xxx(i); if (item) { ... }`
- Import generated payload classes from `@/flatbuffers/messageClass` (barrel) or from individual
  generated files (both patterns exist in codebase)

### Step 6 — Implement HostView (Big Screen)

This is the **most critical component for visual quality**. The host view displays on screens
ranging from 13" laptops to 65" TVs.

#### Component Structure

```vue
<script lang="ts" setup>
import { ref, computed } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'

const props = defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const payloadData = ref<{GameName}HostData>({
  // ... initial values
})

const results = ref<{GameName}Result>({
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.{GameName}Host: {
      viewState.value = ViewState.MiniGame
      payloadData.value = parse{GameName}HostPayload(data)
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.{GameName}Result: {
      viewState.value = ViewState.Results
      results.value = parse{GameName}Result(data)
      break
    }
  }
  return []
}

defineExpose({
  update
})
</script>
```

#### Responsive Design — PixiJS Games

For a new minigame, all canvas rendering goes through the shared composables in
`@/composables/pixi` (`useGameCanvas`, `createEntityLayer`, `useSnapshotBuffer`) — never
`vue3-pixi`, and never a hand-rolled `ResizeObserver`. There is no `<Application>` or `<Graphics
@render>` in this pattern at all; the template is just a plain mount point:

```vue
<template>
  <div ref="containerRef" class="bg-black rounded-lg overflow-hidden flex-1" />
</template>
```

**Simple tier — one shared `Graphics`, redrawn on every update** (see `shellShuffle`):

```ts
import { ref, computed } from 'vue'
import { Graphics } from 'pixi.js'
import { useGameCanvas, useSnapshotBuffer } from '@/composables/pixi'

const containerRef = ref<HTMLElement | null>(null)
const gfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: GAME_MAP_WIDTH, height: GAME_MAP_HEIGHT }, // world units, not pixels
  backgroundColor: 0x1a7a3c,
  backgroundAlpha: 1,
  render: drawScene
})
canvas.root.addChild(gfx)

const buffer = useSnapshotBuffer<{GameName}HostData, {GameName}Hud>(initialSnapshot, {
  invalidate: canvas.invalidate,
  toHud: (d) => ({ /* the handful of fields a DOM overlay needs */ })
})
// toHud(initial) runs synchronously before this line returns — safe non-null assertion.
const hud = computed(() => buffer.hud.value!)

function drawScene() {
  const data = buffer.current
  gfx.clear()
  gfx.rect(0, 0, GAME_MAP_WIDTH, GAME_MAP_HEIGHT).fill(0x1a7a3c)
  // ... draw everything in world units; canvas.root's transform already carries the letterbox scale
}

function push(data: {GameName}HostData) {
  buffer.push(data) // calls canvas.invalidate() internally — no separate invalidate() call needed
}

defineExpose({ push })
```

**Complex tier — pooled, variable-count entities** (see `marbleMania`): use `createEntityLayer`
instead of a `v-for`/`:key`-driven Application recreation. `parent` is usually `canvas.root`, or a
`Container` added under it when you need explicit z-ordering across multiple entity kinds:

```ts
import { Graphics, Container } from 'pixi.js'
import { createEntityLayer } from '@/composables/pixi'

const marblesContainer = new Container()
canvas.root.addChild(marblesContainer) // added in the order you want it to draw relative to siblings

const marbleLayer = createEntityLayer<MarbleEntity, Graphics>(marblesContainer, {
  key: (entity) => entity.id,
  create: () => new Graphics(),
  update: (display, entity) => {
    display.clear()
    display.circle(0, 0, entity.radius).fill(entity.color)
    display.position.set(entity.x, entity.y)
  }
})

function drawScene() {
  marbleLayer.sync(buffer.current.marbles) // full-snapshot diff; missing entities go back to the pool
}
```

Duplicate keys passed to one `sync()` call are dropped (with a `console.warn`), not merged — make
sure entity ids are unique before calling it. `clear()` releases everything to the pool (e.g. on a
phase reset) without destroying; `destroy()` is the final teardown. z-order isn't stable across
recycling by default (reused/new displays always append last) — fine for flat, non-interactive,
≤16-entity games; if a future game needs stable stacking, set `parent.sortableChildren = true` and
give each display object a stable `.zIndex`.

**Getting live data from `HostView.vue` into the canvas-owning child** — default to a prop +
`watch(..., { immediate: true })` in the child, not a template-ref `push()` call from the parent:

```ts
// HostView.vue
const hostData = shallowRef<{GameName}HostData | null>(null)
const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.{GameName}Host: {
      viewState.value = ViewState.MiniGame
      hostData.value = parse{GameName}HostPayload(data)
      break
    }
    // ...
  }
  return []
}
```

```vue
<{GameName}GameView :host-data="hostData" />
```

```ts
// {GameName}GameView.vue
const props = defineProps<{ hostData: {GameName}HostData | null }>()
watch(
  () => props.hostData,
  (data) => { if (data) buffer.push(data) },
  { immediate: true }
)
```

**Why**: `update()` typically sets `hostData.value` and flips `viewState.value` in the same
synchronous tick. A template ref into the child isn't populated yet at that point (Vue defers ref
population until after the DOM update), so a `gameViewRef.value?.push(...)` call silently drops
the very first message — a real bug this exact fix shipped for in `marbleMania`. A prop watched
with `{ immediate: true }` doesn't have that race. `shellShuffle`'s simpler template-ref `push()`
call is still valid — it's a single, always-present child with no such race — but don't reach for
it reflexively in new code; default to prop + watch unless you're confident there's no race.

**Sizing modes** — `useGameCanvas` supports exactly two, pick one (mutually exclusive):
- **`worldSize: { width, height }`** (used by both `shellShuffle` and `marbleMania`) — letterboxes
  to that aspect ratio via the container element's own `ResizeObserver`, computing
  `canvas.scale`/`canvas.offsetX`/`canvas.offsetY` for you. Draw in world units inside `render`.
- **`size: () => ({ width, height })`** — "trust the parent" mode (e.g. wiring straight to
  `GameManager`'s own `width`/`height` props) instead of a second, redundant `ResizeObserver`. No
  letterboxing math runs (`scale` stays `1`). This is the direct replacement for the old
  `appSize = Math.min(props.width, props.height)` square-canvas pattern — compute whatever shape
  you need once inside the `size()` callback instead of hand-rolling it.

Neither migrated minigame currently uses `size:` (both use `worldSize`) — it's a fully supported
option, just not yet exercised by a shipped example.

**HTML overlays aligned with the canvas** (e.g. shellShuffle's cup-number labels) — read
`canvas.scale.value`, `canvas.offsetX.value`, `canvas.offsetY.value`. These are `ComputedRef`s and
are **not** auto-unwrapped in the template, because `canvas` itself isn't a top-level `ref` —
always write `canvas.scale.value`, never `canvas.scale`.

**Continuous animation** — for frame-rate-independent interpolation that must run every frame
regardless of network cadence (camera easing — see `marbleMania`'s `animateCamera`), call
`canvas.startAnimating((ctx) => { /* ctx.dt is ms since last frame */ })`. This is opt-in on top of
the default invalidate-on-push model. **It is never auto-stopped** — call `canvas.stopAnimating()`
yourself once the interpolation has settled (e.g. on a phase change), or an idle scene keeps
rendering every frame forever for no reason.

**Static one-shot canvas** — a third, simpler tier for a canvas built once and never updated again
(see `MarbleManiaResultsView.vue`): call `useGameCanvas` with a no-op `render: () => {}`, build the
scene once by adding display objects to `canvas.root`, then call `canvas.invalidate()` once. No
`useSnapshotBuffer`/`createEntityLayer` needed.

**Mental test**: "Would this look correct on a 13-inch laptop AND a 65-inch TV?"

#### Responsive Design — Non-PixiJS Games

For games using HTML/Tailwind (no canvas):

- Use relative sizing: `w-full`, `h-full`, `flex`, `grid`, `max-w-*`
- Use responsive text: `text-2xl sm:text-3xl md:text-4xl lg:text-5xl` or the global
  `.text-title`/`.text-body` classes from main.css
- Use `clamp()` via inline `style` when Tailwind can't express dynamic font sizing
- For grid layouts: use CSS grid with `auto-fit` / `auto-fill` and `minmax()`
- **Naive UI is available and expected** for structured UI — use `NCard` for card styling,
  `NScrollbar` for scrollable lists (see `memoryMixer`, `crazyCounting`, `unscrambled`,
  `rightOnTime` for real examples). Prefer it over hand-rolled equivalents

#### `<style>` blocks

Global convention is Tailwind-only, but `<style scoped>` is a real, accepted exception for
keyframe/CSS animations that Tailwind can't express — it's used in roughly a dozen components
across the codebase, including `launchParty`, `rightOnTime`, `highwayHustle`, `marbleMania`, and
even `teambasedPong` itself. Use it the same way: scoped, animation-only, not for general layout
or color styling (that stays in Tailwind classes).

#### Template Structure

The template MUST switch on `viewState`:

```vue
<template>
  <template v-if="viewState == ViewState.Introduction">
    <Introduction logoSVG="/assets/games/{gameName}/{gameName}Logo.svg" :data="intro" />
  </template>

  <template v-else-if="viewState == ViewState.MiniGame">
    <!-- Game content here -->
  </template>

  <template v-else-if="viewState == ViewState.Results">
    <!-- Results display here -->
  </template>
</template>
```

For multi-round games, add `RoundPrep` and `RoundResult` states between `Introduction` and
`MiniGame`/`Results`.

### Step 7 — Implement PlayerView (Mobile Phone)

The player view is rendered on mobile phones (320–430px wide). Design mobile-first.

#### Component Structure

```vue
<script lang="ts" setup>
import { ref, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'

const websocketStore = useWebSocketStore()

const props = defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const payloadData = ref<{GameName}PlayerData>({
  // ... initial values
})

const results = ref<{GameName}Result>({
  results: []
})

const personalResult = computed(() => {
  return results.value.results.find(
    (result) => result.name === websocketStore.clientName
  )
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.{GameName}Player: {
      viewState.value = ViewState.MiniGame
      payloadData.value = parse{GameName}PlayerPayload(data)
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.{GameName}Result: {
      viewState.value = ViewState.Results
      results.value = parse{GameName}Result(data)
      break
    }
  }
  return []
}

defineExpose({
  update
})
</script>
```

#### Sending Player Input

**For joystick-based games** — use the shared joystick utilities:

```ts
import JoystickComponent from '../shared/JoystickComponent.vue'
import { sendPlayerAction, sendPlayerEvent } from '@/util/joystickMessageBuilder'

const move = ({ x, y }: any) => {
  sendPlayerAction('{gameName}', x, y)
}

const stop = () => {
  sendPlayerAction('{gameName}', 0, 0)
}
```

Template:
```vue
<JoystickComponent
  :disabled="false"
  class="no-project-style"
  :size="200"
  base-color="lightgray"
  stick-color="black"
  :throttle="100"
  @move="move"
  @stop="stop"
/>
```

**For button/tap-based games** — build and send the FlatBuffer message inline:

```ts
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import {
  GameStatePayload,
  GameStateType,
  MessageType,
  MiniGamePayloadType,
  Payload,
  {GameName}PlayerInputPayload
} from '@/flatbuffers/messageClass'

const sendPlayerInput = (/* action params */) => {
  const builder = new flatbuffers.Builder()

  const playerInput = {GameName}PlayerInputPayload.create{GameName}PlayerInputPayload(
    builder, /* fields */
  )

  const miniGame = builder.createString('{gameName}')

  const miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.{GameName}PlayerInput,
    GameStatePayload.{GameName}PlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}
```

**IMPORTANT**: The game name string passed to `builder.createString()` MUST match
`get_camel_case_name()` from the backend.

#### Mobile Responsive Design Rules

- **Buttons**: minimum 44x44px touch target (use `p-4` or larger, `text-2xl`+)
- **Layout**: use `flex flex-col` to stack elements vertically; avoid horizontal scrolling
- **Viewport**: the app uses `calc(var(--vh, 1vh) * 100)` for iOS Safari — use `h-full` /
  `h-screen`
- **Text**: minimum `text-lg` for body text, `text-2xl`+ for important info, `text-4xl`+ for
  prominent results
- **Joystick**: center with flex, give ample margin (`m-12` or `style="margin: 50px"`)
- **Results**: show personal result prominently (large placement number), with scrollable list
  via `NScrollbar`
- **Introduction**: use `TimeComponent` and description text, centered with
  `flex flex-col justify-center items-center`

#### Player Introduction Template Pattern

```vue
<template v-if="viewState == ViewState.Introduction">
  <div class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center">
    <div class="w-full flex justify-center px-8">
      <div>
        <TimeComponent :timeLeft="intro.time_left" />
      </div>
    </div>
    <div>
      <div class="w-full h-full mt-16">
        <p class="text-4xl text-white">{{ intro.description }}</p>
      </div>
    </div>
  </div>
</template>
```

#### Player Results Template Pattern

```vue
<template v-else-if="viewState == ViewState.Results">
  <div
    v-if="personalResult"
    class="flex flex-col gap-4 w-full h-full justify-center items-center p-4"
  >
    <div class="text-5xl text-white font-bold mb-4">You placed:</div>
    <div class="text-8xl text-white font-bold mb-6">
      {{ formatOrdinals(personalResult.placement) }}
    </div>
  </div>
</template>
```

#### Ordinal Formatting Helper

Include this in both HostView and PlayerView when displaying placements:

```ts
const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n: number) => {
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
}
```

### Step 8 — Wiring

GameManager.vue dynamically imports components by `gameName` — **no manual case statement or
switch needed**. The import path is:

```ts
`./${name}/${componentName}View.vue`
```

Where `name` is the `minigame()` string from `MiniGamePayloadType` and `componentName` is `'Host'`
or `'Player'`. Confirmed by reading `GameManager.vue` directly — it uses
`defineAsyncComponent(() => import(...))` with this template string and nothing else; there is no
switch/case anywhere in the file.

This means:
- The directory name **MUST** be camelCase matching `get_camel_case_name()` from the backend
- The files **MUST** be named exactly `HostView.vue` and `PlayerView.vue`
- No registration, no import, no edit to `GameManager.vue` — it's automatic via
  `defineAsyncComponent` and dynamic `import()`

### Step 9 — Verify

After implementation, verify:

1. **Directory name** matches `get_camel_case_name()` from backend exactly (camelCase)
2. **Files** are named `HostView.vue` and `PlayerView.vue` exactly
3. **`defineExpose({ update })`** is present in both components
4. **`GameStateType` enum values** match what's in `frontend/src/flatbuffers/messageClass.ts`
5. **No unscoped `<style>` blocks** — `<style scoped>` is fine for animations, avoid it for
   general layout/color
6. **No semicolons**, single quotes, 2-space indentation
7. **All FlatBuffer bigint fields** wrapped with `Number()`
8. **All FlatBuffer string fields** decoded with `decodeURI()`
9. **Game name string** in message builders matches backend `get_camel_case_name()`
10. **Host view scales properly** — check that canvas/layout uses scale factor or relative sizing,
    no hardcoded pixel values
11. **Player view is mobile-friendly** — check touch targets, vertical layout, no horizontal
    scroll

## Quick Reference — Code Style

| Element | Convention | Example |
|---------|-----------|---------|
| Indentation | 2 spaces (no tabs) | — |
| Semicolons | None | — |
| Quotes | Single | `'hello'` |
| Line width | 100 characters | — |
| Trailing commas | None | — |
| Component files | PascalCase | `HostView.vue`, `PlayerView.vue` |
| Minigame directories | camelCase | `teambasedPong/`, `shellShuffle/` |
| Model files | `{GameName}Models.ts` | `TeambasedPongModels.ts` |
| Processor files | `{GameName}Processor.ts` | `TeambasedPongProcessor.ts` |
| SpriteMap files | `{GameName}SpriteMap.ts` | `HighwayHustleSpriteMap.ts` |
| Interfaces | PascalCase | `TeambasedPongHostData` |
| Variables/functions | camelCase | `payloadData`, `sendPlayerAction` |
| Store composables | `use` + PascalCase + `Store` | `useWebSocketStore` |
| Script setup | Always `<script lang="ts" setup>` | — |
| Section order | `<script>` → `<template>` | `<style scoped>` allowed for animation only |
| Props | `defineProps<{ width: number; height: number }>()` | — |
| Expose | `defineExpose({ update })` | — |

## Common Pitfalls

- **Directory name mismatch** — must exactly match `get_camel_case_name()` from backend;
  `teambasedPong` not `TeambasedPong` or `teambased_pong`
- **Forgetting `defineExpose({ update })`** — GameManager calls `gameViewRef.value?.update(data)`,
  won't work without expose
- **Hardcoded pixel values in HostView** — breaks on different screen sizes; draw in world units
  and let `useGameCanvas`'s `worldSize` letterboxing (or its `size:` callback) handle scaling —
  never multiply coordinates by a hand-rolled scale factor
- **Not wrapping FlatBuffer bigints with `Number()`** — causes TypeScript type errors or NaN
  rendering
- **Not decoding FlatBuffer strings with `decodeURI()`** — player names with special characters
  display as encoded
- **Using unscoped `<style>` blocks for general styling** — only Tailwind utility classes for
  layout/color; `<style scoped>` is fine, but only for CSS animations
- **Semicolons or double quotes** — Prettier config forbids both
- **Wrong GameStateType enum** — always read `messageClass.ts` after schema build; never guess
  values
- **Importing from individual generated files instead of barrel** — prefer
  `@/flatbuffers/messageClass` for types used in `update()` switch; individual imports are
  acceptable in Processor files
- **Not showing personal result on PlayerView results** — always compute `personalResult` from
  `websocketStore.clientName` and display it prominently
- **Missing `return []`** — the `update()` function should return `[]` at the end (convention
  from existing games)
- **Hand-rolling a `ResizeObserver` for canvas sizing** — `useGameCanvas`'s `worldSize`/`size`
  modes already handle this; a second, independent `ResizeObserver` (or a manually computed
  `Math.min(width, height)`) duplicates work it already does and is a sign the composable isn't
  being used
- **Importing from `vue3-pixi`, or writing an `<Application>`/`<Graphics @render>` template block,
  in a NEW minigame** — that library and pattern are legacy (5 not-yet-migrated minigames still use
  them; full removal is a separate future phase, see `docs/frontend-infrastructure-plan.md`). New
  canvas code goes through `@/composables/pixi` only: a plain `<div ref="containerRef">` mount
  point plus raw PixiJS display objects added imperatively to `canvas.root`
- **Forgetting `canvas.stopAnimating()`** after a `startAnimating()`-driven camera ease or similar
  — it is never auto-stopped, so an idle scene will keep rendering every frame forever
- **Duplicate keys in one `createEntityLayer.sync()` call** — silently dropped with a
  `console.warn`, not merged; make sure your entity list has unique keys before calling `sync()`
- **Forgetting Naive UI exists** — for non-canvas result/settings screens, check whether `NCard`/
  `NScrollbar`/etc. already covers what you're about to hand-roll in Tailwind
