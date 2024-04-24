echo "Building the main schema file..."

# ======================================================
# First build the GameStatePayloadType according to the different minigame GameStates
# ======================================================

GameStatesTypesDir="schemes/gamestates"
GameStateTypeOutputFile="schemes/payloadtypes/GameStatePayloadType.fbs"
GameStatePayloadTypes=()

rm -f "$GameStateTypeOutputFile"

# Write the header of the main schema file
echo "// Automatically generated main schema file" >> "$GameStateTypeOutputFile"
echo "// Include all gamestate types to build the GameStatePayloadType scheme" >> "$GameStateTypeOutputFile"
echo "" >> "$GameStateTypeOutputFile"

# Loop through all files in the GameStates directory and include them in the main schema
for fbs_file in "$GameStatesTypesDir"/*.fbs; do
    payload_type=$(basename "$fbs_file" .fbs)
	GameStatePayloadTypes+=( $payload_type )
    echo "include \"gamestates/$payload_type.fbs\";" >> "$GameStateTypeOutputFile"
done

# Write GameStateType enum
echo "" >> "$GameStateTypeOutputFile"
echo "enum GameStateType: byte {" >> "$GameStateTypeOutputFile"
for type in ${GameStatePayloadTypes[@]}; do
	echo "    $type," >> "$GameStateTypeOutputFile"
done
echo "}" >> "$GameStateTypeOutputFile"

# Write the union
echo "" >> "$GameStateTypeOutputFile"
echo "union GameStatePayload {" >> "$GameStateTypeOutputFile"
for type in ${GameStatePayloadTypes[@]}; do
	echo "    $type," >> "$GameStateTypeOutputFile"
done
echo "}" >> "$GameStateTypeOutputFile"

# Write the root type definition to the main schema
echo "" >> "$GameStateTypeOutputFile"
echo "table GameStatePayloadType {" >> "$GameStateTypeOutputFile"
echo "    gamestatetype: GameStateType;" >> "$GameStateTypeOutputFile"
echo "    gamestatepayload: GameStatePayload;" >> "$GameStateTypeOutputFile"
echo "}" >> "$GameStateTypeOutputFile"
echo "" >> "$GameStateTypeOutputFile"
echo "root_type GameStatePayloadType;" >> "$GameStateTypeOutputFile"

# ======================================================
# build the main scheme file
# ======================================================

PayloadTypesDir="schemes/payloadtypes"
OutputFile="schemes/message.fbs"
payloadTypes=()

rm -f "$OutputFile"

# Write the header of the main schema file
echo "// Automatically generated main schema file" >> "$OutputFile"
echo "// Include all payload types to build the scheme" >> "$OutputFile"
echo "" >> "$OutputFile"

# Loop through all files in the payloadTypes directory and include them in the main schema
for fbs_file in "$PayloadTypesDir"/*.fbs; do
    payload_type=$(basename "$fbs_file" .fbs)
	payloadTypes+=( $payload_type )
    echo "include \"payloadtypes/$payload_type.fbs\";" >> "$OutputFile"
done

# Write messageType enum
echo "" >> "$OutputFile"
echo "enum MessageType: byte {" >> "$OutputFile"
for type in ${payloadTypes[@]}; do
	messageType=${type//"PayloadType"/""}
	echo "    $messageType," >> "$OutputFile"
done
echo "}" >> "$OutputFile"

# Write the union
echo "" >> "$OutputFile"
echo "union Payload {" >> "$OutputFile"
for type in ${payloadTypes[@]}; do
	echo "    $type," >> "$OutputFile"
done
echo "}" >> "$OutputFile"

# Write the root type definition to the main schema
echo "" >> "$OutputFile"
echo "table Message {" >> "$OutputFile"
echo "    type: MessageType;" >> "$OutputFile"
echo "    payload: Payload;" >> "$OutputFile"
echo "}" >> "$OutputFile"
echo "" >> "$OutputFile"
echo "root_type Message;" >> "$OutputFile"
