---
name: create-minigame-schemas
description: "Create FlatBuffer schema (.fbs) files for a new PeopleParty minigame. Use when: adding a new minigame, creating .fbs schemas, adding FlatBuffer payloads for a game, or setting up minigame data definitions under schemes/minigamedata/."
---

# Create Minigame Schemas

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
