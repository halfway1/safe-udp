#!/usr/bin/env bash

docker exec \
    -u root \
    -it safe_udp \
    env LANG=C.UTF-8 \
    /bin/bash

# -u root：指定在容器内以 root 用户执行命令
# env LANG=C.UTF-8：在执行命令之前设置环境变量 LANG 为 C.UTF-8。这可以确保终端输出使用 UTF-8 编码，以便正确显示国际化字符
# /bin/bash：在容器内启动 Bash shell