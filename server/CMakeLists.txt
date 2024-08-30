add_executable(server server.c redis_utils.c thread_pool.c log_utils.c task_queue.c)
target_link_libraries(server ${OPENSSL_LIBRARIES} ${HIREDIS_LIBRARIES} common)
