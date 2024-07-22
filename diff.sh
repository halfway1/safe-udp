#!/bin/bash

# 检查脚本的参数数量是否为 1。如果不是，则输出使用方法并退出

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <file>"
    exit 1
fi

# "$#" 表示传递给脚本的参数数量。 $# 是一个特殊变量，用于表示命令行参数的个数
# -ne 1：表示“不等于 1”
# "$0" 表示脚本的名称

# exit 1：退出脚本，并返回状态码 1。状态码 1 通常表示发生错误或异常情况
# fi：结束 if 语句的块

file_1="/work/files/client_files/$1"
file_2="/work/files/server_files/$1"
# $1 是传递给脚本的第一个参数，即文件名


# 检查 file_1 和 file_2 是否存在。如果任一文件不存在，则输出错误消息并退出。
# -f 选项用于检查文件是否存在并且是一个常规文件。

if [ ! -f "$file_1" ]; then
    echo "Error: File $file_1 does not exist."
    exit 1
fi

if [ ! -f "$file_2" ]; then
    echo "Error: File $file_2 does not exist."
    exit 1
fi

# 使用 diff 命令比较 file_1 和 file_2 的内容，并将输出存储在 diff_output 变量中
# 检查 diff 命令的退出状态码：
# 如果状态码为 0，表示文件相同。
# 如果状态码非 0，表示文件不同，并输出差异。

diff_output=$(diff $file_1 $file_2)

if [ $? -eq 0 ]; then
    echo "Files are identical."
else
    echo "Files are different."
    echo "Differences:"
    echo "$diff_output"
fi

# $? 是一个特殊变量，保存了上一个命令的退出状态码
