// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.34.2
// 	protoc        v5.28.2
// source: game_com.proto

package protobuf

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type PROTO_GAME_UPDATE_REASON int32

const (
	PROTO_GAME_UPDATE_REASON_GAME_TICK    PROTO_GAME_UPDATE_REASON = 0
	PROTO_GAME_UPDATE_REASON_PLAYER_INPUT PROTO_GAME_UPDATE_REASON = 1
)

// Enum value maps for PROTO_GAME_UPDATE_REASON.
var (
	PROTO_GAME_UPDATE_REASON_name = map[int32]string{
		0: "GAME_TICK",
		1: "PLAYER_INPUT",
	}
	PROTO_GAME_UPDATE_REASON_value = map[string]int32{
		"GAME_TICK":    0,
		"PLAYER_INPUT": 1,
	}
)

func (x PROTO_GAME_UPDATE_REASON) Enum() *PROTO_GAME_UPDATE_REASON {
	p := new(PROTO_GAME_UPDATE_REASON)
	*p = x
	return p
}

func (x PROTO_GAME_UPDATE_REASON) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (PROTO_GAME_UPDATE_REASON) Descriptor() protoreflect.EnumDescriptor {
	return file_game_com_proto_enumTypes[0].Descriptor()
}

func (PROTO_GAME_UPDATE_REASON) Type() protoreflect.EnumType {
	return &file_game_com_proto_enumTypes[0]
}

func (x PROTO_GAME_UPDATE_REASON) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use PROTO_GAME_UPDATE_REASON.Descriptor instead.
func (PROTO_GAME_UPDATE_REASON) EnumDescriptor() ([]byte, []int) {
	return file_game_com_proto_rawDescGZIP(), []int{0}
}

type ProtoGameState struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Host    string            `protobuf:"bytes,1,opt,name=host,proto3" json:"host,omitempty"`
	Global  string            `protobuf:"bytes,2,opt,name=global,proto3" json:"global,omitempty"`
	Players map[string]string `protobuf:"bytes,3,rep,name=players,proto3" json:"players,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
}

func (x *ProtoGameState) Reset() {
	*x = ProtoGameState{}
	if protoimpl.UnsafeEnabled {
		mi := &file_game_com_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ProtoGameState) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ProtoGameState) ProtoMessage() {}

func (x *ProtoGameState) ProtoReflect() protoreflect.Message {
	mi := &file_game_com_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ProtoGameState.ProtoReflect.Descriptor instead.
func (*ProtoGameState) Descriptor() ([]byte, []int) {
	return file_game_com_proto_rawDescGZIP(), []int{0}
}

func (x *ProtoGameState) GetHost() string {
	if x != nil {
		return x.Host
	}
	return ""
}

func (x *ProtoGameState) GetGlobal() string {
	if x != nil {
		return x.Global
	}
	return ""
}

func (x *ProtoGameState) GetPlayers() map[string]string {
	if x != nil {
		return x.Players
	}
	return nil
}

type Proto_GameUpdate struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Reason     PROTO_GAME_UPDATE_REASON `protobuf:"varint,1,opt,name=reason,proto3,enum=PROTO_GAME_UPDATE_REASON" json:"reason,omitempty"`
	GameState  *ProtoGameState          `protobuf:"bytes,2,opt,name=gameState,proto3" json:"gameState,omitempty"`
	Payload    string                   `protobuf:"bytes,3,opt,name=payload,proto3" json:"payload,omitempty"`
	DeltaTime  int32                    `protobuf:"varint,4,opt,name=deltaTime,proto3" json:"deltaTime,omitempty"`
	Playername *string                  `protobuf:"bytes,5,opt,name=playername,proto3,oneof" json:"playername,omitempty"`
}

func (x *Proto_GameUpdate) Reset() {
	*x = Proto_GameUpdate{}
	if protoimpl.UnsafeEnabled {
		mi := &file_game_com_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Proto_GameUpdate) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Proto_GameUpdate) ProtoMessage() {}

func (x *Proto_GameUpdate) ProtoReflect() protoreflect.Message {
	mi := &file_game_com_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Proto_GameUpdate.ProtoReflect.Descriptor instead.
func (*Proto_GameUpdate) Descriptor() ([]byte, []int) {
	return file_game_com_proto_rawDescGZIP(), []int{1}
}

func (x *Proto_GameUpdate) GetReason() PROTO_GAME_UPDATE_REASON {
	if x != nil {
		return x.Reason
	}
	return PROTO_GAME_UPDATE_REASON_GAME_TICK
}

func (x *Proto_GameUpdate) GetGameState() *ProtoGameState {
	if x != nil {
		return x.GameState
	}
	return nil
}

func (x *Proto_GameUpdate) GetPayload() string {
	if x != nil {
		return x.Payload
	}
	return ""
}

func (x *Proto_GameUpdate) GetDeltaTime() int32 {
	if x != nil {
		return x.DeltaTime
	}
	return 0
}

func (x *Proto_GameUpdate) GetPlayername() string {
	if x != nil && x.Playername != nil {
		return *x.Playername
	}
	return ""
}

var File_game_com_proto protoreflect.FileDescriptor

var file_game_com_proto_rawDesc = []byte{
	0x0a, 0x0e, 0x67, 0x61, 0x6d, 0x65, 0x5f, 0x63, 0x6f, 0x6d, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x22, 0xb0, 0x01, 0x0a, 0x0e, 0x50, 0x72, 0x6f, 0x74, 0x6f, 0x47, 0x61, 0x6d, 0x65, 0x53, 0x74,
	0x61, 0x74, 0x65, 0x12, 0x12, 0x0a, 0x04, 0x68, 0x6f, 0x73, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28,
	0x09, 0x52, 0x04, 0x68, 0x6f, 0x73, 0x74, 0x12, 0x16, 0x0a, 0x06, 0x67, 0x6c, 0x6f, 0x62, 0x61,
	0x6c, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x06, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x12,
	0x36, 0x0a, 0x07, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x73, 0x18, 0x03, 0x20, 0x03, 0x28, 0x0b,
	0x32, 0x1c, 0x2e, 0x50, 0x72, 0x6f, 0x74, 0x6f, 0x47, 0x61, 0x6d, 0x65, 0x53, 0x74, 0x61, 0x74,
	0x65, 0x2e, 0x50, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x52, 0x07,
	0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x73, 0x1a, 0x3a, 0x0a, 0x0c, 0x50, 0x6c, 0x61, 0x79, 0x65,
	0x72, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x12, 0x10, 0x0a, 0x03, 0x6b, 0x65, 0x79, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x03, 0x6b, 0x65, 0x79, 0x12, 0x14, 0x0a, 0x05, 0x76, 0x61, 0x6c,
	0x75, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x05, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x3a,
	0x02, 0x38, 0x01, 0x22, 0xe0, 0x01, 0x0a, 0x10, 0x50, 0x72, 0x6f, 0x74, 0x6f, 0x5f, 0x47, 0x61,
	0x6d, 0x65, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 0x12, 0x31, 0x0a, 0x06, 0x72, 0x65, 0x61, 0x73,
	0x6f, 0x6e, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x19, 0x2e, 0x50, 0x52, 0x4f, 0x54, 0x4f,
	0x5f, 0x47, 0x41, 0x4d, 0x45, 0x5f, 0x55, 0x50, 0x44, 0x41, 0x54, 0x45, 0x5f, 0x52, 0x45, 0x41,
	0x53, 0x4f, 0x4e, 0x52, 0x06, 0x72, 0x65, 0x61, 0x73, 0x6f, 0x6e, 0x12, 0x2d, 0x0a, 0x09, 0x67,
	0x61, 0x6d, 0x65, 0x53, 0x74, 0x61, 0x74, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x0f,
	0x2e, 0x50, 0x72, 0x6f, 0x74, 0x6f, 0x47, 0x61, 0x6d, 0x65, 0x53, 0x74, 0x61, 0x74, 0x65, 0x52,
	0x09, 0x67, 0x61, 0x6d, 0x65, 0x53, 0x74, 0x61, 0x74, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x70, 0x61,
	0x79, 0x6c, 0x6f, 0x61, 0x64, 0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52, 0x07, 0x70, 0x61, 0x79,
	0x6c, 0x6f, 0x61, 0x64, 0x12, 0x1c, 0x0a, 0x09, 0x64, 0x65, 0x6c, 0x74, 0x61, 0x54, 0x69, 0x6d,
	0x65, 0x18, 0x04, 0x20, 0x01, 0x28, 0x05, 0x52, 0x09, 0x64, 0x65, 0x6c, 0x74, 0x61, 0x54, 0x69,
	0x6d, 0x65, 0x12, 0x23, 0x0a, 0x0a, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x65,
	0x18, 0x05, 0x20, 0x01, 0x28, 0x09, 0x48, 0x00, 0x52, 0x0a, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72,
	0x6e, 0x61, 0x6d, 0x65, 0x88, 0x01, 0x01, 0x42, 0x0d, 0x0a, 0x0b, 0x5f, 0x70, 0x6c, 0x61, 0x79,
	0x65, 0x72, 0x6e, 0x61, 0x6d, 0x65, 0x2a, 0x3b, 0x0a, 0x18, 0x50, 0x52, 0x4f, 0x54, 0x4f, 0x5f,
	0x47, 0x41, 0x4d, 0x45, 0x5f, 0x55, 0x50, 0x44, 0x41, 0x54, 0x45, 0x5f, 0x52, 0x45, 0x41, 0x53,
	0x4f, 0x4e, 0x12, 0x0d, 0x0a, 0x09, 0x47, 0x41, 0x4d, 0x45, 0x5f, 0x54, 0x49, 0x43, 0x4b, 0x10,
	0x00, 0x12, 0x10, 0x0a, 0x0c, 0x50, 0x4c, 0x41, 0x59, 0x45, 0x52, 0x5f, 0x49, 0x4e, 0x50, 0x55,
	0x54, 0x10, 0x01, 0x42, 0x11, 0x5a, 0x0f, 0x67, 0x6f, 0x74, 0x65, 0x73, 0x74, 0x2f, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x62, 0x75, 0x66, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_game_com_proto_rawDescOnce sync.Once
	file_game_com_proto_rawDescData = file_game_com_proto_rawDesc
)

func file_game_com_proto_rawDescGZIP() []byte {
	file_game_com_proto_rawDescOnce.Do(func() {
		file_game_com_proto_rawDescData = protoimpl.X.CompressGZIP(file_game_com_proto_rawDescData)
	})
	return file_game_com_proto_rawDescData
}

var file_game_com_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_game_com_proto_msgTypes = make([]protoimpl.MessageInfo, 3)
var file_game_com_proto_goTypes = []any{
	(PROTO_GAME_UPDATE_REASON)(0), // 0: PROTO_GAME_UPDATE_REASON
	(*ProtoGameState)(nil),        // 1: ProtoGameState
	(*Proto_GameUpdate)(nil),      // 2: Proto_GameUpdate
	nil,                           // 3: ProtoGameState.PlayersEntry
}
var file_game_com_proto_depIdxs = []int32{
	3, // 0: ProtoGameState.players:type_name -> ProtoGameState.PlayersEntry
	0, // 1: Proto_GameUpdate.reason:type_name -> PROTO_GAME_UPDATE_REASON
	1, // 2: Proto_GameUpdate.gameState:type_name -> ProtoGameState
	3, // [3:3] is the sub-list for method output_type
	3, // [3:3] is the sub-list for method input_type
	3, // [3:3] is the sub-list for extension type_name
	3, // [3:3] is the sub-list for extension extendee
	0, // [0:3] is the sub-list for field type_name
}

func init() { file_game_com_proto_init() }
func file_game_com_proto_init() {
	if File_game_com_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_game_com_proto_msgTypes[0].Exporter = func(v any, i int) any {
			switch v := v.(*ProtoGameState); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_game_com_proto_msgTypes[1].Exporter = func(v any, i int) any {
			switch v := v.(*Proto_GameUpdate); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	file_game_com_proto_msgTypes[1].OneofWrappers = []any{}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_game_com_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   3,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_game_com_proto_goTypes,
		DependencyIndexes: file_game_com_proto_depIdxs,
		EnumInfos:         file_game_com_proto_enumTypes,
		MessageInfos:      file_game_com_proto_msgTypes,
	}.Build()
	File_game_com_proto = out.File
	file_game_com_proto_rawDesc = nil
	file_game_com_proto_goTypes = nil
	file_game_com_proto_depIdxs = nil
}
