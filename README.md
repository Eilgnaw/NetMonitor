# NetMonitor
Ubutu网络监控及通知

由于办公室的路由器质量问题,桥接来的网不久就会断网,需要重置或者桥接到新的路由器,导致我本地服务器 ip 变动,为了能及时获取新的 ip 写进 hosts,所以在服务器上做了监听 ip变化,然后推送到钉钉群.

### 使用说明(所有代码都放在了/opt 下,路径请根据自己需要替换)
-  clone 代码后,编译程序. `gcc -o  NetMonitor  NetMonitor.c` ,`./NetMonitor`运行即可.
- 因为服务器有两张网卡,所以在NetMonitor.c 内,进行了判断,只推送 eth0 网卡 ip 变动.
- 看了下 c 程序内调用 python 感觉太麻烦,所以写了个 shell 脚本做了下中介,使用之前记得`chmod `授权
- sendip.py 只是负责把传过来的 ip 通过钉钉机器人发送出去,没什么好说的.
- 为了保证服务器重启后还能正常监听,所以在`/etc/rc.local` 的`exit 0` 前加入了`su - wzq -c "/opt/testc &" `
### 参考资料
- 主要代码来自[《Linux下使用NetLink 监听网络变化》](http://blog.csdn.net/gt945/article/details/45315911)
- [《rtnetlink 中文描述》](http://blog.csdn.net/romainxie/article/details/8300443)
- 扩展阅读[《DDNS 的工作原理及其在 Linux 上的实现》](http://blog.csdn.net/romainxie/article/details/8300443)
-
- 
