# udpforward
udp 端口转发. 将某个端口的包, 转发给其他的多个地址.

socat 是比较强大的端口转发程序, 但是只支持单个输出, 无法转发给多个目的地. 因此, 编写了这个小工具.



## 用法

udpforward [option] sink_address
  -b, --bufsize         recv buffer size of socket
  -d, --drop            rate of drop packet. must be in [0.0-1.0]
  -f, --file            dump file. such as out.ts
  -h, --help            print help info
  -p, --port            local port.
  -v, --version         print version info

-b 表示socket接收缓冲区大小

-d 表示丢包率, 取值 [0.0,1.0]

-f 表示导出文件

-p 表示UDP接收端口

sink_address 格式为 ip:port .比如 127.0.0.1:9000



## 实例

将本地端口 10000 接收到的包转发给本机7000,8000, 并且导出文件到out.ts

udpforward -p 10000 -f out.ts 127.0.0.1:7000 127.0.0.1:8000



启动程序后, 可以输入控制命令. 目前支持的控制命令有:

drop  表示设置丢包率. 比如 drop 0.0.1

