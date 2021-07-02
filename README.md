# sOS32

一个简单的 32 位操作系统 🐍OS，参考了大佬的教程。目前仅能在屏幕上输出信息，并不能进行交互。

使用 GRUB 进行引导，借助了一个安装有 GRUB 的软盘 floppy.img。可以查看 Makefile 中的 update_image 部分

## startup

``` shell
# 编译并安装内核到软盘上
make
# 目前依赖 qemu 启动，也可以使用 bochs
make qemu
```

## 参考资料

[Bran's Kernel Development Tutorial](http://www.osdever.net/bkerndev/Docs/title.htm)

[hurley25/hurlex-doc](https://github.com/hurley25/hurlex-doc)