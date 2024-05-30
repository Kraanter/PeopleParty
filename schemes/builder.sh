echo "Building the main schema file..."

# ======================================================
# First build the MiniGamePayloadType according to the different minigame GameStates
# ======================================================

GameStatesTypesDir="schemes/minigamedata"
GameStateTypeOutputFile="schemes/payloadtypes/MiniGamePayloadType.fbs"
GameStatePayloadTypes=()

rm -f "$GameStateTypeOutputFile"

# Write the header of the main schema file
echo "// Automatically generated main schema file" >> "$GameStateTypeOutputFile"
echo "// Include all gamestate types to build the MiniGamePayloadType scheme" >> "$GameStateTypeOutputFile"
echo "" >> "$GameStateTypeOutputFile"

# Loop through all files in the GameStates directory and include them in the main schema
# also loop recursively through the subdirectories
while IFS= read -r -d '' fbs_file; do
    payload_type=$(basename "$fbs_file" .fbs)
    folder_name=$(basename "$(dirname "$fbs_file")")
    GameStatePayloadTypes+=( "$payload_type" )
	if [ $folder_name == "minigamedata" ]; then
		folder_name=""
	else
		folder_name=$folder_name"/"
	fi
    echo "include \"minigamedata/$folder_name$payload_type.fbs\";" >> "$GameStateTypeOutputFile"
done < <(find "$GameStatesTypesDir" -type f -name '*.fbs' -print0)

# Write GameStateType enum
echo "" >> "$GameStateTypeOutputFile"
echo "enum GameStateType: byte {" >> "$GameStateTypeOutputFile"
for type in ${GameStatePayloadTypes[@]}; do
	gamestateType=${type//"Payload"/""}
	echo "    $gamestateType," >> "$GameStateTypeOutputFile"
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
echo "table MiniGamePayloadType {" >> "$GameStateTypeOutputFile"
echo "    minigame: string;" >> "$GameStateTypeOutputFile"
echo "    gamestatetype: GameStateType;" >> "$GameStateTypeOutputFile"
echo "    gamestatepayload: GameStatePayload;" >> "$GameStateTypeOutputFile"
echo "}" >> "$GameStateTypeOutputFile"
echo "" >> "$GameStateTypeOutputFile"
echo "root_type MiniGamePayloadType;" >> "$GameStateTypeOutputFile"

# ======================================================
# build the main scheme file
# ======================================================

PayloadTypesDir="schemes/payloadtypes"
OutputFile="schemes/messageClass.fbs"
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