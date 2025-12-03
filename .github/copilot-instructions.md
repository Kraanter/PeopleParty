# People Party - AI Coding Agent Instructions

## Project Overview
People Party is a multiplayer party game with a **C++ WebSocket backend** and **Vue 3 + TypeScript frontend**, designed for big-screen gameplay with mobile controllers. The architecture uses **FlatBuffers** for efficient binary serialization between client and server.

## Architecture & Communication

### FlatBuffers Schema System
- **Schema definitions** live in `schemes/` directory with auto-generation via `builder.sh`
- Run `npm run build` (or OS-specific variants) to regenerate schemas after modifying `.fbs` files
- Generated files: `backend/src/flatbuffer/messageClass_generated.h` and `frontend/src/flatbuffers/*.ts`
- The build script automatically discovers minigame payloads from `schemes/minigamedata/` and builds unions

### Message Flow
```
Client (Vue) → WebSocket → Backend (C++) → Party → Game → GameState (PartyPrep/MiniGame/Leaderboard)
```

- WebSocket endpoints: `/host` (creates party) and `/join/:room/:name` (joins existing party)
- Party IDs are 4-digit numeric codes generated in `party.cpp`
- All messages use FlatBuffer `Message` wrapper with `MessageType` enum and `Payload` union
- Frontend WebSocket store: `frontend/src/stores/confettiStore.ts` (naming is legacy, manages all WS communication)

## Adding a New Minigame

### Backend (C++)
1. Create directory in `backend/src/minigames/{game_name}/`
2. Inherit from `MiniGame` base class (see `minigame.h`)
3. Implement required methods:
   - `start_introduction()` - 7-second intro with name/description
   - `start_minigame()` - main game logic
   - `start_result()` - show results and rankings
   - `getMinigameResult()` - return player placements for leaderboard
   - `process_input(const MiniGamePayloadType* payload, Client* from)` - handle player input
   - `get_display_name()`, `get_camel_case_name()`, `get_description()` - metadata
4. Add minigame to `Game::add_minigames()` queue in `game.cpp`
5. Register in `game.h` includes

### Frontend (Vue 3)
1. Create `frontend/src/components/{gameName}/HostView.vue` (big screen display)
2. Create `frontend/src/components/{gameName}/PlayerView.vue` (mobile controller)
3. Add case to `GameManager.vue` switch statement for both host and player views
4. Use `useWebSocketStore()` from `confettiStore.ts` to send/receive messages
5. Access WebSocket store methods: `sendMessage(uint8Array)`, `clientName`, `partyCode`

### FlatBuffer Schema
1. Create payload types in `schemes/minigamedata/{GameName}/` (e.g., `{GameName}HostPayload.fbs`, `{GameName}PlayerInputPayload.fbs`)
2. Schema builder automatically discovers and includes them in `MiniGamePayloadType` union
3. Each payload needs unique `GameStateType` enum value (auto-generated)
4. See existing games like `CrazyCounting/` for reference patterns

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
./PeopleParty-Backend  # macOS/Linux
```

**Frontend only:**
```bash
cd frontend
npm install
npm run dev  # Runs on 0.0.0.0 for mobile testing
```

**Schema regeneration required when:**
- Adding/modifying `.fbs` files in `schemes/payloadtypes/` or `schemes/minigamedata/`
- Run platform-specific build: `npm run build:macos` / `build:nix` / `build:windows`

### Testing
- Frontend: Cypress component tests (`npm run test:unit:dev`) and E2E (`npm run test:e2e:dev`)
- Backend: Doctest framework (currently disabled in CMakeLists.txt, see commented sections)

## Project Conventions

### Naming Patterns
- **Backend classes**: `{GameName}_MiniGame` (e.g., `CrazyCounting_MiniGame`)
- **Frontend components**: `{GameName}/HostView.vue` and `{GameName}/PlayerView.vue`
- **FlatBuffer types**: `{GameName}Payload.fbs`, suffixed with purpose (e.g., `PlayerInputPayload`, `HostPayload`)
- **Camel case** for minigame identifiers in messages (e.g., "crazyCounting")

### Key Dependencies
- **Backend**: uWebSockets (v20.71.0), FlatBuffers (v24.3.25), Box2D (v2.4.1), Conan for dep management
- **Frontend**: Vue 3, Pinia (state), Vue Router, PixiJS (rendering), Tailwind CSS, FlatBuffers.js

### Docker Setup
- Three services: `nginx` (proxy), `client` (frontend), `api` (backend)
- Dev config: `docker-compose-dev.yml` with volume mounts for hot-reload
- Production images published to GitHub Container Registry: `ghcr.io/kraanter/peopleparty*`

### Game State Machine
The `Game` class orchestrates three primary states:
1. **PartyPrep** - Lobby for selecting rounds/games
2. **MiniGame** - Active minigame (queued, randomly selected)
3. **Leaderboard** - Score display between games

State transitions use `Game::nextGameState<T>()` template pattern. After minigames, leaderboard updates with `Game::update_leaderboard()` using placement pairs.

### Mobile-Specific Handling
- Frontend auto-redirects mobile users to `/join` route (see `router/index.ts`)
- Orientation lock attempts portrait mode in `main.ts`
- Custom viewport height handling via `--vh` CSS variable for iOS Safari

## Important File Locations
- WebSocket handler: `backend/src/websocket.cpp`
- Party management: `backend/src/party.cpp` and `party_repository.cpp`
- Frontend WS store: `frontend/src/stores/confettiStore.ts`
- Main game loop: `backend/src/game.cpp`
- Minigame base class: `backend/src/minigames/minigame.h`
- Schema builder: `schemes/builder.sh`

## Common Gotchas
- Always rebuild FlatBuffer schemas after `.fbs` changes - errors are cryptic
- Party IDs must be numeric 4-digit codes (validation in `websocket.cpp`)
- WebSocket binary type must be `arraybuffer` (set in confettiStore.ts)
- Minigames must be added to `Game::add_minigames()` queue, not just created
- Frontend component names must match GameStateType enum values for routing in GameManager
