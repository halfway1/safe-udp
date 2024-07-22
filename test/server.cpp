#include <cstdlib>
#include <iostream>
#include <string>
#include <glog/logging.h>
// glog 是 Google 开发的一个高性能的 C++ 日志库

#include "udp_server.h"

constexpr char SERVER_FILE_PATH[] = "/work/files/server_files/";

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  // 初始化 Google 的日志库。参数 argv[0] 是程序的名称，通常是命令行参数的第一个元素
  FLAGS_logtostderr = true;
  // 设置一个全局标志，使所有日志信息都输出到标准错误（stderr）（终端）而不是文件中
  FLAGS_minloglevel = google::GLOG_INFO;
  // 设置日志记录的最低级别。google::GLOG_INFO 表示记录 INFO 级别及其以上级别的日志信息（即 INFO、WARNING、ERROR 和 FATAL）

  int sfd = 0;
  int port_num = 8080;
  int recv_window = 0;
  char *message_recv;
  if (argc < 3) {
    LOG(INFO) << "Please provide a port number and receive window";
    LOG(ERROR) << "Please provide format: <server-port> <receiver-window>";
    exit(1);
  }
  if (argv[1] != NULL) {
    port_num = atoi(argv[1]); // 字符串转换为 int 类型
  }
  if (argv[2] != NULL) {
    recv_window = atoi(argv[2]);
  }

  safe_udp::UdpServer *udp_server = new safe_udp::UdpServer();
  udp_server->rwnd_ = recv_window;
  sfd = udp_server->StartServer(port_num);
  message_recv = udp_server->GetRequest(sfd);
  // char cwd[1024];
  // if (getcwd(cwd, sizeof(cwd)) != NULL) {
  //   LOG(INFO) << "Current working directory: " << cwd;
  // } else {
  //   perror("getcwd() error");
  //   return 1;
  // }

  std::string file_name =
      std::string(SERVER_FILE_PATH) + std::string(message_recv);
      
  if (udp_server->OpenFile(file_name)) {
    udp_server->StartFileTransfer();
  } else {
    udp_server->SendError();
  }

  free(udp_server);
  return 0;
}
