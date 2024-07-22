#include "data_segment.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

namespace safe_udp {
DataSegment::DataSegment() {
  ack_number_ = -1;
  seq_number_ = -1;
  length_ = -1;
}

char *DataSegment::SerializeToCharArray() { // 都是小端序
  if (final_packet_ != nullptr) {
    memset(final_packet_, 0, MAX_PACKET_SIZE);
    //检查 final_packet_ 是否已经分配内存。
    //如果 final_packet_ 不为空，则使用 memset 将 final_packet_ 内存区域清零，大小为 MAX_PACKET_SIZE 字节
  } else {
    final_packet_ =
        reinterpret_cast<char *>(calloc(MAX_PACKET_SIZE, sizeof(char)));
        //reinterpret_cast<char *> 将 calloc 返回的 void* 指针转换为 char* 类型的指针。
        //这种转换是强制性的，因为 calloc 返回 void* 类型，而在 C++ 中需要将其转换为特定类型的指针才能使用。
    if (final_packet_ == nullptr) {
      return nullptr;
      //如果 final_packet_ 为空（即尚未分配内存），使用 calloc 分配 MAX_PACKET_SIZE 字节的内存，并将其转换为 char* 类型。
      //如果内存分配失败（calloc 返回空指针），则函数返回 nullptr。
    }
  }

  memcpy(final_packet_, &seq_number_, sizeof(seq_number_));

  memcpy(final_packet_ + 4, &ack_number_, sizeof(ack_number_));

  memcpy((final_packet_ + 8), &ack_flag_, 1);

  memcpy((final_packet_ + 9), &fin_flag_, 1);

  memcpy((final_packet_ + 10), &length_, sizeof(length_));

  memcpy((final_packet_ + 12), data_, length_);
  // memcpy 将 source 所指向的内存区域中的前 num 个字节复制到 destination 所指向的内存区域
  return final_packet_; //返回指向 final_packet_ 的指针，即序列化后的字符数组
}

void DataSegment::DeserializeToDataSegment(unsigned char *data_segment,
                                           int length) {  // 都是小端序
  seq_number_ = convert_to_uint32(data_segment, 0);
  ack_number_ = convert_to_uint32(data_segment, 4);
  ack_flag_ = convert_to_bool(data_segment, 8);
  fin_flag_ = convert_to_bool(data_segment, 9);
  length_ = convert_to_uint16(data_segment, 10);

  data_ = reinterpret_cast<char *>(calloc(length + 1, sizeof(char))); //分配 length + 1 字节的内存，用于存储 data_。加 1 的原因是为了在结尾添加一个空字符 \0
  if (data_ == nullptr) {
    return;
  }
  memcpy(data_, data_segment + HEADER_LENGTH, length); //使用 memcpy 从字节数组的 HEADER_LENGTH 偏移量开始复制 length 字节到 data_
  *(data_ + length) = '\0'; //在 data_ 的最后一个字节添加空字符 \0，使其成为一个以空字符结尾的字符串
}

uint32_t DataSegment::convert_to_uint32(unsigned char *buffer,
                                        int start_index) {
  uint32_t uint32_value =
      (buffer[start_index + 3] << 24) | (buffer[start_index + 2] << 16) |
      (buffer[start_index + 1] << 8) | (buffer[start_index]);
  return uint32_value;
}
  //在这个上下文中，uint32_t 是一个 32 位（4 字节）的整数，每个字节由 8 位组成。
  //为了将每个字节放置在正确的位位置上，左移的位数必须是 8 的倍数，而不是 4。

uint16_t DataSegment::convert_to_uint16(unsigned char *buffer,
                                        int start_index) {
  uint16_t uint16_value =
      (buffer[start_index + 1] << 8) | (buffer[start_index]);
  return uint16_value;
}

bool DataSegment::convert_to_bool(unsigned char *buffer, int index) {
  bool bool_value = buffer[index];
  return bool_value;
}
}  // namespace safe_udp