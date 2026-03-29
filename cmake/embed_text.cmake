# Embeds a text file as a C char array + length variable
# Usage: cmake -DINPUT_FILE=... -DOUTPUT_FILE=... -DARRAY_NAME=... -P embed_text.cmake

file(READ "${INPUT_FILE}" CONTENT)
string(LENGTH "${CONTENT}" CONTENT_LEN)

file(WRITE "${OUTPUT_FILE}" "// Auto-generated from ${INPUT_FILE}\n")
file(APPEND "${OUTPUT_FILE}" "const char ${ARRAY_NAME}[] = \n")

# Escape for C string literal
string(REPLACE "\\" "\\\\" CONTENT "${CONTENT}")
string(REPLACE "\"" "\\\"" CONTENT "${CONTENT}")
string(REPLACE "\n" "\\n\"\n\"" CONTENT "${CONTENT}")

file(APPEND "${OUTPUT_FILE}" "\"${CONTENT}\";\n")
file(APPEND "${OUTPUT_FILE}" "const unsigned int ${ARRAY_NAME}_len = ${CONTENT_LEN};\n")
