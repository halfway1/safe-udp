#!/usr/bin/env bash

MONITOR_HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"
# dirname "${BASH_SOURCE[0]}" 
# /docker/scipts

# MONITOR_HOME_DIR
# saft-udp


local_host="$(hostname)"
user="${USER}"
uid="$(id -u)"
group="$(id -g -n)"
gid="$(id -g)"
# id -u 命令返回当前用户的用户ID（UID）
# id -g -n 命令返回当前用户的主组名
# id -g 命令返回当前用户的主组ID（GID）


echo "stop and rm docker" 
docker stop safe_udp > /dev/null
docker rm -v -f safe_udp > /dev/null

# > /dev/null：将命令的标准输出重定向到 /dev/null，即丢弃输出信息，保持终端清洁。
# -v：删除与容器关联的匿名卷。
# -f：强制删除运行中的容器（如果尚未停止，则强制停止后删除）。

echo "start docker"
docker run -it -d \
--privileged=true \
--name safe_udp \
-e DOCKER_USER="${user}" \
-e USER="${user}" \
-e DOCKER_USER_ID="${uid}" \
-e DOCKER_GRP="${group}" \
-e DOCKER_GRP_ID="${gid}" \
-v ${MONITOR_HOME_DIR}:/work \
-v ${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR} \
-v /tmp:/tmp \
--network host \
safe:udp

# -it：以交互模式运行容器，并分配一个伪终端
# --privileged=true：以特权模式运行容器，允许容器访问宿主机的所有设备
# -v ${MONITOR_HOME_DIR}:/work：将宿主机的 MONITOR_HOME_DIR 挂载到容器内的 /work 目录  宿主机目录:容器目录
# 即使宿主机目录挂载到容器目录时，容器内的目录不存在，Docker 也会自动创建这个目录。所以即使容器内没有 /work 目录，只要你挂载了宿主机目录到 /work，容器内就会有这个目录
