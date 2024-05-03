<h2> TASK 4 </h2>
- Creating a driver with multiple nodes (/dev/memdev-1, /dev/memdev-2, /dev/memdev-3, /dev/memdev-4)

| Nodes             | Permissions                                               |
|-------------------|-----------------------------------------------------------|
| /dev/memdev-1     | RDONLY                                                    |
| /dev/memdev-2     | WRONLY                                                    |
| /dev/memdev-3     | RDWR                                                      |
| /dev/memdev-4     | RDWR                                                      |




## 1. Steps
- Compile the module against the host kernel
```shell
make host
```
- You can use the command below to check the module info
```shell
modinfo hello_world_mod.ko
```
- Insert and remove the module
```shell
sudo insmod hello_world_mod.ko
suod rmmod hello_world_mod.ko
```
- You use the command below to view the install module from dmesg
```shell
dmesg | tail -5
```
![local.conf file](mod-init-exit.png)