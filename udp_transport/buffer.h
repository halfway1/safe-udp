#pragma once

#include <sys/time.h>
#include <time.h>


//因为是文件内容传输，所以真正内容是在文本文件里，buffer类中记录索引
namespace safe_udp { //命名空间声明： 在这个命名空间内，定义的所有标识符都会被限定在 safe_udp 下
class SlidWinBuffer {
 public:
  SlidWinBuffer() {}
  ~SlidWinBuffer() {}

  int first_byte_; //第一个字节索引值
  int data_length_; //该buffer 数据大小
  int seq_num_; //该buffer 序列号
  struct timeval time_sent_; //发送时间戳，为了记录超时重传时间
};
}  // namespace safe_udp