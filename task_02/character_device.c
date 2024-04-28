#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>

#define MEM_SIZE 1024
char device_data [MEM_SIZE];

dev_t device_number;
unsigned int major_num, minor_num; 
struct cdev device_cdev = {
	.owner = THIS_MODULE
};
struct class* mem_class;
struct device* mem_device;

int dev_open (struct inode *inode, struct file *filp){
	pr_info("Module has been opened\n");
	return 0;
}

ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	if ((*f_pos + count)> MEM_SIZE)
		count = MEM_SIZE - *f_pos;
	
	if (copy_to_user(buf, device_data, count))
		return -EFAULT;

	*f_pos += count;
	pr_info("Number of bytes read: %zu\n", count);
	return count;
}

ssize_t dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *f_pos){
	if ((*f_pos + count)> MEM_SIZE)
		count = MEM_SIZE - *f_pos;
	
	if (copy_from_user(buf, device_data, count))
		return -ENOMEM;

	*f_pos += count;
	pr_info("Number of bytes written: %zu\n", count);
	return count;
}

int dev_release (struct inode *inode, struct file *filp){
	pr_info("Module has been released\n");
	return 0;
}

loff_t dev_llseek(struct file *filp, loff_t offset, int whence){
	
	loff_t temp;
	unsigned int max_size;

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
	int ret;

	/* obtain device number */
	ret = alloc_chrdev_region(&device_number, 0, 1, "char_device"); 
	if (ret < 0)
	{
		pr_err("Failed to get device number\n");
		goto err_dev_num;
	}

	major_num = MAJOR(device_number); minor_num = MINOR(device_number);
	pr_info("Device number <Major>:<Minor> = %d:%d\n", major_num, minor_num);

	/* register device with virtual file system(VFS). This includes tying to a file operation */
	cdev_init(&device_cdev, &dev_files);
	ret = cdev_add(&device_cdev, device_number, 1);
	if (ret < 0){
		pr_err("Failed to register device with VFS\n");
		goto error_cdev;
	}

	/* create a class - visibility in in /sys/class */
	mem_class = class_create(THIS_MODULE, "mem_class");
	if (IS_ERR(mem_class))
	{
		ret = PTR_ERR(mem_class);
		pr_err("Failed to create a class.\n");
		goto error_class;
	}

	/* Create class */
	mem_device = device_create(mem_class, NULL ,device_number, NULL, "mem");
	if (IS_ERR(mem_device)){
		ret = PTR_ERR(mem_device);
		pr_err("Failed to create a device.\n");
		goto error_device;
	}

	pr_info("Module loaded\n");
	return 0;

	error_device:
		class_destroy(mem_class);

	error_class:
		cdev_del(&device_cdev);

	error_cdev:
		unregister_chrdev_region(device_number, 1);

	err_dev_num:
		return ret;
}

static void __exit mem_device_driver_exit(void)
{
	device_destroy(mem_class, device_number);
	class_destroy(mem_class);
	cdev_del(&device_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("Module unloaded\n");
}

module_init(mem_device_driver_init);
module_exit(mem_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dean Prince Agbodjan <dean.agbodjan@gmail.com>");
MODULE_DESCRIPTION("This is a hello world module");
MODULE_INFO(board,"Beaglebone black");
