// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MESSAGECLASS_H_
#define FLATBUFFERS_GENERATED_MESSAGECLASS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

struct HostPayloadType;
struct HostPayloadTypeBuilder;

struct JoinPayloadType;
struct JoinPayloadTypeBuilder;

struct CrazyCountingHostEntitiesPayload;
struct CrazyCountingHostEntitiesPayloadBuilder;

struct FBCrazyCountingEntity;
struct FBCrazyCountingEntityBuilder;

struct CrazyCountingPlayerInputPayload;
struct CrazyCountingPlayerInputPayloadBuilder;

struct CrazyCountingPlayerIntPayload;
struct CrazyCountingPlayerIntPayloadBuilder;

struct MiniGamePayloadType;
struct MiniGamePayloadTypeBuilder;

struct Message;
struct MessageBuilder;

enum Input : int8_t {
  Input_Increase = 0,
  Input_Decrease = 1,
  Input_Submit = 2,
  Input_MIN = Input_Increase,
  Input_MAX = Input_Submit
};

inline const Input (&EnumValuesInput())[3] {
  static const Input values[] = {
    Input_Increase,
    Input_Decrease,
    Input_Submit
  };
  return values;
}

inline const char * const *EnumNamesInput() {
  static const char * const names[4] = {
    "Increase",
    "Decrease",
    "Submit",
    nullptr
  };
  return names;
}

inline const char *EnumNameInput(Input e) {
  if (::flatbuffers::IsOutRange(e, Input_Increase, Input_Submit)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesInput()[index];
}

enum GameStateType : int8_t {
  GameStateType_CrazyCountingHostEntities = 0,
  GameStateType_CrazyCountingPlayerInput = 1,
  GameStateType_CrazyCountingPlayerInt = 2,
  GameStateType_MIN = GameStateType_CrazyCountingHostEntities,
  GameStateType_MAX = GameStateType_CrazyCountingPlayerInt
};

inline const GameStateType (&EnumValuesGameStateType())[3] {
  static const GameStateType values[] = {
    GameStateType_CrazyCountingHostEntities,
    GameStateType_CrazyCountingPlayerInput,
    GameStateType_CrazyCountingPlayerInt
  };
  return values;
}

inline const char * const *EnumNamesGameStateType() {
  static const char * const names[4] = {
    "CrazyCountingHostEntities",
    "CrazyCountingPlayerInput",
    "CrazyCountingPlayerInt",
    nullptr
  };
  return names;
}

inline const char *EnumNameGameStateType(GameStateType e) {
  if (::flatbuffers::IsOutRange(e, GameStateType_CrazyCountingHostEntities, GameStateType_CrazyCountingPlayerInt)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesGameStateType()[index];
}

enum GameStatePayload : uint8_t {
  GameStatePayload_NONE = 0,
  GameStatePayload_CrazyCountingHostEntitiesPayload = 1,
  GameStatePayload_CrazyCountingPlayerInputPayload = 2,
  GameStatePayload_CrazyCountingPlayerIntPayload = 3,
  GameStatePayload_MIN = GameStatePayload_NONE,
  GameStatePayload_MAX = GameStatePayload_CrazyCountingPlayerIntPayload
};

inline const GameStatePayload (&EnumValuesGameStatePayload())[4] {
  static const GameStatePayload values[] = {
    GameStatePayload_NONE,
    GameStatePayload_CrazyCountingHostEntitiesPayload,
    GameStatePayload_CrazyCountingPlayerInputPayload,
    GameStatePayload_CrazyCountingPlayerIntPayload
  };
  return values;
}

inline const char * const *EnumNamesGameStatePayload() {
  static const char * const names[5] = {
    "NONE",
    "CrazyCountingHostEntitiesPayload",
    "CrazyCountingPlayerInputPayload",
    "CrazyCountingPlayerIntPayload",
    nullptr
  };
  return names;
}

inline const char *EnumNameGameStatePayload(GameStatePayload e) {
  if (::flatbuffers::IsOutRange(e, GameStatePayload_NONE, GameStatePayload_CrazyCountingPlayerIntPayload)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesGameStatePayload()[index];
}

template<typename T> struct GameStatePayloadTraits {
  static const GameStatePayload enum_value = GameStatePayload_NONE;
};

template<> struct GameStatePayloadTraits<CrazyCountingHostEntitiesPayload> {
  static const GameStatePayload enum_value = GameStatePayload_CrazyCountingHostEntitiesPayload;
};

template<> struct GameStatePayloadTraits<CrazyCountingPlayerInputPayload> {
  static const GameStatePayload enum_value = GameStatePayload_CrazyCountingPlayerInputPayload;
};

template<> struct GameStatePayloadTraits<CrazyCountingPlayerIntPayload> {
  static const GameStatePayload enum_value = GameStatePayload_CrazyCountingPlayerIntPayload;
};

bool VerifyGameStatePayload(::flatbuffers::Verifier &verifier, const void *obj, GameStatePayload type);
bool VerifyGameStatePayloadVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types);

enum MessageType : int8_t {
  MessageType_Host = 0,
  MessageType_Join = 1,
  MessageType_MiniGame = 2,
  MessageType_MIN = MessageType_Host,
  MessageType_MAX = MessageType_MiniGame
};

inline const MessageType (&EnumValuesMessageType())[3] {
  static const MessageType values[] = {
    MessageType_Host,
    MessageType_Join,
    MessageType_MiniGame
  };
  return values;
}

inline const char * const *EnumNamesMessageType() {
  static const char * const names[4] = {
    "Host",
    "Join",
    "MiniGame",
    nullptr
  };
  return names;
}

inline const char *EnumNameMessageType(MessageType e) {
  if (::flatbuffers::IsOutRange(e, MessageType_Host, MessageType_MiniGame)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMessageType()[index];
}

enum Payload : uint8_t {
  Payload_NONE = 0,
  Payload_HostPayloadType = 1,
  Payload_JoinPayloadType = 2,
  Payload_MiniGamePayloadType = 3,
  Payload_MIN = Payload_NONE,
  Payload_MAX = Payload_MiniGamePayloadType
};

inline const Payload (&EnumValuesPayload())[4] {
  static const Payload values[] = {
    Payload_NONE,
    Payload_HostPayloadType,
    Payload_JoinPayloadType,
    Payload_MiniGamePayloadType
  };
  return values;
}

inline const char * const *EnumNamesPayload() {
  static const char * const names[5] = {
    "NONE",
    "HostPayloadType",
    "JoinPayloadType",
    "MiniGamePayloadType",
    nullptr
  };
  return names;
}

inline const char *EnumNamePayload(Payload e) {
  if (::flatbuffers::IsOutRange(e, Payload_NONE, Payload_MiniGamePayloadType)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPayload()[index];
}

template<typename T> struct PayloadTraits {
  static const Payload enum_value = Payload_NONE;
};

template<> struct PayloadTraits<HostPayloadType> {
  static const Payload enum_value = Payload_HostPayloadType;
};

template<> struct PayloadTraits<JoinPayloadType> {
  static const Payload enum_value = Payload_JoinPayloadType;
};

template<> struct PayloadTraits<MiniGamePayloadType> {
  static const Payload enum_value = Payload_MiniGamePayloadType;
};

bool VerifyPayload(::flatbuffers::Verifier &verifier, const void *obj, Payload type);
bool VerifyPayloadVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types);

struct HostPayloadType FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef HostPayloadTypeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ROOM_ID = 4
  };
  int64_t room_id() const {
    return GetField<int64_t>(VT_ROOM_ID, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ROOM_ID, 8) &&
           verifier.EndTable();
  }
};

struct HostPayloadTypeBuilder {
  typedef HostPayloadType Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_room_id(int64_t room_id) {
    fbb_.AddElement<int64_t>(HostPayloadType::VT_ROOM_ID, room_id, 0);
  }
  explicit HostPayloadTypeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<HostPayloadType> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<HostPayloadType>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<HostPayloadType> CreateHostPayloadType(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int64_t room_id = 0) {
  HostPayloadTypeBuilder builder_(_fbb);
  builder_.add_room_id(room_id);
  return builder_.Finish();
}

struct JoinPayloadType FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef JoinPayloadTypeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SUCCESS = 4
  };
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS, 1) &&
           verifier.EndTable();
  }
};

struct JoinPayloadTypeBuilder {
  typedef JoinPayloadType Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(JoinPayloadType::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  explicit JoinPayloadTypeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<JoinPayloadType> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<JoinPayloadType>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<JoinPayloadType> CreateJoinPayloadType(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool success = false) {
  JoinPayloadTypeBuilder builder_(_fbb);
  builder_.add_success(success);
  return builder_.Finish();
}

struct CrazyCountingHostEntitiesPayload FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CrazyCountingHostEntitiesPayloadBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ENTITIES = 4
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<FBCrazyCountingEntity>> *entities() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<FBCrazyCountingEntity>> *>(VT_ENTITIES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_ENTITIES) &&
           verifier.VerifyVector(entities()) &&
           verifier.VerifyVectorOfTables(entities()) &&
           verifier.EndTable();
  }
};

struct CrazyCountingHostEntitiesPayloadBuilder {
  typedef CrazyCountingHostEntitiesPayload Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_entities(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<FBCrazyCountingEntity>>> entities) {
    fbb_.AddOffset(CrazyCountingHostEntitiesPayload::VT_ENTITIES, entities);
  }
  explicit CrazyCountingHostEntitiesPayloadBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<CrazyCountingHostEntitiesPayload> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<CrazyCountingHostEntitiesPayload>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<CrazyCountingHostEntitiesPayload> CreateCrazyCountingHostEntitiesPayload(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<FBCrazyCountingEntity>>> entities = 0) {
  CrazyCountingHostEntitiesPayloadBuilder builder_(_fbb);
  builder_.add_entities(entities);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<CrazyCountingHostEntitiesPayload> CreateCrazyCountingHostEntitiesPayloadDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<FBCrazyCountingEntity>> *entities = nullptr) {
  auto entities__ = entities ? _fbb.CreateVector<::flatbuffers::Offset<FBCrazyCountingEntity>>(*entities) : 0;
  return CreateCrazyCountingHostEntitiesPayload(
      _fbb,
      entities__);
}

struct FBCrazyCountingEntity FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FBCrazyCountingEntityBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_X_POS = 4,
    VT_Y_POS = 6
  };
  float x_pos() const {
    return GetField<float>(VT_X_POS, 0.0f);
  }
  float y_pos() const {
    return GetField<float>(VT_Y_POS, 0.0f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_X_POS, 4) &&
           VerifyField<float>(verifier, VT_Y_POS, 4) &&
           verifier.EndTable();
  }
};

struct FBCrazyCountingEntityBuilder {
  typedef FBCrazyCountingEntity Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_x_pos(float x_pos) {
    fbb_.AddElement<float>(FBCrazyCountingEntity::VT_X_POS, x_pos, 0.0f);
  }
  void add_y_pos(float y_pos) {
    fbb_.AddElement<float>(FBCrazyCountingEntity::VT_Y_POS, y_pos, 0.0f);
  }
  explicit FBCrazyCountingEntityBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FBCrazyCountingEntity> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FBCrazyCountingEntity>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FBCrazyCountingEntity> CreateFBCrazyCountingEntity(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    float x_pos = 0.0f,
    float y_pos = 0.0f) {
  FBCrazyCountingEntityBuilder builder_(_fbb);
  builder_.add_y_pos(y_pos);
  builder_.add_x_pos(x_pos);
  return builder_.Finish();
}

struct CrazyCountingPlayerInputPayload FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CrazyCountingPlayerInputPayloadBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INPUT_TYPE = 4
  };
  Input input_type() const {
    return static_cast<Input>(GetField<int8_t>(VT_INPUT_TYPE, 0));
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_INPUT_TYPE, 1) &&
           verifier.EndTable();
  }
};

struct CrazyCountingPlayerInputPayloadBuilder {
  typedef CrazyCountingPlayerInputPayload Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_input_type(Input input_type) {
    fbb_.AddElement<int8_t>(CrazyCountingPlayerInputPayload::VT_INPUT_TYPE, static_cast<int8_t>(input_type), 0);
  }
  explicit CrazyCountingPlayerInputPayloadBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<CrazyCountingPlayerInputPayload> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<CrazyCountingPlayerInputPayload>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<CrazyCountingPlayerInputPayload> CreateCrazyCountingPlayerInputPayload(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    Input input_type = Input_Increase) {
  CrazyCountingPlayerInputPayloadBuilder builder_(_fbb);
  builder_.add_input_type(input_type);
  return builder_.Finish();
}

struct CrazyCountingPlayerIntPayload FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CrazyCountingPlayerIntPayloadBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NEW_INT = 4
  };
  int16_t new_int() const {
    return GetField<int16_t>(VT_NEW_INT, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int16_t>(verifier, VT_NEW_INT, 2) &&
           verifier.EndTable();
  }
};

struct CrazyCountingPlayerIntPayloadBuilder {
  typedef CrazyCountingPlayerIntPayload Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_new_int(int16_t new_int) {
    fbb_.AddElement<int16_t>(CrazyCountingPlayerIntPayload::VT_NEW_INT, new_int, 0);
  }
  explicit CrazyCountingPlayerIntPayloadBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<CrazyCountingPlayerIntPayload> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<CrazyCountingPlayerIntPayload>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<CrazyCountingPlayerIntPayload> CreateCrazyCountingPlayerIntPayload(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int16_t new_int = 0) {
  CrazyCountingPlayerIntPayloadBuilder builder_(_fbb);
  builder_.add_new_int(new_int);
  return builder_.Finish();
}

struct MiniGamePayloadType FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef MiniGamePayloadTypeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_GAMESTATETYPE = 4,
    VT_GAMESTATEPAYLOAD_TYPE = 6,
    VT_GAMESTATEPAYLOAD = 8
  };
  GameStateType gamestatetype() const {
    return static_cast<GameStateType>(GetField<int8_t>(VT_GAMESTATETYPE, 0));
  }
  GameStatePayload gamestatepayload_type() const {
    return static_cast<GameStatePayload>(GetField<uint8_t>(VT_GAMESTATEPAYLOAD_TYPE, 0));
  }
  const void *gamestatepayload() const {
    return GetPointer<const void *>(VT_GAMESTATEPAYLOAD);
  }
  template<typename T> const T *gamestatepayload_as() const;
  const CrazyCountingHostEntitiesPayload *gamestatepayload_as_CrazyCountingHostEntitiesPayload() const {
    return gamestatepayload_type() == GameStatePayload_CrazyCountingHostEntitiesPayload ? static_cast<const CrazyCountingHostEntitiesPayload *>(gamestatepayload()) : nullptr;
  }
  const CrazyCountingPlayerInputPayload *gamestatepayload_as_CrazyCountingPlayerInputPayload() const {
    return gamestatepayload_type() == GameStatePayload_CrazyCountingPlayerInputPayload ? static_cast<const CrazyCountingPlayerInputPayload *>(gamestatepayload()) : nullptr;
  }
  const CrazyCountingPlayerIntPayload *gamestatepayload_as_CrazyCountingPlayerIntPayload() const {
    return gamestatepayload_type() == GameStatePayload_CrazyCountingPlayerIntPayload ? static_cast<const CrazyCountingPlayerIntPayload *>(gamestatepayload()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_GAMESTATETYPE, 1) &&
           VerifyField<uint8_t>(verifier, VT_GAMESTATEPAYLOAD_TYPE, 1) &&
           VerifyOffset(verifier, VT_GAMESTATEPAYLOAD) &&
           VerifyGameStatePayload(verifier, gamestatepayload(), gamestatepayload_type()) &&
           verifier.EndTable();
  }
};

template<> inline const CrazyCountingHostEntitiesPayload *MiniGamePayloadType::gamestatepayload_as<CrazyCountingHostEntitiesPayload>() const {
  return gamestatepayload_as_CrazyCountingHostEntitiesPayload();
}

template<> inline const CrazyCountingPlayerInputPayload *MiniGamePayloadType::gamestatepayload_as<CrazyCountingPlayerInputPayload>() const {
  return gamestatepayload_as_CrazyCountingPlayerInputPayload();
}

template<> inline const CrazyCountingPlayerIntPayload *MiniGamePayloadType::gamestatepayload_as<CrazyCountingPlayerIntPayload>() const {
  return gamestatepayload_as_CrazyCountingPlayerIntPayload();
}

struct MiniGamePayloadTypeBuilder {
  typedef MiniGamePayloadType Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_gamestatetype(GameStateType gamestatetype) {
    fbb_.AddElement<int8_t>(MiniGamePayloadType::VT_GAMESTATETYPE, static_cast<int8_t>(gamestatetype), 0);
  }
  void add_gamestatepayload_type(GameStatePayload gamestatepayload_type) {
    fbb_.AddElement<uint8_t>(MiniGamePayloadType::VT_GAMESTATEPAYLOAD_TYPE, static_cast<uint8_t>(gamestatepayload_type), 0);
  }
  void add_gamestatepayload(::flatbuffers::Offset<void> gamestatepayload) {
    fbb_.AddOffset(MiniGamePayloadType::VT_GAMESTATEPAYLOAD, gamestatepayload);
  }
  explicit MiniGamePayloadTypeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<MiniGamePayloadType> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<MiniGamePayloadType>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<MiniGamePayloadType> CreateMiniGamePayloadType(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    GameStateType gamestatetype = GameStateType_CrazyCountingHostEntities,
    GameStatePayload gamestatepayload_type = GameStatePayload_NONE,
    ::flatbuffers::Offset<void> gamestatepayload = 0) {
  MiniGamePayloadTypeBuilder builder_(_fbb);
  builder_.add_gamestatepayload(gamestatepayload);
  builder_.add_gamestatepayload_type(gamestatepayload_type);
  builder_.add_gamestatetype(gamestatetype);
  return builder_.Finish();
}

struct Message FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef MessageBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TYPE = 4,
    VT_PAYLOAD_TYPE = 6,
    VT_PAYLOAD = 8
  };
  MessageType type() const {
    return static_cast<MessageType>(GetField<int8_t>(VT_TYPE, 0));
  }
  Payload payload_type() const {
    return static_cast<Payload>(GetField<uint8_t>(VT_PAYLOAD_TYPE, 0));
  }
  const void *payload() const {
    return GetPointer<const void *>(VT_PAYLOAD);
  }
  template<typename T> const T *payload_as() const;
  const HostPayloadType *payload_as_HostPayloadType() const {
    return payload_type() == Payload_HostPayloadType ? static_cast<const HostPayloadType *>(payload()) : nullptr;
  }
  const JoinPayloadType *payload_as_JoinPayloadType() const {
    return payload_type() == Payload_JoinPayloadType ? static_cast<const JoinPayloadType *>(payload()) : nullptr;
  }
  const MiniGamePayloadType *payload_as_MiniGamePayloadType() const {
    return payload_type() == Payload_MiniGamePayloadType ? static_cast<const MiniGamePayloadType *>(payload()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_TYPE, 1) &&
           VerifyField<uint8_t>(verifier, VT_PAYLOAD_TYPE, 1) &&
           VerifyOffset(verifier, VT_PAYLOAD) &&
           VerifyPayload(verifier, payload(), payload_type()) &&
           verifier.EndTable();
  }
};

template<> inline const HostPayloadType *Message::payload_as<HostPayloadType>() const {
  return payload_as_HostPayloadType();
}

template<> inline const JoinPayloadType *Message::payload_as<JoinPayloadType>() const {
  return payload_as_JoinPayloadType();
}

template<> inline const MiniGamePayloadType *Message::payload_as<MiniGamePayloadType>() const {
  return payload_as_MiniGamePayloadType();
}

struct MessageBuilder {
  typedef Message Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_type(MessageType type) {
    fbb_.AddElement<int8_t>(Message::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_payload_type(Payload payload_type) {
    fbb_.AddElement<uint8_t>(Message::VT_PAYLOAD_TYPE, static_cast<uint8_t>(payload_type), 0);
  }
  void add_payload(::flatbuffers::Offset<void> payload) {
    fbb_.AddOffset(Message::VT_PAYLOAD, payload);
  }
  explicit MessageBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Message> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Message>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Message> CreateMessage(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    MessageType type = MessageType_Host,
    Payload payload_type = Payload_NONE,
    ::flatbuffers::Offset<void> payload = 0) {
  MessageBuilder builder_(_fbb);
  builder_.add_payload(payload);
  builder_.add_payload_type(payload_type);
  builder_.add_type(type);
  return builder_.Finish();
}

inline bool VerifyGameStatePayload(::flatbuffers::Verifier &verifier, const void *obj, GameStatePayload type) {
  switch (type) {
    case GameStatePayload_NONE: {
      return true;
    }
    case GameStatePayload_CrazyCountingHostEntitiesPayload: {
      auto ptr = reinterpret_cast<const CrazyCountingHostEntitiesPayload *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case GameStatePayload_CrazyCountingPlayerInputPayload: {
      auto ptr = reinterpret_cast<const CrazyCountingPlayerInputPayload *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case GameStatePayload_CrazyCountingPlayerIntPayload: {
      auto ptr = reinterpret_cast<const CrazyCountingPlayerIntPayload *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyGameStatePayloadVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyGameStatePayload(
        verifier,  values->Get(i), types->GetEnum<GameStatePayload>(i))) {
      return false;
    }
  }
  return true;
}

inline bool VerifyPayload(::flatbuffers::Verifier &verifier, const void *obj, Payload type) {
  switch (type) {
    case Payload_NONE: {
      return true;
    }
    case Payload_HostPayloadType: {
      auto ptr = reinterpret_cast<const HostPayloadType *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Payload_JoinPayloadType: {
      auto ptr = reinterpret_cast<const JoinPayloadType *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Payload_MiniGamePayloadType: {
      auto ptr = reinterpret_cast<const MiniGamePayloadType *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyPayloadVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyPayload(
        verifier,  values->Get(i), types->GetEnum<Payload>(i))) {
      return false;
    }
  }
  return true;
}

inline const Message *GetMessage(const void *buf) {
  return ::flatbuffers::GetRoot<Message>(buf);
}

inline const Message *GetSizePrefixedMessage(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<Message>(buf);
}

inline bool VerifyMessageBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Message>(nullptr);
}

inline void FinishMessageBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Message> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Message> root) {
  fbb.FinishSizePrefixed(root);
}

#endif  // FLATBUFFERS_GENERATED_MESSAGECLASS_H_
