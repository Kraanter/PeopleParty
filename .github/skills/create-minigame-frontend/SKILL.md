---
name: create-minigame-frontend
description: "Implement responsive Vue 3 frontend components (HostView + PlayerView) for a new PeopleParty minigame. Use when: creating minigame frontend components, building host/player views, adding a minigame UI, or working on minigame Vue components."
---

# Create Minigame Frontend

Implement the Vue 3 + TypeScript frontend components for a new PeopleParty minigame — responsive HostView (big screen) and PlayerView (mobile phone controller), with optional Models, Processor, and SpriteMap files.

## When to Use

- User asks to create minigame frontend components
- User asks to build host or player views for a game
- User asks to add a minigame UI / Vue components
- User asks to wire a minigame into the frontend

## Procedure

### Step 1 — Gather Context

Before writing any code, read existing implementations to match established patterns. Read BOTH `<script>` and `<template>` sections for each.

**Required reading — reference games** (read ALL files in each directory):
- `frontend/src/components/teambasedPong/` — **NEWEST, best reference**: aspect-ratio-locked canvas via computed `canvasWidth`/`canvasHeight` with `MAP_ASPECT`, responsive team display, scale factor from map dimensions, joystick input, multi-phase ViewState (RoundPrep, MiniGame, RoundResult, Results)
- `frontend/src/components/highwayHustle/` — PixiJS with sprite maps, joystick input, entity rendering, `Application` + `Graphics` + `Sprite` + `Text` from `pixi.js` / `vue3-pixi`
- `frontend/src/components/memoryMixer/` — Grid-based layout (non-PixiJS), GridView subcomponent, card flipping, NCard/NScrollbar from naive-ui, `sendPlayerAction` inline pattern
- `frontend/src/components/crazyCounting/` — PixiJS with entity rendering, `appSize = Math.min(width, height)` canvas sizing pattern

**Recommended reading — additional patterns** (scan files):
- `frontend/src/components/marbleMania/` — PixiJS with camera system, dynamic container sizing
- `frontend/src/components/businessBailout/` — PixiJS for graphs, parser pattern, multiple subcomponents
- `frontend/src/components/rpsBracket/` — PixiJS for bracket visualization
- `frontend/src/components/unscrambled/` — text-based, Models + Processor pattern
- `frontend/src/components/rightOnTime/` — FlipClock subcomponent, Models + Processor, custom input sending
- `frontend/src/components/launchParty/` — LightsComponent subcomponent, simple button interaction, `<style scoped>` exception for CSS animations

**Required reading — infrastructure**:
- `frontend/src/components/GameManager.vue` — dynamic component loading by `gameName`, `width`/`height` from `ResizeObserver`, prop passing, `update()` call forwarding
- `frontend/src/components/introduction/Introduction.vue` — shared Introduction component (props: `logoSVG`, `data: IntroductionData`)
- `frontend/src/components/shared/JoystickComponent.vue` — shared joystick (if game uses joystick)
- `frontend/src/components/TimeComponent.vue` — shared time display (prop: `timeLeft`)
- `frontend/src/util/flatbufferMessageBuilder.ts` — `buildMessage(builder, payload, messageType, payloadType)` utility
- `frontend/src/util/joystickMessageBuilder.ts` — `sendPlayerAction(gameName, x, y)` and `sendPlayerEvent(gameName, event)`
- `frontend/src/stores/confettiStore.ts` — WebSocket store: `sendMessage()`, `clientName`, `isHosting`
- `frontend/src/assets/main.css` — global styles, CSS variables, color tokens, `.text-body`/`.text-title` responsive classes

### Step 2 — Confirm Prerequisites

Before creating frontend files, verify:
1. FlatBuffer schemas exist in `schemes/minigamedata/{GameName}/` — if not, create them first (use the `create-minigame-schemas` skill)
2. Schemas have been built (`npm run build:macos` or platform variant) so that `frontend/src/flatbuffers/` contains the generated TypeScript files
3. Read `frontend/src/flatbuffers/messageClass.ts` to find the exact `GameStateType` enum values for the new game's payloads
4. Know the `get_camel_case_name()` value from the backend — the frontend directory name MUST match this exactly

### Step 3 — Create Directory and Files

**Directory**: `frontend/src/components/{gameName}/` — camelCase, MUST match `get_camel_case_name()` from backend

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

Define TypeScript interfaces for all data shapes. Follow the naming pattern from existing games:

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
- Import generated payload classes from `@/flatbuffers/messageClass` (barrel) or from individual generated files (both patterns exist in codebase)

### Step 6 — Implement HostView (Big Screen)

This is the **most critical component for visual quality**. The host view displays on screens ranging from 13" laptops to 65" TVs.

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

// Introduction data
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

// Host payload data
const payloadData = ref<{GameName}HostData>({
  // ... initial values
})

// Results data
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

For games rendering a canvas with PixiJS, ALWAYS use the aspect-ratio-locked pattern from teambasedPong:

```ts
// Canvas sizing — dynamically tracks container width and maintains aspect ratio
const gameCanvasRef = ref<HTMLElement | null>(null)
const canvasWidth = ref(780) // default fallback
const MAP_ASPECT = 16 / 9   // or whatever your game world ratio is (e.g., 800/600)
const canvasHeight = computed(() => Math.round(canvasWidth.value / MAP_ASPECT))
const scale = computed(() => canvasWidth.value / GAME_WORLD_WIDTH)

// ResizeObserver to track container width changes
let resizeObserver: ResizeObserver | null = null
watch(gameCanvasRef, (el) => {
  resizeObserver?.disconnect()
  if (el) {
    canvasWidth.value = el.clientWidth
    resizeObserver = new ResizeObserver((entries) => {
      canvasWidth.value = entries[0].contentRect.width
    })
    resizeObserver.observe(el)
  }
})
onUnmounted(() => {
  resizeObserver?.disconnect()
})
```

In the template, bind the canvas container with `ref="gameCanvasRef"` and use the `Application` component:

```vue
<div ref="gameCanvasRef" class="bg-black rounded-lg overflow-hidden flex-1">
  <Application
    :width="canvasWidth"
    :height="canvasHeight"
    :backgroundAlpha="1"
    :backgroundColor="0x000000"
  >
    <Graphics @render="renderGame" />
  </Application>
</div>
```

In all render functions, **ALWAYS use the scale factor** — never hardcode pixel values:

```ts
const renderGame = (graphics: Graphics) => {
  graphics.clear()
  const s = scale.value

  // All positions multiplied by scale
  graphics.beginFill(0xffffff)
  graphics.drawCircle(entity.x * s, entity.y * s, radius * s)
  graphics.endFill()
}
```

**Mental test**: "Would this look correct on a 13-inch laptop AND a 65-inch TV?"

#### Responsive Design — Non-PixiJS Games

For games using HTML/Tailwind (no canvas):

- Use relative sizing: `w-full`, `h-full`, `flex`, `grid`, `max-w-*`
- Use responsive text: `text-2xl sm:text-3xl md:text-4xl lg:text-5xl` or the global `.text-title`/`.text-body` classes from main.css
- Use `clamp()` via inline `style` when Tailwind can't express dynamic font sizing
- For grid layouts: use CSS grid with `auto-fit` / `auto-fill` and `minmax()`
- Result tables: use `NCard` from naive-ui for card styling, `NScrollbar` for scrollable lists

#### Alternative Canvas Sizing Pattern

For games where the canvas should be a square fitted to the smallest dimension (like crazyCounting):

```ts
const appSize = computed(() => Math.min(props.width, props.height))
```

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

For multi-round games, add `RoundPrep` and `RoundResult` states between `Introduction` and `MiniGame`/`Results`.

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

// Introduction data
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

// Player-specific payload data
const payloadData = ref<{GameName}PlayerData>({
  // ... initial values
})

// Results
const results = ref<{GameName}Result>({
  results: []
})

// Personal result lookup
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

**IMPORTANT**: The game name string passed to `builder.createString()` MUST match `get_camel_case_name()` from the backend.

#### Mobile Responsive Design Rules

- **Buttons**: minimum 44x44px touch target (use `p-4` or larger, `text-2xl`+)
- **Layout**: use `flex flex-col` to stack elements vertically; avoid horizontal scrolling
- **Viewport**: the app uses `calc(var(--vh, 1vh) * 100)` for iOS Safari — use `h-full` / `h-screen`
- **Text**: minimum `text-lg` for body text, `text-2xl`+ for important info, `text-4xl`+ for prominent results
- **Joystick**: center with flex, give ample margin (`m-12` or `style="margin: 50px"`)
- **Results**: show personal result prominently (large placement number), with scrollable list via `NScrollbar`
- **Introduction**: use `TimeComponent` and description text, centered with `flex flex-col justify-center items-center`

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

GameManager.vue dynamically imports components by `gameName` — **no manual case statement or switch needed**. The import path is:

```ts
`./${name}/${componentName}View.vue`
```

Where `name` is the `minigame()` string from `MiniGamePayloadType` and `componentName` is `'Host'` or `'Player'`.

This means:
- The directory name **MUST** be camelCase matching `get_camel_case_name()` from the backend
- The files **MUST** be named exactly `HostView.vue` and `PlayerView.vue`
- No registration, no import — it's automatic via `defineAsyncComponent` and dynamic `import()`

### Step 9 — Verify

After implementation, verify:

1. **Directory name** matches `get_camel_case_name()` from backend exactly (camelCase)
2. **Files** are named `HostView.vue` and `PlayerView.vue` exactly
3. **`defineExpose({ update })`** is present in both components
4. **`GameStateType` enum values** match what's in `frontend/src/flatbuffers/messageClass.ts`
5. **No `<style>` blocks** unless absolutely necessary (CSS animations only)
6. **No semicolons**, single quotes, 2-space indentation
7. **All FlatBuffer bigint fields** wrapped with `Number()`
8. **All FlatBuffer string fields** decoded with `decodeURI()`
9. **Game name string** in message builders matches backend `get_camel_case_name()`
10. **Host view scales properly** — check that canvas/layout uses scale factor or relative sizing, no hardcoded pixel values
11. **Player view is mobile-friendly** — check touch targets, vertical layout, no horizontal scroll

## Quick Reference — Code Style

| Element | Convention | Example |
|---------|-----------|---------|
| Indentation | 2 spaces (no tabs) | — |
| Semicolons | None | — |
| Quotes | Single | `'hello'` |
| Line width | 100 characters | — |
| Trailing commas | None | — |
| Component files | PascalCase | `HostView.vue`, `PlayerView.vue` |
| Minigame directories | camelCase | `teambasedPong/`, `crazyCounting/` |
| Model files | `{GameName}Models.ts` | `TeambasedPongModels.ts` |
| Processor files | `{GameName}Processor.ts` | `TeambasedPongProcessor.ts` |
| SpriteMap files | `{GameName}SpriteMap.ts` | `HighwayHustleSpriteMap.ts` |
| Interfaces | PascalCase | `TeambasedPongHostData` |
| Variables/functions | camelCase | `payloadData`, `sendPlayerAction` |
| Store composables | `use` + PascalCase + `Store` | `useWebSocketStore` |
| Script setup | Always `<script lang="ts" setup>` | — |
| Section order | `<script>` → `<template>` | No `<style>` blocks |
| Props | `defineProps<{ width: number; height: number }>()` | — |
| Expose | `defineExpose({ update })` | — |

## Common Pitfalls

- **Directory name mismatch** — must exactly match `get_camel_case_name()` from backend; `teambasedPong` not `TeambasedPong` or `teambased_pong`
- **Forgetting `defineExpose({ update })`** — GameManager calls `gameViewRef.value?.update(data)`, won't work without expose
- **Hardcoded pixel values in HostView** — breaks on different screen sizes; always use scale factor or relative units
- **Not wrapping FlatBuffer bigints with `Number()`** — causes TypeScript type errors or NaN rendering
- **Not decoding FlatBuffer strings with `decodeURI()`** — player names with special characters display as encoded
- **Using `<style>` blocks** — only Tailwind utility classes; exception only for complex CSS animations
- **Semicolons or double quotes** — Prettier config forbids both
- **Wrong GameStateType enum** — always read `messageClass.ts` after schema build; never guess values
- **Importing from individual generated files instead of barrel** — prefer `@/flatbuffers/messageClass` for types used in `update()` switch; individual imports are acceptable in Processor files
- **Not showing personal result on PlayerView results** — always compute `personalResult` from `websocketStore.clientName` and display it prominently
- **Missing `return []`** — the `update()` function should return `[]` at the end (convention from existing games)
- **Canvas not responsive** — always use ResizeObserver + computed dimensions or `Math.min(width, height)` patterns
