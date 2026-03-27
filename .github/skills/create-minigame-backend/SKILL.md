---
name: create-minigame-backend
description: "Implement the C++ backend for a new PeopleParty minigame. Use when: creating minigame backend code, implementing game logic in C++, adding a new game to the backend, wiring a minigame into the game loop, or working on backend minigame lifecycle/physics/timers."
---

# Create Minigame Backend

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
1. FlatBuffer schemas exist in `schemes/minigamedata/{GameName}/` — if not, create them first (use the `create-minigame-schemas` skill)
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
    // auto someVector = builder.CreateVector(data);

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

## Quick Reference — Code Style

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

## Common Pitfalls

- **Forgetting to clear timers in destructor** — causes crashes from dangling `this` captures
- **Forgetting to wire in `game.h` / `game.cpp`** — game compiles but never appears in the queue
- **Wrong `GameStateType_` enum values** — always read `messageClass_generated.h` after schema build, never guess
- **Creating FlatBuffer strings after the table** — strings/vectors MUST be created before `Create{Table}()`
- **Sending from timer threads without defer** — causes uWS crashes; use `party->send_gamestate()` which handles this
- **Not skipping host in player population** — host is a Client too; always check `client->party->host == client`
- **`get_camel_case_name()` mismatch** — must exactly match frontend folder name and `add_minigames()` key
