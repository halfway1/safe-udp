set(file
  data_segment.cpp
  packet_statistics.cpp
  sliding_window.cpp
  udp_server.cpp
  udp_client.cpp
  )

add_library(udp_transport SHARED ${file})
target_link_libraries(udp_transport  glog)

# 将名为 udp_transport 的构建目标安装到项目的二进制目录下的 lib 子目录中
install(TARGETS  udp_transport DESTINATION  ${PROJECT_BINARY_DIR}/lib)




