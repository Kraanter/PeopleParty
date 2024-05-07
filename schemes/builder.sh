echo "Building the main schema file..."

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
