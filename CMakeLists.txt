cmake_minimum_required(VERSION 3.20)
project(CSystemsEngineering C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

function(add_cse_lab target source_file)
	add_executable(${target} ${source_file})
	target_compile_options(
		${target}
		PRIVATE
			-Wall
			-Wextra
			-Wpedantic
	)
endfunction()

add_cse_lab(
	cse1_hex_binary_inspector_starter
	"CSE1-Hex-and-Binary-Inspector/starter/main.c"
)
add_cse_lab(
	cse1_hex_binary_inspector_solution
	"CSE1-Hex-and-Binary-Inspector/solution/main.c"
)

add_cse_lab(
	cse2_bitflag_parser_starter
	"CSE2-Bitflag-Configuration-Parser/starter/main.c"
)
add_cse_lab(
	cse2_bitflag_parser_solution
	"CSE2-Bitflag-Configuration-Parser/solution/main.c"
)

add_cse_lab(
	cse3_xor_encoder_starter
	"CSE3-XOR-Encoder-Decoder/starter/main.c"
)
add_cse_lab(
	cse3_xor_encoder_solution
	"CSE3-XOR-Encoder-Decoder/solution/main.c"
)

add_cse_lab(
	cse4_memory_visualizer_starter
	"CSE4-Memory-Visualizer/starter/main.c"
)
add_cse_lab(
	cse4_memory_visualizer_solution
	"CSE4-Memory-Visualizer/solution/main.c"
)

add_cse_lab(
	cse5_byte_buffer_workbench_starter
	"CSE5-Byte-Buffer-Workbench/starter/main.c"
)
add_cse_lab(
	cse5_byte_buffer_workbench_solution
	"CSE5-Byte-Buffer-Workbench/solution/main.c"
)

add_cse_lab(
	cse6_packet_serializer_starter
	"CSE6-Packet-Serializer-Deserializer/starter/main.c"
)
add_cse_lab(
	cse6_packet_serializer_solution
	"CSE6-Packet-Serializer-Deserializer/solution/main.c"
)

add_cse_lab(
	cse7_dynamic_ring_buffer_starter
	"CSE7-Dynamic-Ring-Buffer/starter/main.c"
)
add_cse_lab(
	cse7_dynamic_ring_buffer_solution
	"CSE7-Dynamic-Ring-Buffer/solution/main.c"
)

add_cse_lab(
	cse8_fixed_log_reader_starter
	"CSE8-Fixed-Size-Log-File-Reader/starter/main.c"
)
add_cse_lab(
	cse8_fixed_log_reader_solution
	"CSE8-Fixed-Size-Log-File-Reader/solution/main.c"
)

add_cse_lab(
	cse9_capstone_transform_starter
	"CSE9-Capstone-Telemetry-Transform-CLI/starter/main.c"
)
add_cse_lab(
	cse9_capstone_transform_solution
	"CSE9-Capstone-Telemetry-Transform-CLI/solution/main.c"
)
