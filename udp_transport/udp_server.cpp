#include "udp_server.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>
#include <glog/logging.h>

namespace safe_udp {
UdpServer::UdpServer() {
  sliding_window_ = std::make_unique<SlidingWindow>();
  packet_statistics_ = std::make_unique<PacketStatistics>();
  // std::make_unique 是 C++11 引入的用于创建 std::unique_ptr 的工厂函数。
  // sliding_window_ 和 packet_statistics_ 都是 std::unique_ptr 类型的成员变量。
  // std::make_unique<SlidingWindow>() 和 std::make_unique<PacketStatistics>() 分别创建一个指向 SlidingWindow 和 PacketStatistics 对象的独占智能指针。

  sockfd_ = 0;
  smoothed_rtt_ = 20000;
  smoothed_timeout_ = 30000;
  dev_rtt_ = 0;

  initial_seq_number_ = 67; // 随机值
  start_byte_ = 0;

  ssthresh_ = 128;
  cwnd_ = 1;

  is_slow_start_ = true;
  is_cong_avd_ = false;
  is_fast_recovery_ = false;
}

int UdpServer::StartServer(int port) {
  int sfd;
  struct sockaddr_in server_addr;
  LOG(INFO) << "Starting the webserver... port: " << port;
  sfd = socket(AF_INET, SOCK_DGRAM, 0);                                               // socket

  if (sfd < 0) {
    LOG(ERROR) << " Failed to socket !!!";
    exit(0);
  }

  memset(&server_addr, 0, sizeof(server_addr)); // 将 server_addr 结构体的内存全部设置为 0
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(port); //htons(port) 函数将主机字节序的端口号转换为网络字节序

  if (bind(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {           // bind
    LOG(ERROR) << "binding error !!!";
    exit(0);
  }
  LOG(INFO) << "**Server Bind set to addr: " << server_addr.sin_addr.s_addr;
  LOG(INFO) << "**Server Bind set to port: " << server_addr.sin_port;
  LOG(INFO) << "**Server Bind set to family: " << server_addr.sin_family;
  LOG(INFO) << "Started successfully";
  sockfd_ = sfd;
  return sfd;
}

bool UdpServer::OpenFile(const std::string &file_name) {
  LOG(INFO) << "Opening the file " << file_name;

  file_.open(file_name.c_str(), std::ios::in); // 以读取方式打开文件。这是 std::ios 命名空间中定义的一个标志，表示打开文件以供读取
  // file_name.c_str() 返回一个 const char* 指针，指向存储文件名的 C 风格字符串

  if (!this->file_.is_open()) {
    LOG(INFO) << "File: " << file_name << " opening failed";
    return false;
  } else {
    LOG(INFO) << "File: " << file_name << " opening success";
    return true;
  }
}

void UdpServer::StartFileTransfer() {
  LOG(INFO) << "Starting the file_ transfer ";

  file_.seekg(0, std::ios::end); // 将文件流 file_ 定位到文件末尾
  file_length_ = file_.tellg(); // 获取当前文件指针的位置，即文件的大小
  file_.seekg(0, std::ios::beg); // 将文件流 file_ 的文件指针重新定位到文件开头

  send();
}

void UdpServer::SendError() {
  std::string error("FILE NOT FOUND");
  sendto(sockfd_, error.c_str(), error.size(), 0,
         (struct sockaddr *)&cli_address_, sizeof(cli_address_));
}

void UdpServer::send() {
  LOG(INFO) << "Entering Send()";

  int sent_count = 1;
  int sent_count_limit = 1;

  struct timeval process_start_time;
  struct timeval process_end_time;
  // struct timeval：这是一个结构体，用于存储时间值，通常在 C 和 C++ 中用于精确地表示时间点或时间间隔
  // process_start_time 和 process_end_time：这是两个 struct timeval 类型的变量，用来存储程序开始和结束的时间
  gettimeofday(&process_start_time, NULL);
  //gettimeofday：这是一个 POSIX 标准中定义的函数，用于获取当前时间。
  //第一个参数 &process_start_time 是一个指向 struct timeval 结构体的指针，gettimeofday 函数会将当前的时间信息填充到这个结构体中。
  //第二个参数 NULL 是一个指向时区信息的指针，如果不需要获取时区信息，可以传入 NULL。

  if (sliding_window_->last_packet_sent_ == -1) {
    start_byte_ = 0;
  }

  while (start_byte_ <= file_length_) {
    fd_set rfds;
    struct timeval tv;
    int res;

    sent_count = 1;
    sent_count_limit = std::min(rwnd_, cwnd_); 
    // sent_count_limit 是接收窗口和拥塞窗口的较小值，确保发送的数据包数量既不会超过接收方的接收能力，也不会导致网络拥塞

    LOG(INFO) << "SEND START  !!!!";
    LOG(INFO) << "Before the window rwnd_: " << rwnd_ << " cwnd_: " << cwnd_
              << " window used: "
              << sliding_window_->last_packet_sent_ - sliding_window_->last_acked_packet_; //已发送但尚未确认的数据包数量

    while (sliding_window_->last_packet_sent_ - sliding_window_->last_acked_packet_ <=std::min(rwnd_, cwnd_) 
          && sent_count <= sent_count_limit) { // sent_count <= sent_count_limit：确保发送次数不超过设定的限制
      send_packet(start_byte_ + initial_seq_number_, start_byte_);

      if (is_slow_start_) {
        // LOG(INFO) << "Window In SLOW START Window";
        packet_statistics_->slow_start_packet_sent_count_++;
      } else if (is_cong_avd_) { // 拥塞避免
        // LOG(INFO) << "Window In CONG AVD Window";
        packet_statistics_->cong_avd_packet_sent_count_++;
      }

      // break
      start_byte_ = start_byte_ + MAX_DATA_SIZE;
      if (start_byte_ > file_length_) {
        LOG(INFO) << "No more data left to be sent";
        break;
      }
      sent_count++;
    }

    LOG(INFO) << "SEND END !!!!!";

    // socket listen whith timeout
    FD_ZERO(&rfds);  // 将文件描述符集合 rfds 清空，确保在使用之前没有任何文件描述符被设置
    FD_SET(sockfd_, &rfds); //将 sockfd_ 对应的文件描述符添加到 rfds 文件描述符集合中
    // 这两行代码通常用于准备使用 select 函数来进行文件描述符的多路复用。在调用 select 前，需要清空并设置待检查的文件描述符集合

    tv.tv_sec = (int64_t)smoothed_timeout_ / 1000000;
    tv.tv_usec = smoothed_timeout_ - tv.tv_sec;
    // 这段代码用于设置 select 函数的超时时间。select 函数用于等待文件描述符集合中的一个或多个文件描述符变为可读、可写或异常，或者超时
    // tv 结构体指定了等待的最长时间，当超时时间达到或者有事件发生时，select 函数会返回。

    LOG(INFO) << "SELECT START:" << smoothed_timeout_ << "!!!";

    while (true) {
      res = select(sockfd_ + 1, &rfds, NULL, NULL, &tv); // select监听
      if (res == -1) {
        LOG(ERROR) << "Error in select";
      } else if (res > 0) {  // ACK available event
        wait_for_ack();

        if (cwnd_ >= ssthresh_) {
          //慢启动---->拥塞避免
          LOG(INFO) << "CHANGE TO CONG AVD";
          is_cong_avd_ = true;
          is_slow_start_ = false;

          cwnd_ = 1;
          ssthresh_ = 64;
        }

        if (sliding_window_->last_acked_packet_ == sliding_window_->last_packet_sent_) { // 检查是否所有已发送的数据包都已经收到了确认
          if (is_slow_start_) {
            cwnd_ = cwnd_ * 2;
          } else {
            cwnd_ = cwnd_ + 1;
          }
          break;
        }
      } else {
        // 拥塞发生--超时重传
        LOG(INFO) << "Timeout occurred SELECT::" << smoothed_timeout_;
        // LOG(INFO) << "CHANGE TO SLOW START";
        ssthresh_ = cwnd_ / 2;
        if (ssthresh_ < 1) {
          ssthresh_ = 1;
        }
        cwnd_ = 1;

        // 重新开始慢启动
        if (is_fast_recovery_) {
          // LOG(INFO) << "CHANGE TO SLOW START from FastRecovery start_byte_:"
          //           << start_byte_;
          is_fast_recovery_ = false; // 如果 is_fast_recovery_ 为真，说明之前处于快速恢复状态，需要将其置为假，表示从快速恢复状态切换回慢启动状态
        }
        is_slow_start_ = true;
        is_cong_avd_ = false; // 表示不处于拥塞避免状态

        // retransmit all unacked segments
        for (int i = sliding_window_->last_acked_packet_ + 1;
             i <= sliding_window_->last_packet_sent_; i++) {
          int retransmit_start_byte = 0;
          if (sliding_window_->last_acked_packet_ != -1) {
            retransmit_start_byte = sliding_window_->sliding_window_buffers_[sliding_window_->last_acked_packet_].first_byte_ + MAX_DATA_SIZE;
          }
          // sliding_window_->sliding_window_buffers_[sliding_window_->last_acked_packet_] 获取最后一个被确认的数据包。 .first_byte_ 是该数据包的第一个字节位置
          // MAX_DATA_SIZE 是每个数据包的最大数据大小

          LOG(INFO) << "Timeout Retransmit seq number"
                    << retransmit_start_byte + initial_seq_number_; // 记录要重传的数据包序列号
          retransmit_segment(retransmit_start_byte);
          packet_statistics_->retransmit_count_++;
          LOG(INFO) << "Timeout: retransmission at " << retransmit_start_byte;
        }

        break;
      }
    }
    LOG(INFO) << "SELECT END !!!";
    LOG(INFO) << "current byte ::" << start_byte_ << " file_length_ "
              << file_length_;
  }

  gettimeofday(&process_end_time, NULL);

  int64_t total_time =
      (process_end_time.tv_sec * 1000000 + process_end_time.tv_usec) -
      (process_start_time.tv_sec * 1000000 + process_start_time.tv_usec);

  int total_packet_sent = packet_statistics_->slow_start_packet_sent_count_ +
                          packet_statistics_->cong_avd_packet_sent_count_;
  LOG(INFO) << "\n";
  LOG(INFO) << "========================================";
  LOG(INFO) << "Total Time: " << (float)total_time / pow(10, 6) << " secs";
  LOG(INFO) << "Statistics: 拥塞控制--慢启动: "
            << packet_statistics_->slow_start_packet_sent_count_
            << " 拥塞控制--拥塞避免: "
            << packet_statistics_->cong_avd_packet_sent_count_;
  LOG(INFO) << "Statistics: Slow start: "
            << ((float)packet_statistics_->slow_start_packet_sent_count_ /
                total_packet_sent) * 100 << "% CongAvd: "
            << ((float)packet_statistics_->cong_avd_packet_sent_count_ /
                total_packet_sent) * 100 << "%";
  LOG(INFO) << "Statistics: Retransmissions: "
            << packet_statistics_->retransmit_count_;
  LOG(INFO) << "========================================";
}

void UdpServer::send_packet(int seq_number, int start_byte) {
  bool lastPacket = false;
  int dataLength = 0;
  if (file_length_ <= start_byte + MAX_DATA_SIZE) { // 判断是否为最后一个数据包
    LOG(INFO) << "Last packet to be sent !!!";
    dataLength = file_length_ - start_byte;
    lastPacket = true;
  } else {
    dataLength = MAX_DATA_SIZE;
  }

  struct timeval time;

  gettimeofday(&time, NULL);

  if (sliding_window_->last_packet_sent_ != -1 &&
      start_byte < sliding_window_->sliding_window_buffers_[sliding_window_->last_packet_sent_].first_byte_) {  
  // // 如果last_packet_sent_不为-1且start_byte小于上次发送的数据包的起始字节
    for (int i = sliding_window_->last_acked_packet_ + 1;
         i < sliding_window_->last_packet_sent_; i++) { // 在滑动窗口中查找该起始字节对应的缓冲区，并更新发送时间
      if (sliding_window_->sliding_window_buffers_[i].first_byte_ ==
          start_byte) {
        sliding_window_->sliding_window_buffers_[i].time_sent_ = time;
        break;
      }
    }

  } else { // 否则，创建一个新的SlidWinBuffer并添加到滑动窗口中
    SlidWinBuffer slidingWindowBuffer;
    slidingWindowBuffer.first_byte_ = start_byte;
    slidingWindowBuffer.data_length_ = dataLength;
    slidingWindowBuffer.seq_num_ = initial_seq_number_ + start_byte;
    struct timeval time;
    gettimeofday(&time, NULL);
    slidingWindowBuffer.time_sent_ = time;
    sliding_window_->last_packet_sent_ =
        sliding_window_->AddToBuffer(slidingWindowBuffer);
  }
  read_file_and_send(lastPacket, start_byte, start_byte + dataLength);
}

void UdpServer::wait_for_ack() { 
  unsigned char buffer[MAX_PACKET_SIZE];
  memset(buffer, 0, MAX_PACKET_SIZE);
  socklen_t addr_size;
  struct sockaddr_in client_address;
  addr_size = sizeof(client_address);
  int n = 0;
  int ack_number;

  // 接收数据包直到成功为止
  while ((n = recvfrom(sockfd_, buffer, MAX_PACKET_SIZE, 0,
                       (struct sockaddr *)&client_address, &addr_size)) <= 0) {
  };

  // 反序列化接收到的数据包到 DataSegment 结构
  DataSegment ack_segment;
  ack_segment.DeserializeToDataSegment(buffer, n);

  // LOG(INFO) << "ACK Received: ack_number " << ack_segment->ack_number_;
  SlidWinBuffer last_packet_acked_buffer =
      sliding_window_->sliding_window_buffers_[sliding_window_->last_acked_packet_]; 
  //  从滑动窗口缓冲区中获取最后一个确认的数据包缓冲区

  if (ack_segment.ack_flag_) { // 确认接收到的数据包是一个 ACK 包
    if (ack_segment.ack_number_ == sliding_window_->send_base_) { // 如果 ACK 号等于 send_base_，表示重复 ACK，增加重复 ACK 计数
      LOG(INFO) << "DUP ACK Received: ack_number: " << ack_segment.ack_number_;
      sliding_window_->dup_ack_++;
      // 快速重传
      if (sliding_window_->dup_ack_ == 3) {
        packet_statistics_->retransmit_count_++;
        LOG(INFO) << "Fast Retransmit seq_number: " << ack_segment.ack_number_;
        retransmit_segment(ack_segment.ack_number_ - initial_seq_number_);
        sliding_window_->dup_ack_ = 0;
        if (cwnd_ > 1) {
          cwnd_ = cwnd_ / 2;
        }
        ssthresh_ = cwnd_;
        is_fast_recovery_ = true;
        // LOG(INFO) << "Change to fast Recovery ack_segment->ack_number:"
        //           << ack_segment->ack_number_;
      }
      // 如果接收到三个重复 ACK，则触发快速重传机制，重传该数据段，并更新拥塞窗口 cwnd_ 和慢启动阈值 ssthresh_，进入快速恢复状态

    } else if (ack_segment.ack_number_ > sliding_window_->send_base_) {
      if (is_fast_recovery_) { // 快恢复
        // LOG(INFO) << "Change to Cong Avoidance from fast recovery recv ack:"
        //           << ack_segment->ack_number_;
        cwnd_++;
        is_fast_recovery_ = false;
        is_cong_avd_ = true; // 拥塞状态
        is_slow_start_ = false;
      }
      // 如果 ACK 号大于 send_base_，则表示接收到新的 ACK。如果当前处于快速恢复状态，则更新窗口和状态。

      sliding_window_->dup_ack_ = 0; // 清零
      sliding_window_->send_base_ = ack_segment.ack_number_;
      // 当接收到新的 ACK 包时，清除重复 ACK 计数，并将 send_base_ 更新为最新的 ACK 号

      // 如果 last_acked_packet_ 为 -1（表示没有已确认的包），则将其初始化为 0，并获取第一个已确认的包的缓冲区信息
      if (sliding_window_->last_acked_packet_ == -1) { 
        sliding_window_->last_acked_packet_ = 0;
        last_packet_acked_buffer = sliding_window_->sliding_window_buffers_[sliding_window_->last_acked_packet_];
      }

      ack_number = last_packet_acked_buffer.seq_num_ +
                   last_packet_acked_buffer.data_length_; // 计算当前已确认包的 ACK 号，该值为包的序列号加上包的数据长度

      while (ack_number < ack_segment.ack_number_) {
        sliding_window_->last_acked_packet_++;
        last_packet_acked_buffer = sliding_window_->sliding_window_buffers_[sliding_window_->last_acked_packet_];
        ack_number = last_packet_acked_buffer.seq_num_ + last_packet_acked_buffer.data_length_;
      }
      // 如果计算的 ack_number 小于接收到的 ACK 包的 ACK 号，则更新 last_acked_packet_，
      // 并获取下一个已确认包的缓冲区信息，直到 ack_number 大于或等于接收到的 ACK 号

      struct timeval startTime = last_packet_acked_buffer.time_sent_;
      struct timeval endTime;
      gettimeofday(&endTime, NULL);

      // LOG(INFO) << "seq num of last_acked "
      //           << last_packet_acked_buffer.seq_num_;
      calculate_rtt_and_time(startTime, endTime);
    }
    // LOG(INFO) << "sliding_window_->lastAckedPacket"
    //           << sliding_window_->last_acked_packet_;
  }
}

void UdpServer::calculate_rtt_and_time(struct timeval start_time,
                                       struct timeval end_time) {
  if (start_time.tv_sec == 0 && start_time.tv_usec == 0) {
    return;
  }
  long sample_rtt = (end_time.tv_sec * 1000000 + end_time.tv_usec) -
                    (start_time.tv_sec * 1000000 + start_time.tv_usec); 
  // 计算从 start_time 到 end_time 的样本往返时延（RTT），单位为微秒

  smoothed_rtt_ = smoothed_rtt_ + 0.125 * (sample_rtt - smoothed_rtt_);
  // 平滑的往返时延，通过加权平均来更新，以减少抖动

  dev_rtt_ = 0.75 * dev_rtt_ + 0.25 * (abs(smoothed_rtt_ - sample_rtt));
  smoothed_timeout_ = smoothed_rtt_ + 4 * dev_rtt_;
  // 根据平滑RTT和RTT偏差计算的平滑超时时间。通常情况下，超时时间应该考虑网络传输的不确定性，平滑超时时间可以更好地适应网络环境的变化

  if (smoothed_timeout_ > 1000000) {
    smoothed_timeout_ = rand() % 30000;
  }
}

void UdpServer::retransmit_segment(int index_number) {
  for (int i = sliding_window_->last_acked_packet_ + 1;
       i < sliding_window_->last_packet_sent_; i++) {
    if (sliding_window_->sliding_window_buffers_[i].first_byte_ == index_number) {
      struct timeval time;
      gettimeofday(&time, NULL); // 获取当前时间戳，记录下数据段的重传时间
      sliding_window_->sliding_window_buffers_[i].time_sent_ = time;
      // 更新找到的数据段的 time_sent_ 属性为当前时间戳 time
      break;
    }
  }

  read_file_and_send(false, index_number, index_number + MAX_DATA_SIZE);
}

void UdpServer::read_file_and_send(bool fin_flag, int start_byte,
                                   int end_byte) {
  int datalength = end_byte - start_byte;
  if (file_length_ - start_byte < datalength) { // 判断最后一个数据包
    datalength = file_length_ - start_byte;
    fin_flag = true;
  }
  char *fileData = reinterpret_cast<char *>(calloc(datalength, sizeof(char)));
  if (!file_.is_open()) {
    LOG(ERROR) << "File open failed !!!";
    return;
  }

  // read txt
  file_.seekg(start_byte); // 将文件流 file_ 的读取位置（文件指针）移动到 start_byte 字节处
  file_.read(fileData, datalength); // 从当前文件指针位置开始读取数据，并存储到 fileData 数组中

  DataSegment *data_segment = new DataSegment();
  data_segment->seq_number_ = start_byte + initial_seq_number_;
  data_segment->ack_number_ = 0;
  data_segment->ack_flag_ = false;
  data_segment->fin_flag_ = fin_flag;
  data_segment->length_ = datalength;
  data_segment->data_ = fileData;

  send_data_segment(data_segment);
  LOG(INFO) << "Packet sent:seq number: " << data_segment->seq_number_;

  free(fileData);
  free(data_segment);
}

char *UdpServer::GetRequest(int client_sockfd) {
  char *buffer =
      reinterpret_cast<char *>(calloc(MAX_PACKET_SIZE, sizeof(char))); // calloc 函数用于分配一块大小为 MAX_PACKET_SIZE 字节的内存，并将其初始化为零
  struct sockaddr_in client_address;
  socklen_t addr_size;
  memset(buffer, 0, MAX_PACKET_SIZE); // 使用 memset 将 buffer 置零，确保缓冲区没有残留数据
  addr_size = sizeof(client_address);
  recvfrom(client_sockfd, buffer, MAX_PACKET_SIZE, 0,                    
           (struct sockaddr *)&client_address, &addr_size);                   // recvfrom

  LOG(INFO) << "***Request received is: " << buffer;
  cli_address_ = client_address;
  return buffer;
}

void UdpServer::send_data_segment(DataSegment *data_segment) {
  char *datagramChars = data_segment->SerializeToCharArray();
  sendto(sockfd_, datagramChars, MAX_PACKET_SIZE, 0,                        // sendto
         (struct sockaddr *)&cli_address_, sizeof(cli_address_));
  free(datagramChars);
}
}  // namespace safe_udp
