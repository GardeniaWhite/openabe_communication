
# 基于openABE的加密通信
使用openabe进行数据加密。
### Debian/Ubuntu-based Linux环境下的openabe安装

关于openabe，详情可见https://github.com/zeutro/openabe

To compile OpenABE on Ubuntu or Debian Linux-based distro, first run the `deps/install_pkgs.sh` script from the source directory to install the OpenABE system-specific dependencies as follows:
	
	cd libopenabe-1.0.0/
	sudo -E ./deps/install_pkgs.sh

Note that you only have to do this once per system setup. After completion, you can proceed to compile the OpenABE as follows:

	. ./env
	make
	make test

All the unit tests should pass at this point and you can proceed to install the OpenABE in a standard location (`/usr/local`) as follows:

	sudo -E make install

To change the installation path prefix, modify the `INSTALL_PREFIX` variable in `libopenabe-1.0.0/Makefile`.

## Quick Start——检测openabe是否编译安装成功

To compile example C++ apps that use the high-level OpenABE crypto box API, do the following:

	. ./env
	make examples
	cd examples/

Then, execute the test apps for each mode of encryption supported:

	./test_kp
	./test_cp
	./test_pk
	
You can also execute the example that demonstrates use of the keystore with ABE decryption:
	
	./test_km

## 加密通信测试
1.可以在3台机器下安装部署好该项目，然后两端作为client，一端作为server。

2.在examples目录下，server_controller代表server端，client_host_a和client_host_b代表client端。

3.编译生成可执行文件以后，server端开启一个端口9999：./server_controller 9999

4.client-a端可以输入对应host的ip，假设端口为192.168.124.140，执行命令：./client_host_a 192.168.124.140 9999

5.client-b同理：./client_host_b 192.168.124.141 9999

6.在server与两台client同时连接的情况下，然后server端就可以分别向client-a和client-b发送密文，然后client-a和client-b解密。
