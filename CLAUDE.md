# People Party — AI Coding Agent Instructions

## Project Overview

People Party is a multiplayer party game with a **C++ WebSocket backend** and **Vue 3 + TypeScript frontend**, designed for big-screen gameplay with mobile phone controllers. The architecture uses **FlatBuffers** for binary serialization between client and server.

---

## Architecture & Communication

### Message Flow

```
Client (Vue) → WebSocket → nginx (/confetti/) → Backend (C++) → Party → Game → GameState
```

- WebSocket endpoints: `/host` (creates party) and `/join/:room/:name` (joins existing party)
- Party IDs are 4-digit numeric codes generated in `party.cpp`
- All messages use FlatBuffer `Message` wrapper with `MessageType` enum and `Payload` union
- Frontend WebSocket store: `frontend/src/stores/confettiStore.ts` (legacy name — manages all WS communication)

### FlatBuffers Schema System

- Schema definitions live in `schemes/` with auto-generation via `schemes/builder.sh`
- `builder.sh` Phase 1: scans `schemes/minigamedata/` recursively, auto-generates `MiniGamePayloadType.fbs` (enum `GameStateType` + union `GameStatePayload`)
- `builder.sh` Phase 2: scans `schemes/payloadtypes/`, auto-generates `messageClass.fbs` (enum `MessageType` + union `Payload` + root `Message` table)
- Run `npm run build` (or `build:macos` / `build:nix` / `build:windows`) to regenerate after `.fbs` changes
- Generated outputs:
  - C++: `backend/src/flatbuffer/messageClass_generated.h`
  - TypeScript: `frontend/src/flatbuffers/*.ts` (barrel: `messageClass.ts`)
- **Never edit generated files** — they are overwritten on each build

### Game State Machine

The `Game` class orchestrates three primary states:
1. **PartyPrep** — Lobby for selecting rounds/games
2. **MiniGame** — Active minigame (queued, randomly selected)
3. **Leaderboard** — Score display between games

State transitions use `Game::nextGameState<T>()` template pattern with `typeid` dispatch. The `finished()` method (protected, called by the state itself after a result timer fires) triggers transitions. After minigames, leaderboard updates via `Game::update_leaderboard()` with placement pairs.

---

## Adding a New Minigame

### 1. FlatBuffer Schemas

1. Create `schemes/minigamedata/{GameName}/` (PascalCase folder name)
2. Add payload `.fbs` files (minimum):
   - `{GameName}HostPayload.fbs` — data sent to big screen
   - `{GameName}PlayerInputPayload.fbs` — input from player to server
   - `{GameName}ResultPayload.fbs` — end-of-game results
3. Optional: `{GameName}PlayerPayload.fbs`, `{GameName}RoundResultPayload.fbs`, etc.
4. Run `npm run build:macos` (or platform variant) to regenerate schemas
5. See existing games like `CrazyCounting/` for reference

### 2. Backend (C++)

1. Create `backend/src/minigames/{game_name}/` (snake_case directory)
2. Create `{game_name}_mini_game.h` and `{game_name}_mini_game.cpp`
3. Class name: `{GameName}_MiniGame`, inheriting from `MiniGame`
4. Implement all required virtual methods (see code standards below)
5. Add `#include` in `game.h` and push to queue in `Game::add_minigames()` in `game.cpp`

### 3. Frontend (Vue 3)

1. Create `frontend/src/components/{gameName}/` (camelCase folder name)
2. Create `HostView.vue` (big screen) and `PlayerView.vue` (mobile controller)
3. Optional: `{GameName}Models.ts`, `{GameName}Processor.ts`, `{GameName}SpriteMap.ts`
4. Add case to `GameManager.vue` switch for both host and player views
5. Both views must `defineExpose({ update(data: MiniGamePayloadType) { ... } })`

---

## Code Standards — Backend (C++)

### Language & Build

- **C++23** (`CMAKE_CXX_STANDARD 23`)
- CMake 3.12+, Conan for dependencies
- `file(GLOB_RECURSE ...)` collects all source files — no explicit source listing
- Dependencies: uWebSockets, FlatBuffers, Box2D, Doctest (disabled)

### Naming Conventions

| Element | Convention | Example |
|---|---|---|
| Classes | `PascalCase` | `Game`, `Party`, `Client`, `GameState` |
| Minigame classes | `{Name}_MiniGame` | `CrazyCounting_MiniGame`, `HighwayHustle_MiniGame` |
| Supporting classes | `{Game}_{Role}` | `CrazyCounting_Entity`, `CrazyCounting_Player` |
| Methods | `snake_case` | `start_introduction()`, `send_host_update()` |
| Variables / members | `snake_case` | `party_id`, `remaining_time`, `update_interval` |
| File names | `snake_case` | `crazy_counting_mini_game.h`, `party_repository.cpp` |
| Minigame directories | `snake_case` | `backend/src/minigames/crazy_counting/` |
| Enums (hand-written) | `SCREAMING_SNAKE_CASE` values | `ROCK`, `PAPER`, `SCISSORS`, `NONE` |
| Constants | `snake_case` member variables | `const int match_time`, `const int result_time` |
| Repository factories | `PascalCase` | `CreateClient()`, `CreateParty()` |
| Time DSL macros | `SCREAMING_CASE` | `MILLISECONDS`, `SECONDS`, `MINUTES` |

### Header Guards

Use traditional `#ifndef` guards, **not** `#pragma once`:
```cpp
#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
// ...
#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
```

### Formatting

- **4-space indentation** (no tabs)
- **Allman braces** for class and function definitions (opening brace on its own line)
- K&R braces for short inline methods in headers
- File header comment: `// Created by {author} on {date}.`
- Includes: `""` for project files, `<>` for library/system headers

### Time Constants

Always use the DSL macros from `util/defines.h` — never raw integer milliseconds:
```cpp
7 SECONDS          // → 7000
500 MILLISECONDS   // → 500
2 MINUTES          // → 120000
```

### Memory Management

- **Raw pointers** throughout — no smart pointers
- `new`/`delete` manually; `delete` in destructors
- Repositories store objects by value in `std::map` and return raw `T*` pointers
- Game entities allocated with `new`, cleaned in destructor loops
- Every timer must be `.clear()`-ed in the destructor

### Threading

- `ThreadTimer` (`util/thread_timer.h`) is the sole concurrency mechanism
- `setInterval(callback, ms)` and `setTimeout(callback, ms)` spawn detached `std::thread`s
- **Critical**: timer callbacks run on background threads; all WebSocket sends must be deferred to the uWS event loop:
  ```cpp
  server_loop->defer([client, message]() { client->send(message); });
  ```
- Each minigame typically maintains 2–4 timers: `timer` (inherited), `introduction_timer`, `result_timer`, optionally more

### Global Singletons

Defined in `util/globals.h` using `inline`:
```cpp
inline PartyRepository party_repository;
inline ClientRepository client_repository;
inline uWS::Loop* server_loop;
```

### Error Handling

- No exceptions — void returns with early `return` on failure
- Null pointer checks via `if (ptr == nullptr) { return; }`
- Logging: `std::cout << "..." << std::endl;` — no logging framework

### Minigame Implementation Pattern

Every minigame follows this lifecycle:

1. **Constructor**: set `min_players`, `max_players`, initialize members
2. **Destructor**: `.clear()` all timers, `delete` all heap-allocated entities
3. **`start_introduction()`**: set `update_interval = 500 MILLISECONDS`, start `introduction_timer.setInterval()`
4. **`introduction_update(dt)`**: decrement `introduction_time`, call `send_minigame_introduction()`, on `<= 0` → `start_minigame()`
5. **`start_minigame()`**: populate player map from `game->get_clients()`, start `timer.setInterval()`
6. **`update(dt)`**: decrement `remaining_time`, send state updates, on `<= 0` → `start_result()`
7. **`start_result()`**: send result payload, `result_timer.setTimeout([this](){ finished(); }, N SECONDS)`
8. **`process_input(payload, from)`**: `switch(payload->gamestatetype())` → handle per-type
9. **`getMinigameResult()`**: return `std::vector<std::pair<Client*, int>>` — client + placement

Player data stored in `std::map<int, PlayerData>` keyed by `client_id`.

### FlatBuffer Building Pattern (C++)

```cpp
flatbuffers::FlatBufferBuilder builder;
// 1. Create strings/nested objects before the tables that reference them
auto name = builder.CreateString("gameName");
// 2. Create leaf payload
auto payload = CreateSomePayload(builder, field1, field2);
// 3. Wrap in MiniGamePayloadType with GameStateType enum
auto gsPayload = CreateMiniGamePayloadType(builder, name,
    GameStateType_SomeResult, GameStatePayload_SomeResultPayload, payload.Union());
// 4. Send via party
game->party->send_gamestate(filter_lambda, builder, gsPayload.Union());
```

Filter lambdas for targeting:
- Host only: `[](Client* c) { return c->party->host == c; }`
- Specific client: `[id](Client* c) { return c->client_id == id; }`
- All clients: `[](Client* c) { return true; }`

### Parsing Incoming Messages (C++)

```cpp
const uint8_t* buffer = reinterpret_cast<const uint8_t*>(message.data());
auto parsedMessage = GetMessage(buffer);
auto miniGamePayload = parsedMessage->payload_as_MiniGamePayloadType();
auto input = miniGamePayload->gamestatepayload_as_SomePlayerInputPayload();
```

---

## Code Standards — Frontend (Vue 3 + TypeScript)

### Component Conventions

- **Always `<script setup lang="ts">`** — Composition API only, no Options API
- **Section order**: `<script>` → `<template>` — no `<style>` blocks (Tailwind only)
- Props: `defineProps<{ ... }>()` with TypeScript generics
- Emits: `defineEmits<{ eventName: [arg1Type] }>()` with TypeScript generics
- Minigame views expose: `defineExpose({ update(data: MiniGamePayloadType) { ... } })`

### Naming Conventions

| Element | Convention | Example |
|---|---|---|
| Component files | `PascalCase.vue` | `PartyButton.vue`, `HostView.vue` |
| Minigame folders | `camelCase` | `crazyCounting/`, `highwayHustle/` |
| Store files | `camelCase` + `Store.ts` | `confettiStore.ts`, `viewStore.ts` |
| Store composables | `use` + `PascalCase` + `Store` | `useWebSocketStore`, `useViewStore` |
| Pinia store IDs | lowercase, no prefix | `'websocket'`, `'view'`, `'color'` |
| Model files | `{GameName}Models.ts` | `HighwayHustleModels.ts` |
| Processor files | `{GameName}Processor.ts` | `HighwayHustleProcessor.ts` |
| SpriteMap files | `{GameName}SpriteMap.ts` | `HighwayHustleSpriteMap.ts` |
| Util files | `camelCase.ts` | `flatbufferMessageBuilder.ts` |
| Interfaces/types | `PascalCase` | `HighwayHustleData`, `LeaderboardPlayer` |
| Variables/functions | `camelCase` | `partyCode`, `sendMessage` |
| Route names | lowercase strings | `'host'`, `'join'`, `'info'` |

### Formatting (Prettier)

- **No semicolons**
- **Single quotes**
- **2-space indentation**
- **100-character line width**
- **No trailing commas**

### TypeScript Usage

- `strict: false` in tsconfig — not strict mode
- `interface` preferred for object shapes and model structs
- `type` used for unions and store-exported data shapes
- `import type { ... }` for type-only imports (used but not fully enforced)
- `Number()` wraps all FlatBuffer `bigint` fields
- `decodeURI()` on all FlatBuffer string fields (names are URI-encoded on backend)
- `@/` path alias for cross-directory imports; relative `../` within same folder

### State Management (Pinia)

Always setup store syntax:
```ts
export const useXxxStore = defineStore('id', () => {
  const state = ref<Type>(default)
  function action() { ... }
  return { state, action }
})
```
Use `storeToRefs()` for reactive destructuring in consumers.

### Styling

- **No `<style>` blocks** — all styling via Tailwind utility classes in templates
- Global CSS in `assets/main.css` with Tailwind directives and CSS custom properties
- Color tokens backed by CSS variables (`bg-primary`, `text-primary` → `var(--color-primary)`)
- Custom font: `font-kanit` applied at root
- Inline `style=` only when Tailwind can't express the value
- Mobile viewport: `calc(var(--vh, 1vh) * 100)` for iOS Safari

### WebSocket Communication

**Sending:**
```ts
const builder = new flatbuffers.Builder()
const payload = SomePayload.createSomePayload(builder, ...)
const mgPayload = MiniGamePayloadType.createMiniGamePayloadType(
  builder, builder.createString('gameName'), GameStateType.X, GameStatePayload.X, payload
)
websocketStore.sendMessage(
  buildMessage(builder, mgPayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
)
```

**Receiving** (in confettiStore):
```ts
const buf = new flatbuffers.ByteBuffer(data)
const msg = Message.getRootAsMessage(buf)
// switch on msg.type(), extract payload with msg.payload(new SomePayloadType())
```

**Always import from `@/flatbuffers/messageClass`** (barrel file) — never from individual generated files.

### PixiJS Integration

- `vue3-pixi` + `pixi.js` v8, configured in `vite.config.ts`
- `<Application>` component as canvas container
- Drawing via `:draw` prop with imperative `Graphics` API
- Sprite lookups in `{GameName}SpriteMap.ts` files
- `onTick` from `vue3-pixi` for animation loops

### Routing

- `createWebHistory` with lazy-loaded route components: `() => import('../routes/XRoute.vue')`
- Mobile detection at `/` redirects to `/join` via UA sniffing (`util/detectmobilebrowser.ts`)
- Four routes: `host`, `join`, `info`, `releases`

### Minigame Component Pattern

Each minigame has a local `ViewState` enum:
```ts
enum ViewState { None, Introduction, MiniGame, Results }
```

Components manage their own state transitions by switching on `gamestatetype` from incoming `MiniGamePayloadType` updates.

---

## Code Standards — FlatBuffer Schemas

### File Organization

```
schemes/
├── builder.sh                          ← Auto-generation script (run via npm)
├── messageClass.fbs                    ← AUTO-GENERATED root schema
├── payloadtypes/                       ← Top-level protocol types
│   ├── HostPayloadType.fbs
│   ├── JoinPayloadType.fbs
│   ├── LeaderboardPayloadType.fbs
│   ├── MiniGamePayloadType.fbs         ← AUTO-GENERATED by builder.sh
│   ├── PartyPrepPayloadType.fbs
│   └── PausePayloadType.fbs
└── minigamedata/                       ← Minigame-specific schemas
    ├── JoystickDataPayload.fbs         ← Shared payloads (root-level)
    ├── JoystickEventPayload.fbs
    ├── MiniGameIntroductionPayload.fbs
    └── {GameName}/                     ← PascalCase folder per game
        ├── {GameName}HostPayload.fbs
        ├── {GameName}PlayerInputPayload.fbs
        └── {GameName}ResultPayload.fbs
```

### Naming Conventions

| Element | Convention | Example |
|---|---|---|
| Table names | `PascalCase` + purpose suffix | `CrazyCountingHostEntitiesPayload` |
| Helper/sub-tables | `FB` prefix + `PascalCase` | `FBCrazyCountingEntity`, `FBLeaderboardPlayer`, `FBVec2` |
| Field names | `snake_case` | `time_left`, `x_pos`, `is_dead`, `car_type` |
| Enum names | `PascalCase` | `GameStateType`, `MessageType`, `Input` |
| Enum values | `PascalCase` for protocol values | `Host`, `Join`, `MiniGame`, `Start`, `Stop` |
| Enum values | `SCREAMING_SNAKE_CASE` for game constants | `TEAM_A`, `ROCK`, `PAPER`, `BALLOON` |
| Union names | `PascalCase` + `Payload` suffix | `Payload`, `GameStatePayload`, `PartyPrepPayload` |
| Wrapper tables | `{Domain}PayloadType` | `MiniGamePayloadType`, `LeaderboardPayloadType` |
| Schema files | `PascalCase` + purpose suffix | `CrazyCountingHostEntitiesPayload.fbs` |
| Schema directories | `PascalCase` | `CrazyCounting/`, `HighwayHustle/` |
| Namespaces | **Not used** | — |
| `file_identifier` | **Not used** | — |

### Enum Underlying Types

Default to `: byte`. Use `: ubyte` only when explicitly needed (e.g., `FBEntityType: ubyte`).

### Payload Purpose Suffixes

| Suffix | Direction | Description |
|---|---|---|
| `HostPayload` | server → host screen | Big screen display data |
| `HostEntitiesPayload` | server → host screen | Entity-focused host data |
| `PlayerPayload` | server → player phone | Individual player state |
| `PlayerUpdatePayload` | server → player phone | Per-tick player state |
| `PlayerInputPayload` | player → server | Player action/input |
| `ResultPayload` | server → all | End-of-game results |
| `RoundResultPayload` | server → all | Per-round results |
| `RoundPrepPayload` | server → all | Pre-round setup data |

### Auto-Generation Rules

- `builder.sh` strips `"Payload"` from table names to create `GameStateType` enum values
  - `CrazyCountingHostEntitiesPayload` → enum value `CrazyCountingHostEntities`
- `builder.sh` strips `"PayloadType"` from filenames to create `MessageType` enum values
  - `MiniGamePayloadType.fbs` → enum value `MiniGame`
- Enum integer values depend on filesystem traversal order — **always reference by name, not number**

---

## Development Workflows

### Build & Run

**Full stack (Docker):**
```bash
npm run start  # Installs deps, builds schemas, starts docker-compose-dev
```

**Backend only (local):**
```bash
cd backend
conan install . --output-folder=build --build=missing --settings=build_type=Debug
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
./PeopleParty-Backend
```

**Frontend only:**
```bash
cd frontend
npm install
npm run dev  # Runs on 0.0.0.0 for mobile testing
```

**Schema regeneration (required after `.fbs` changes):**
```bash
npm run build:macos   # macOS
npm run build:nix     # Linux
npm run build:windows # Windows
```

### Docker Architecture

Three services in all environments:

| Service | Role | Internal Port |
|---|---|---|
| `nginx` | Reverse proxy, routes `/confetti/` → backend | 80 |
| `client` | Vue frontend (SPA) | 80 |
| `api` | C++ WebSocket backend | 7899 |

Exposed port: `7789` (dev/prod) or `7889` (staging). Production images from `ghcr.io/kraanter/peopleparty*`.

### Testing

- Frontend: Cypress scaffolded (`npm run test:unit:dev`, `npm run test:e2e:dev`) — no tests written yet
- Backend: Doctest framework present but disabled in CMakeLists.txt

---

## Key Dependencies

| Layer | Dependency | Version | Purpose |
|---|---|---|---|
| Backend | uWebSockets | v20.71.0 | WebSocket server (SSL) |
| Backend | FlatBuffers | v24.3.25 | Binary serialization |
| Backend | Box2D | v2.4.1 | Physics (minigames) |
| Backend | Doctest | — | Testing (disabled) |
| Backend | Conan | — | C++ package manager |
| Frontend | Vue 3 | — | UI framework |
| Frontend | Pinia | — | State management |
| Frontend | Vue Router | — | Client-side routing |
| Frontend | PixiJS v8 | — | Canvas rendering |
| Frontend | vue3-pixi | — | Vue ↔ PixiJS bridge |
| Frontend | Tailwind CSS | — | Utility-first styling |
| Frontend | FlatBuffers.js | — | Binary serialization |

---

## Important File Locations

### Backend
- Entry point: `backend/src/main.cpp`
- WebSocket handler: `backend/src/websocket.h/cpp`
- Party management: `backend/src/party.h/cpp`, `backend/src/party_repository.h/cpp`
- Client model: `backend/src/client.h/cpp`, `backend/src/client_repository.h/cpp`
- Game orchestrator: `backend/src/game.h/cpp`
- Game state base: `backend/src/game_state.h/cpp`
- Minigame base class: `backend/src/minigames/minigame.h/cpp`
- FlatBuffer helpers: `backend/src/util/flatbuffer/flatbuffer_message_builder.h/cpp`
- Timer utility: `backend/src/util/thread_timer.h`
- Time macros: `backend/src/util/defines.h`
- Global singletons: `backend/src/util/globals.h`
- Type aliases: `backend/src/util/typedefs.h`
- Socket data: `backend/src/util/socketdata.h`
- Generated FlatBuffer header: `backend/src/flatbuffer/messageClass_generated.h`

### Frontend
- App root: `frontend/src/App.vue`
- Bootstrap: `frontend/src/main.ts`
- WebSocket store: `frontend/src/stores/confettiStore.ts`
- View state store: `frontend/src/stores/viewStore.ts`
- Color theming store: `frontend/src/stores/colorStore.ts`
- Router: `frontend/src/router/index.ts`
- Game manager (minigame switch): `frontend/src/components/GameManager.vue`
- FlatBuffer barrel export: `frontend/src/flatbuffers/messageClass.ts`
- FlatBuffer message builder utility: `frontend/src/util/flatbufferMessageBuilder.ts`
- Joystick message builder: `frontend/src/util/joystickMessageBuilder.ts`
- Mobile detection: `frontend/src/util/detectmobilebrowser.ts`
- Global styles: `frontend/src/assets/main.css`

### Schemas
- Schema builder: `schemes/builder.sh`
- Root schema (auto-generated): `schemes/messageClass.fbs`
- Protocol payloads: `schemes/payloadtypes/`
- Minigame payloads: `schemes/minigamedata/`

### Infrastructure
- Nginx config: `nginx/default.conf`
- Dev compose: `docker-compose-dev.yml`
- Production compose: `docker-compose.yml`

---

## Common Gotchas

- **Always rebuild schemas** after `.fbs` changes — errors from stale generated code are cryptic
- **Party IDs** must be numeric 4-digit codes (validated in `websocket.cpp`)
- **WebSocket binary type** must be `'arraybuffer'` (set in confettiStore)
- **Minigames must be added** to `Game::add_minigames()` queue — creating the class isn't enough
- **Frontend component names** must match `GameStateType` enum values for routing in `GameManager.vue`
- **Timer threads** are not on the uWS event loop — always `server_loop->defer()` before sending WebSocket messages
- **FlatBuffer strings/vectors** must be created *before* the table that references them
- **`Number()` wrap** all FlatBuffer bigint fields in TypeScript; **`decodeURI()`** all string fields
- **`builder.sh` ordering** is filesystem-dependent — never hardcode `GameStateType` integer values
- **No `<style>` blocks** in Vue components — Tailwind utility classes only
- **No smart pointers** in backend — follow existing raw pointer / manual delete patterns

---
---

# Skill: Create Minigame Schemas

Create the FlatBuffer `.fbs` schema files for a new PeopleParty minigame under `schemes/minigamedata/`.

## When to Use

- User asks to create a new minigame
- User asks to create or add FlatBuffer schemas/payloads for a game
- User mentions `.fbs` files for a minigame
- User asks to set up the data layer for a new game

## Procedure

### Step 1 — Gather Context

Before writing any schemas, read existing examples to match established patterns.

**Required reading — reference games** (read ALL schemas in each folder):
- `schemes/minigamedata/TeambasedPong/` — newest game; multi-round with RoundPrep, RoundResult, separate Player and Host payloads
- `schemes/minigamedata/HighwayHustle/` — entity-based with host entities, player state, results
- `schemes/minigamedata/MarbleMania/` — complex with `FBVec2` helper tables, unions, entity types
- `schemes/minigamedata/Unscrambled/` — multi-round with separate round results

**Required reading — shared payloads** (at the root of `schemes/minigamedata/`):
- `JoystickDataPayload.fbs`
- `JoystickEventPayload.fbs`
- `MiniGameIntroductionPayload.fbs`

**Required reading — build system**:
- `schemes/builder.sh` — understand how auto-generation works:
  - Strips `"Payload"` from table names → `GameStateType` enum values
  - Strips `"PayloadType"` from filenames → `MessageType` enum values
  - Enum integer values depend on filesystem order — **never hardcode them**

### Step 2 — Determine Required Files

Ask the user about game mechanics if not already clear. Then decide which files to create.

**MINIMUM required files** (every minigame needs these):

| File | Direction | Purpose |
|------|-----------|---------|
| `{GameName}HostPayload.fbs` | server → host screen | Big screen display data |
| `{GameName}PlayerInputPayload.fbs` | player → server | Player actions/input |
| `{GameName}ResultPayload.fbs` | server → all | Final game results |

**OPTIONAL files** (based on game design):

| File | Direction | When to include |
|------|-----------|-----------------|
| `{GameName}PlayerPayload.fbs` | server → player | Player needs per-player state on their phone |
| `{GameName}HostEntitiesPayload.fbs` | server → host | Host updates are entity-heavy (separate from timer/state data) |
| `{GameName}PlayerUpdatePayload.fbs` | server → player | Per-tick player state updates |
| `{GameName}RoundResultPayload.fbs` | server → all | Game has multiple rounds with results between them |
| `{GameName}RoundPrepPayload.fbs` | server → all | Game needs a setup phase before each round (e.g., team assignments) |

### Step 3 — Create the Schema Files

Create directory `schemes/minigamedata/{GameName}/` using **PascalCase** for the folder name.

Follow these rules strictly — auto-generation depends on correct naming:

#### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Directory | PascalCase | `TeambasedPong/`, `MarbleMania/` |
| Schema files | PascalCase + purpose suffix | `TeambasedPongHostPayload.fbs` |
| Table names | PascalCase + purpose suffix | `TeambasedPongHostPayload` |
| Helper/sub-tables | `FB` prefix + PascalCase | `FBTeamPlayer`, `FBVec2`, `FBHighwayHustleEntity` |
| Field names | snake_case | `time_left`, `x_pos`, `is_dead` |
| Enum names | PascalCase | `PongTeam`, `FBEntityType` |
| Enum values (protocol) | PascalCase | `Host`, `Join`, `MiniGame` |
| Enum values (game constants) | SCREAMING_SNAKE_CASE | `TEAM_A`, `ROCK`, `BALLOON` |
| Enum underlying type | `: byte` default | Use `: ubyte` only when explicitly needed |

#### Schema File Rules

- Every file **must** end with `root_type {MainTableName};`
- **No namespaces** — never add `namespace` declarations
- **No file_identifiers** — never add `file_identifier`
- Helper tables (`FB`-prefixed) are defined in the **same file** as the table that uses them
- Enums are defined in the same file as the table that uses them
- FlatBuffer strings/vectors must be created before the table referencing them (relevant for C++ usage, but the schema itself just declares them)

#### Common Field Type Guidelines

| Use case | FlatBuffer type |
|----------|----------------|
| Timestamps | `ulong` |
| Milliseconds (timers) | `int` |
| Positions / coordinates | `float` |
| Small counters / indices | `ushort` |
| Boolean flags | `bool` |
| Names / text | `string` |
| Lists of player names | `[string]` |

#### Common Patterns

**Result payload** — always follows this pattern:
```fbs
table FB{GameName}ResultPair {
  name: string;
  placement: ushort;
}

table {GameName}ResultPayload {
  results: [FB{GameName}ResultPair];
}

root_type {GameName}ResultPayload;
```

**Host payload** — typically includes a timer:
```fbs
table {GameName}HostPayload {
  time_left: int;  // milliseconds
  // ... game-specific fields
}

root_type {GameName}HostPayload;
```

**Player input payload** — minimal, only the action:
```fbs
table {GameName}PlayerInputPayload {
  // ... input fields (e.g., answer: string, direction: ushort)
}

root_type {GameName}PlayerInputPayload;
```

**Player payload** — only what that specific player needs:
```fbs
table {GameName}PlayerPayload {
  // ... per-player state
}

root_type {GameName}PlayerPayload;
```

**Joystick-based games** — do NOT create new joystick schemas. Reuse the existing shared payloads:
- `schemes/minigamedata/JoystickDataPayload.fbs`
- `schemes/minigamedata/JoystickEventPayload.fbs`

**Entity-heavy host updates** — use a helper table per entity type:
```fbs
table FB{GameName}Entity {
  id: string;
  x_pos: float;
  y_pos: float;
  // ... entity-specific fields
}

table {GameName}HostPayload {
  entities: [FB{GameName}Entity];
  time_left: int;
}

root_type {GameName}HostPayload;
```

**Multi-round games** — add RoundPrep and/or RoundResult payloads:
```fbs
table {GameName}RoundPrepPayload {
  current_round: ushort;
  time_left: int;
  // ... round setup data
}

root_type {GameName}RoundPrepPayload;
```

### Step 4 — Post-Creation Reminder

After creating all schema files, remind the user:

> Run `npm run build:macos` (or `npm run build:nix` / `npm run build:windows`) to regenerate the C++ and TypeScript bindings from the updated schemas. Never edit generated files — they are overwritten on each build.

The generated outputs will be:
- **C++**: `backend/src/flatbuffer/messageClass_generated.h`
- **TypeScript**: `frontend/src/flatbuffers/*.ts` (barrel: `messageClass.ts`)

---
---

# Skill: Create Minigame Backend

Implement the full C++ backend for a new PeopleParty minigame — lifecycle management, game states, physics, timers, FlatBuffer serialization, and wiring into the game loop.

## When to Use

- User asks to create a new minigame backend
- User asks to implement game logic in C++
- User asks to add a new game to the backend
- User asks to wire a minigame into the game loop
- User asks to work on backend minigame code (lifecycle, physics, timers)

## Procedure

### Step 1 — Gather Context

Before writing any code, read existing implementations to match established patterns. Read BOTH `.h` and `.cpp` files for each.

**Required reading — base classes** (read first):
- `backend/src/minigames/minigame.h` and `minigame.cpp` — base class with ALL virtual methods
- `backend/src/game_state.h` — base `GameState` class

**Required reading — reference games** (read ALL files in each directory):
- `backend/src/minigames/teambased_pong/` — **NEWEST, best reference**: multi-phase rounds (ROUND_PREP → PLAYING → ROUND_RESULT), Box2D physics, team management, elimination, averaged joystick input
- `backend/src/minigames/highway_hustle/` — entity-based with map class, moving entities, joystick input, custom AABB collision, obstacle spawning, lane system
- `backend/src/minigames/marble_mania/` — Box2D physics with gravity, procedural map generation, placement + simulation phases, PHYSICS_SCALE coordinate scaling

**Recommended reading — additional patterns** (scan `.h` files):
- `backend/src/minigames/unscrambled/` — multi-round with round phases, word data management
- `backend/src/minigames/memory_mixer/` — grid-based, elimination, round progression, `build_grid()` reusable builder pattern
- `backend/src/minigames/crazy_counting/` — simpler entity pattern with steering behaviors, `clients_changed` override
- `backend/src/minigames/right_on_time/` — multi-round timing game with per-round targets
- `backend/src/minigames/rps_bracket/` — bracket/tournament, match management with stack-based results
- `backend/src/minigames/launch_party/` — reaction time, practice rounds, phase-based with lights, `std::chrono` timing
- `backend/src/minigames/business_bailout/` — stock-market style, bail mechanic, fine-grained 100ms updates

**Required reading — infrastructure**:
- `backend/src/game.h` and `game.cpp` — `Game::add_minigames()` queue, `update_leaderboard()`, `nextGameState()`, `handle_new_minigame()`
- `backend/src/party.h` — `send_gamestate()`, `send_message()` signatures and filter lambda patterns
- `backend/src/util/thread_timer.h` — `setInterval()`, `setTimeout()`, `clear()`, `pause()`, `resume()`
- `backend/src/util/defines.h` — time DSL macros (`SECONDS`, `MILLISECONDS`, `MINUTES`)
- `backend/src/util/globals.h` — `server_loop`, `party_repository`, `client_repository`
- `backend/src/util/flatbuffer/flatbuffer_message_builder.h` and `.cpp` — only used for handshake messages, NOT minigame payloads

### Step 2 — Confirm Prerequisites

Before creating backend files, verify:
1. FlatBuffer schemas exist in `schemes/minigamedata/{GameName}/` — if not, create them first (use the create-minigame-schemas instructions above)
2. Schemas have been built (`npm run build:macos` or platform variant) so that `backend/src/flatbuffer/messageClass_generated.h` contains the generated types
3. Read `messageClass_generated.h` to find the exact `GameStateType_` and `GameStatePayload_` enum values for the new game's payloads

### Step 3 — Create Files

**Directory**: `backend/src/minigames/{game_name}/` (snake_case)

**Minimum files**:

| File | Contents |
|------|----------|
| `{game_name}_mini_game.h` | Main class header |
| `{game_name}_mini_game.cpp` | Main class implementation |

**Optional supporting files** (based on game complexity):

| File | When to add |
|------|-------------|
| `{game_name}_player.h` | Player data struct beyond simple map entry |
| `{game_name}_map.h` / `.cpp` | Separate map/world management (physics, entities, collision) |
| `{game_name}_entity.h` | Custom entity types |
| `{game_name}_enums.h` | Phase enums, team enums, game-specific constants |

**Naming**:
- Class name: `{GameName}_MiniGame` (e.g., `TeambasedPong_MiniGame`)
- Supporting classes: `{GameName}_{Role}` (e.g., `TeambasedPong_Player`, `TeambasedPong_Map`)
- Check existing games for exact file naming conventions — some use `_mini_game`, others `_minigame`

### Step 4 — Implement the Header

Use `#ifndef` header guards (NOT `#pragma once`):

```cpp
#ifndef PEOPLEPARTY_BACKEND_{GAME_NAME}_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_{GAME_NAME}_MINI_GAME_H

// Created by {author} on {date}.

#include "../minigame.h"
#include "../../util/thread_timer.h"

class {GameName}_MiniGame : public MiniGame
{
public:
    {GameName}_MiniGame(Game* game);
    ~{GameName}_MiniGame() override;

    // Lifecycle
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;

    // Game loop
    void update(int delta_time) override;
    void process_input(const MiniGamePayloadType* payload, Client* from) override;

    // Metadata
    std::string get_display_name() override;
    std::string get_camel_case_name() override;
    std::string get_description() override;

    // Results
    std::vector<std::pair<Client*, int>> getMinigameResult() override;

    // Player management
    void clients_changed(int client_id, bool joined) override;

    // Pause
    void pause() override;
    void resume() override;

private:
    // Timers — ALL must be .clear()-ed in destructor
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;

    // Time constants — always use DSL macros
    int remaining_time = 60 SECONDS;
    const int result_time = 5 SECONDS;
    int delta_time;

    // Player data
    std::map<int, {GameName}_Player> players;

    // Send helpers
    void introduction_update(int dt);
    void send_host_update();
    void send_player_update(int client_id);
    void send_result_data();
};

#endif //PEOPLEPARTY_BACKEND_{GAME_NAME}_MINI_GAME_H
```

### Step 5 — Implement the Lifecycle

Every minigame MUST implement this lifecycle. Follow the pattern exactly:

#### Constructor
```cpp
{GameName}_MiniGame::{GameName}_MiniGame(Game* game) : MiniGame(game)
{
    min_players = 2;
    max_players = -1;  // -1 = no limit
    delta_time = 50;   // or: floor(1000 / target_fps)
}
```

#### Destructor — CRITICAL: clear ALL timers, delete ALL heap allocations
```cpp
{GameName}_MiniGame::~{GameName}_MiniGame()
{
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    // delete any heap-allocated entities:
    // for (auto& [key, entity] : entities) { delete entity; }
    // delete map;
}
```

#### start_introduction()
```cpp
void {GameName}_MiniGame::start_introduction()
{
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]()
    {
        introduction_update(update_interval);
    }, update_interval);
}
```

#### introduction_update(dt)
```cpp
void {GameName}_MiniGame::introduction_update(int dt)
{
    introduction_time -= dt;
    send_minigame_introduction(
        get_camel_case_name(),
        introduction_time,
        get_display_name(),
        get_description()
    );
    if (introduction_time <= 0)
    {
        introduction_timer.clear();
        start_minigame();
    }
}
```

#### start_minigame()
```cpp
void {GameName}_MiniGame::start_minigame()
{
    update_interval = delta_time;
    // Populate players from game->get_clients()
    for (auto* client : game->get_clients())
    {
        if (client->party->host == client) continue;  // skip host
        players[client->client_id] = {GameName}_Player{client};
    }

    minigame_timer.setInterval([this]()
    {
        update(delta_time);
    }, delta_time);
}
```

#### update(dt) — main game loop
```cpp
void {GameName}_MiniGame::update(int dt)
{
    remaining_time -= dt;
    // Game logic here
    send_host_update();
    // Send per-player updates if needed

    if (remaining_time <= 0)
    {
        minigame_timer.clear();
        start_result();
    }
}
```

#### start_result()
```cpp
void {GameName}_MiniGame::start_result()
{
    send_result_data();
    result_timer.setTimeout([this]()
    {
        finished();
    }, result_time);
}
```

#### getMinigameResult()
```cpp
std::vector<std::pair<Client*, int>> {GameName}_MiniGame::getMinigameResult()
{
    std::vector<std::pair<Client*, int>> results;
    // Sort players by score/placement
    // Push pairs of (client_ptr, placement_int) — 1 = first place
    return results;
}
```

#### Metadata methods
```cpp
std::string {GameName}_MiniGame::get_display_name() { return "Game Display Name"; }
std::string {GameName}_MiniGame::get_camel_case_name() { return "gameNameCamelCase"; }
std::string {GameName}_MiniGame::get_description() { return "Short description shown during intro."; }
```

#### clients_changed — empty unless mid-game join/leave handling needed
```cpp
void {GameName}_MiniGame::clients_changed(int client_id, bool joined) { }
```

#### pause / resume
```cpp
void {GameName}_MiniGame::pause()
{
    introduction_timer.pause();
    minigame_timer.pause();
    result_timer.pause();
}

void {GameName}_MiniGame::resume()
{
    introduction_timer.resume();
    minigame_timer.resume();
    result_timer.resume();
}
```

### Step 6 — Multi-Phase Games

For games with multiple phases (like teambased_pong), use a phase enum pattern:

```cpp
// In {game_name}_enums.h or at top of header:
enum class {GameName}Phase : uint8_t
{
    ROUND_PREP = 0,
    PLAYING = 1,
    ROUND_RESULT = 2
};
```

In `update()`, dispatch by phase:
```cpp
void {GameName}_MiniGame::update(int dt)
{
    switch (current_phase)
    {
        case {GameName}Phase::ROUND_PREP:
            update_round_prep_phase(dt);
            break;
        case {GameName}Phase::PLAYING:
            update_playing_phase(dt);
            break;
        case {GameName}Phase::ROUND_RESULT:
            update_round_result_phase(dt);
            break;
    }
}
```

Each phase manages its own timer, countdown, and transition to the next phase.

### Step 7 — FlatBuffer Serialization

#### Building and sending payloads

```cpp
void {GameName}_MiniGame::send_host_update()
{
    flatbuffers::FlatBufferBuilder builder;

    // 1. Create strings/vectors BEFORE the tables that reference them
    auto someString = builder.CreateString("value");

    // 2. Create the leaf payload
    auto payload = Create{GameName}HostPayload(builder, remaining_time, someString /*, ...*/);

    // 3. Wrap in MiniGamePayloadType with correct enum values
    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(builder,
        miniGame,
        GameStateType_{GameName}Host,          // from auto-generated enum
        GameStatePayload_{GameName}HostPayload, // from auto-generated union
        payload.Union());

    // 4. Send via party with filter lambda
    game->party->send_gamestate(
        [](Client* client) { return client->party->host == client; },  // host only
        builder,
        gameStatePayload.Union());
}
```

#### Common filter lambdas

| Target | Lambda |
|--------|--------|
| Host only | `[](Client* c) { return c->party->host == c; }` |
| Specific client | `[client_id](Client* c) { return c->client_id == client_id; }` |
| All clients | `[](Client* c) { return c == c; }` |

**IMPORTANT**: `GameStateType_` and `GameStatePayload_` values are auto-generated by `builder.sh` from the schema table names (stripping `"Payload"` suffix). Read `messageClass_generated.h` to find the exact enum values for your game.

#### Parsing incoming messages

```cpp
void {GameName}_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from)
{
    switch (payload->gamestatetype())
    {
        case GameStateType_{GameName}PlayerInput:
        {
            auto input = payload->gamestatepayload_as_{GameName}PlayerInputPayload();
            // Handle input...
            break;
        }
        case GameStateType_JoystickData:
        {
            auto joystick = payload->gamestatepayload_as_JoystickDataPayload();
            float x = joystick->x();
            float y = joystick->y();
            // Handle joystick...
            break;
        }
        case GameStateType_JoystickEvent:
        {
            auto event = payload->gamestatepayload_as_JoystickEventPayload();
            // Handle start/stop...
            break;
        }
        default:
            break;
    }
}
```

### Step 8 — Box2D Physics (if needed)

If the game uses physics, follow the marble_mania / teambased_pong patterns:

```cpp
// Constants
static constexpr float PHYSICS_SCALE = 100.0f;
static constexpr float MAP_WIDTH = 800.0f;
static constexpr float MAP_HEIGHT = 600.0f;

// Coordinate conversion helpers
Vector2D ToPhysicsCoords(const Vector2D& world) const { return world / PHYSICS_SCALE; }
Vector2D ToWorldCoords(const Vector2D& physics) const { return physics * PHYSICS_SCALE; }

// World setup
b2Vec2 gravity(0.0f, 0.0f);  // or (0.0f, 9.8f) for platformers
b2World* world = new b2World(gravity);

// Body creation
b2BodyDef bodyDef;
bodyDef.type = b2_dynamicBody;  // or b2_staticBody, b2_kinematicBody
bodyDef.position.Set(x / PHYSICS_SCALE, y / PHYSICS_SCALE);
bodyDef.bullet = true;  // for fast-moving objects (CCD)
b2Body* body = world->CreateBody(&bodyDef);

// Stepping
world->Step(dt / 1000.0f, velocityIterations, positionIterations);
```

For custom collision handling, use a `b2ContactListener` subclass — see `PongContactListener` in teambased_pong.

### Step 9 — Threading Rules

**CRITICAL**: Timer callbacks (`setInterval`, `setTimeout`) run on detached background threads, NOT the uWS event loop.

- All WebSocket sends through `party->send_gamestate()` or `party->send_message()` are already safe — they use `server_loop->defer()` internally
- If you ever need to send directly: `server_loop->defer([client, msg]() { client->send(msg); });`
- Never access WebSocket APIs directly from timer callbacks without `defer()`

### Step 10 — Wire Into Game Loop

After implementing the minigame, wire it:

**1. Add include in `game.h`:**
```cpp
#include "minigames/{game_name}/{game_name}_mini_game.h"
```

**2. Add to `Game::add_minigames()` in `game.cpp`:**
```cpp
if (party->settings->IsMiniGameEnabled("{gameNameCamelCase}"))
{
    temp_minigames.push_back(new {GameName}_MiniGame(this));
}
```

The `camelCase` key must match `get_camel_case_name()` and must match the frontend component folder name.

### Step 11 — Verify

After implementation:
1. Check all timers are `.clear()`-ed in the destructor
2. Check all `new`-allocated objects are `delete`-d in the destructor
3. Verify `get_camel_case_name()` matches the frontend component folder and FlatBuffer schema naming
4. Verify all `GameStateType_` and `GameStatePayload_` enum references match `messageClass_generated.h`
5. Build to check for compile errors: `cd backend/build && cmake --build .`

## Quick Reference — Backend Code Style

| Element | Convention | Example |
|---------|-----------|---------|
| Indentation | 4 spaces (no tabs) | — |
| Braces | Allman (opening on own line) | `void foo()\n{` |
| Methods | snake_case | `start_introduction()`, `send_host_update()` |
| Variables | snake_case | `remaining_time`, `delta_time` |
| Classes | PascalCase + _MiniGame | `TeambasedPong_MiniGame` |
| Header guards | `#ifndef PEOPLEPARTY_BACKEND_{NAME}_H` | Not `#pragma once` |
| Time | DSL macros | `7 SECONDS`, `500 MILLISECONDS` |
| Memory | Raw pointers, manual new/delete | No smart pointers |
| File header | `// Created by {author} on {date}.` | — |
| Includes | `""` for project, `<>` for libraries | — |

## Common Backend Pitfalls

- **Forgetting to clear timers in destructor** — causes crashes from dangling `this` captures
- **Forgetting to wire in `game.h` / `game.cpp`** — game compiles but never appears in the queue
- **Wrong `GameStateType_` enum values** — always read `messageClass_generated.h` after schema build, never guess
- **Creating FlatBuffer strings after the table** — strings/vectors MUST be created before `Create{Table}()`
- **Sending from timer threads without defer** — causes uWS crashes; use `party->send_gamestate()` which handles this
- **Not skipping host in player population** — host is a Client too; always check `client->party->host == client`
- **`get_camel_case_name()` mismatch** — must exactly match frontend folder name and `add_minigames()` key

---
---

# Skill: Create Minigame Frontend

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
1. FlatBuffer schemas exist in `schemes/minigamedata/{GameName}/` — if not, create them first (use the create-minigame-schemas instructions above)
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

For games rendering a canvas with PixiJS, ALWAYS use the aspect-ratio-locked pattern that constrains by **both** width and height. Tracking only width causes the canvas to overflow its container on ultrawide screens (computed height exceeds available height).

```ts
// Canvas sizing — constrained by both container width AND height so the canvas
// always fits regardless of screen shape (narrow, normal, or ultrawide).
const gameCanvasRef = ref<HTMLElement | null>(null)
const containerWidth = ref(780)   // default fallback
const containerHeight = ref(440)  // default fallback
const MAP_ASPECT = 16 / 9         // GAME_WORLD_WIDTH / GAME_WORLD_HEIGHT

// canvasWidth is the smaller of: full container width, or height-derived width.
// This means on ultrawide screens the height becomes the constraint instead of width.
const canvasWidth = computed(() =>
  Math.min(containerWidth.value, Math.round(containerHeight.value * MAP_ASPECT))
)
const canvasHeight = computed(() => Math.round(canvasWidth.value / MAP_ASPECT))
const scale = computed(() => canvasWidth.value / GAME_WORLD_WIDTH)

let resizeObserver: ResizeObserver | null = null
watch(gameCanvasRef, (el) => {
  resizeObserver?.disconnect()
  if (el) {
    containerWidth.value = el.clientWidth
    containerHeight.value = el.clientHeight
    resizeObserver = new ResizeObserver((entries) => {
      containerWidth.value = entries[0].contentRect.width
      containerHeight.value = entries[0].contentRect.height
    })
    resizeObserver.observe(el)
  }
})
onUnmounted(() => {
  resizeObserver?.disconnect()
})
```

In the template, add `flex items-center justify-center` to the container so the canvas is centered when narrower than the container (e.g. ultrawide where height is the constraint):

```vue
<div ref="gameCanvasRef" class="bg-black rounded-lg overflow-hidden flex-1 flex items-center justify-center">
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

## Quick Reference — Frontend Code Style

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

## Common Frontend Pitfalls

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
