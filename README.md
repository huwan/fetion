命令行飞信
==========
在cliofetion（程序版本cliofetion-standalone-2.2.2）的基础上在源码中加入默认的发送手机号和飞信密码，以防止在命令记录中泄露密码。

[cliofetion](http://code.google.com/p/ofetion/) 是在中国移动飞信协议的开源实现ofetion的基础上开发的命令行客户端，由[levin108](https://github.com/levin108/)开发和维护。

下载
====
请前往 https://github.com/mengyingchina/fetion/releases 下载。

安装
====
请参考目录下INSTALL文件。

使用方法
========
```
fetion -f mobileno -p password -t receive_mobileno -d message
```
- `-f mobileno -p password` 为发送飞信的手机号和密码，可选，不使用时默认为自己的手机号
- `-t receive_mobileno` 为接收飞信的手机号(自己或者飞信好友)，可选，不使用时默认发送给自己
- `-d message` 为具体的飞信内容，必选

致谢
====
感谢levin108开发的cliofetion，另程序参考了cliofetion群发程序[crh123-ofetion](http://code.google.com/r/crhan123-ofetion/)，一并表示感谢。
