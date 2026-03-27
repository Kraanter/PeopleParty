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
