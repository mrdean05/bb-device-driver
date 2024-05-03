<h2> BEAGLEBONE BLACK DEVICE DRIVERS DEVELOPMENT </h2>
- This project is made up of a number of tasks that describes the development of device drivers for beaglebone black.

## Task
| Task   | Description                                               |
|--------|-----------------------------------------------------------|
| 01     | developed and installed simple kernel module              |
| 04     | driver with multiple nodes                                |

## Setup
When cross compiling a kernel module for a particular target board, the linux kernel version on the target should be the same version used in cross compiling.
The below presents setup for the linux kernel compilation.
- Clone the beagleboard kernel
```shell
git clone https://github.com/beagleboard/linux.git
```
- Install the below
```shell
sudo apt-get install -y libgmp-dev
sudo apt-get install libmpc-dev
sudo apt-get install gcc-arm-linux-gnueabihf
```
- Use the command below to clean all temporary folder, object files, images, etc.
```shell
make ARCH=arm distclean
```
- Generate the .config file. The bb.org_defconfig is found in the arch/arm/configs.
```shell
make ARCH=arm bb.org_defconfig
```
- Cross compile to generate the linux image(both a kernel image and a device tree source files will be generated).
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j4
```
- Build and generate the in-tree loabable kernel module
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
```
- Installs all the generated .ko files in the host default path.
```shell
sudo make ARCH=arm modules_install
```