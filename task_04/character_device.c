#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define NO_OF_DEVICES 4

#define RDONLY	 		0x01
#define WRONLY 			0x02
#define RDWR   			0x03

#define MEM_SIZE_1 		1024
#define MEM_SIZE_2 		512
#define MEM_SIZE_3 		1024
#define MEM_SIZE_4 		512

char device_buf_1 [MEM_SIZE_1];
char device_buf_2 [MEM_SIZE_2];
char device_buf_3 [MEM_SIZE_3];
char device_buf_4 [MEM_SIZE_4];

int check_permission(int permission, int access_mode);
int dev_open (struct inode *inode, struct file *filp);
ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *f_pos);
int dev_release (struct inode *inode, struct file *filp);
loff_t dev_llseek(struct file *filp, loff_t offset, int whence);

/* Structure represent device private data */
struct memdev_private_data {
	char *buffer;
	unsigned size;
	const char *serial_no;
	int permission;
	struct cdev cdev;
	struct mutex memdev_lock;
};

/* Structure represent driver private data */
struct memdrv_private_data {
	int total_device;
	struct memdev_private_data memdev_data[NO_OF_DEVICES];
	dev_t device_number;
	struct class* mem_class;
	struct device* mem_device;
};

/* Initialize the driver private data */
struct memdrv_private_data memdrv_data = {
	.total_device = NO_OF_DEVICES,
	.memdev_data = {
	[0] = {
			.buffer = device_buf_1,
			.size = MEM_SIZE_1,
			.permission = RDONLY,
			.serial_no = "MEM-DEV1-2212"
		}, 
	[1] = {
			.buffer = device_buf_2,
			.size = MEM_SIZE_2,
			.permission = WRONLY,
			.serial_no = "MEM-DEV2-7458"
		},
	[2] = {
			.buffer = device_buf_3,
			.size = MEM_SIZE_3,
			.permission = RDONLY,
			.serial_no = "MEM-DEV3-8754"
		},
	[3] = {
			.buffer = device_buf_3,
			.size = MEM_SIZE_3,
			.permission = RDWR,
			.serial_no = "MEM-DEV1-2212"
		}	
	}
};


unsigned int major_num, minor_num; 
struct cdev device_cdev = {
	.owner = THIS_MODULE
};

int check_permission(int permission, int access_mode){
	if (permission == RDWR) return 0;
	
	/* check read only */
	if ((permission == RDONLY) && ((access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE))) return 0;

	/* check write only */
	if ((permission == WRONLY) && ((access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE))) return 0;

	return -EPERM;
}



/* This gets called when the user tries to open the device files. */
int dev_open (struct inode *inode, struct file *filp){
	pr_info("Module has been opened\n");
	int ret;

	/* 
	 * Since it  has multiple device files of the same type, 
	 * the driver needs to find out on which device file open was attempted by the user. 
	 * This can be achieved by getting the info from the inode. 
	 * https://codebrowser.dev/linux/linux/include/linux/fs.h.html
	*/
	
	int minor_n;
	minor_n = MINOR(inode->i_rdev);
	pr_info("minor number = %d\n", minor_n);

	/* 
	 * get the device's private data structure 
	 * When the specific device file has been found, the driver should be able to provide the specific private data/ specific info.
	 * The container concept below uses the address of the member of a structure to obtain the address of the structure itself.
	 * This -container_of macro- is defined in /include/linux/kernel.h. It takes three members, as seen below
	 * #define container_of(ptr, type, member)
	 * ptr - pointer of the member, type - type of the container struct in which 'member is embedded in', member - name of the member the 'ptr' refers to
	*/

	struct memdev_private_data *drv_data;
	drv_data = container_of(inode->i_cdev, struct memdev_private_data, cdev);

	/*
	 * The obtained private data will be used for other methods - read, write, lseek. 
	 * These methods donot have inode, as such the private data can be saved in the file structure to be accessed by other methods.
	 * https://codebrowser.dev/linux/linux/include/linux/fs.h.html
	 * the struct file has a member called private_data
	*/

	filp->private_data = drv_data;

	/* Check permission */
	ret = check_permission(drv_data->permission, filp->f_mode);
	if(!ret) 
		pr_info("Open successful");
	else
		pr_info("Open not successful");

	return ret;
}

ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	
	struct memdev_private_data *drv_data = (struct memdev_private_data *)filp-> private_data;
	unsigned MEM_SIZE =  drv_data->size;
	
	if (mutex_lock_interruptible(&drv_data->memdev_lock)) return -EINTR;
	
	if ((*f_pos + count)> MEM_SIZE)
		count = MEM_SIZE - *f_pos;
	
	if (copy_to_user(buf, drv_data->buffer, count)) return -EFAULT;
		
	*f_pos += count;
	pr_info("Number of bytes read: %zu\n", count);

	mutex_unlock(&drv_data->memdev_lock);
	return count;
}

ssize_t dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *f_pos){

	struct memdev_private_data *drv_data = (struct memdev_private_data *)filp-> private_data;
	unsigned MEM_SIZE =  drv_data->size;

	if (mutex_lock_interruptible(&drv_data->memdev_lock)) return -EINTR;	
	
	if ((*f_pos + count)> MEM_SIZE)
		count = MEM_SIZE - *f_pos;
	
	if (copy_from_user(buf, drv_data->buffer, count))
		return -ENOMEM;

	*f_pos += count;
	pr_info("Number of bytes written: %zu\n", count);

	mutex_unlock(&drv_data->memdev_lock);
	return count;
}

int dev_release (struct inode *inode, struct file *filp){
	pr_info("Module has been released\n");
	return 0;
}

loff_t dev_llseek(struct file *filp, loff_t offset, int whence){
	
	loff_t temp;
	

	struct memdev_private_data *drv_data = (struct memdev_private_data *)filp-> private_data;
	unsigned int max_size =  drv_data->size;

	switch(whence){
		/* Set the file pointer to the start of the file when performing a seek operation */	
		case SEEK_SET:
			if ((offset > max_size) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
		break;

		/* Set the file pointer to the current position when performing a seek operation */
		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if ((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
		break;

		/* Set the file pointer to the end of file when performing a seek operation */
		case SEEK_END:
			temp = max_size + offset;
				if ((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
		break;

		default:
			return -EINVAL;
	}
	pr_info("The value of the f pos: %lld\n", filp->f_pos);
	return filp->f_pos;
}

struct file_operations dev_files = {
	.llseek = dev_llseek,
	.open = dev_open,
	.write = dev_write,
	.read = dev_read,
	.release = dev_release,
	.owner = THIS_MODULE
};


static int __init mem_device_driver_init(void)
{
	pr_info("initializing module\n");
	int ret, i;

	/* obtain device number */
	ret = alloc_chrdev_region(&memdrv_data.device_number, 0, NO_OF_DEVICES, "char_device"); 
	if (ret < 0)
	{
		pr_err("Failed to get device number\n");
		goto err_dev_num;
	}

	/* create a class - visibility in in /sys/class */
	memdrv_data.mem_class = class_create(THIS_MODULE, "mem_class");
	if (IS_ERR(memdrv_data.mem_class))
	{
		ret = PTR_ERR(memdrv_data.mem_class);
		pr_err("Failed to create a class.\n");
		goto error_class;
	}
	
	for (i = 0; i < NO_OF_DEVICES; i++)
	{
		major_num = MAJOR(memdrv_data.device_number); minor_num = MINOR(memdrv_data.device_number);
		pr_info("Device number <%d> <Major>:<Minor> = %d:%d\n", i+1, major_num, minor_num + i);
	
		/* register device with virtual file system(VFS). This includes tying to a file operation */
		cdev_init(&memdrv_data.memdev_data[i].cdev, &dev_files);


		ret = cdev_add(&memdrv_data.memdev_data[i].cdev, memdrv_data.device_number + i, 1);
		if (ret < 0){
			pr_err("Failed to register device with VFS\n");
			goto error_cdev;
		}
		
		/* Create class */
		memdrv_data.mem_device = device_create(memdrv_data.mem_class, NULL ,memdrv_data.device_number + i, NULL, "memdev-%d", i+1);
		if (IS_ERR(memdrv_data.mem_device)){
			ret = PTR_ERR(memdrv_data.mem_device);
			pr_err("Failed to create a device.\n");
			goto error_device;
		}
	}

	pr_info("Module loaded\n");
	return 0;

	error_cdev:
	error_device:
		for (; i>=0; i--) {
			device_destroy(memdrv_data.mem_class, memdrv_data.device_number + i);
			cdev_del(&memdrv_data.memdev_data[i].cdev);
		}
		class_destroy(memdrv_data.mem_class);

	error_class:
		unregister_chrdev_region(memdrv_data.device_number, NO_OF_DEVICES);

	err_dev_num:
		return ret;
}

static void __exit mem_device_driver_exit(void)
{
	int i;
	for (i = 0; i<=0; i++) {
		device_destroy(memdrv_data.mem_class, memdrv_data.device_number + i);
		cdev_del(&memdrv_data.memdev_data[i].cdev);
	}
	class_destroy(memdrv_data.mem_class);
	unregister_chrdev_region(memdrv_data.device_number, NO_OF_DEVICES);
	pr_info("Module unloaded\n");
}

module_init(mem_device_driver_init);
module_exit(mem_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dean Prince Agbodjan <dean.agbodjan@gmail.com>");
MODULE_DESCRIPTION("This is a hello world module");
MODULE_INFO(board,"Beaglebone black");
