include(FindProtobuf)

set(PROTOS # cmake-format: sortable
           echo.proto
)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${PROTOS})

message(STATUS "Generated PROTO with SERVER PROTO SRC: ${PROTO_SRC}\n")
message(STATUS "Generated PROTO with SERVER PROTO HEADER: ${PROTO_HEADER}\n")

add_library(pb_obj OBJECT)
target_sources(
  pb_obj
  PUBLIC ${PROTO_HEADER}
  PRIVATE ${PROTO_SRC}
)
