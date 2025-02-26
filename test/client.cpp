#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

#include "udp_client.h"

#include <glog/logging.h>

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = google::GLOG_INFO;

  LOG(INFO) << "Starting the client !!!";
  if (argc != 7) {
    LOG(ERROR) << "Please provide format: <server-ip> <server-port> "
                  "<file-name> <receiver-window> <control-param> <drop/delay%>";
    exit(1);
  }

  safe_udp::UdpClient *udp_client = new safe_udp::UdpClient();
  std::string server_ip(argv[1]);
  std::string port_num(argv[2]);
  std::string file_name(argv[3]);
  udp_client->receiver_window_ = atoi(argv[4]);

  // 在网络通信和传输协议的研究、开发以及测试过程中，模拟丢包和时延是一种常见的实践
  int control_param = atoi(argv[5]);
  LOG(INFO) << "control_param: " << control_param;
  if (control_param == 0) { // 0 禁止丢包时延
    udp_client->is_delay_ = false;
    udp_client->is_packet_drop_ = false;
  } else if (control_param == 1) { // 1开启丢包
    udp_client->is_packet_drop_ = true;
    udp_client->is_delay_ = false;
  } else if (control_param == 2) { // 2开启时延 时延（Latency），也称为延迟，是指数据从源端传输到目的端所需要的时间
    udp_client->is_packet_drop_ = false;
    udp_client->is_delay_ = true;
  } else if (control_param == 3) { // 3开启丢包时延
    udp_client->is_packet_drop_ = true;
    udp_client->is_delay_ = true;
  } else {
    LOG(ERROR) << "Invalid argument, should be range in 0-3 !!!";
    return 0;
  }

  int drop_percentage = atoi(argv[6]);
  udp_client->prob_value_ = drop_percentage;

  udp_client->CreateSocketAndServerConnection(server_ip, port_num);
  udp_client->SendFileRequest(file_name);

  free(udp_client);
  return 0;
}