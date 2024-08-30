add_executable(client client.c)
target_link_libraries(client ${OPENSSL_LIBRARIES})