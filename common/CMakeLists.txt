add_library(common STATIC file_utils.c hash_utils.c)
target_link_libraries(common ${OPENSSL_LIBRARIES})