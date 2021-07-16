#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>



struct cdev cdev;
struct device *pdev;
struct class *pclass;
dev_t pdevid;
int ndevices = 1;

char *pbuffer;
int rd_offset = 0;
int wr_offset = 0;
int buflen = 0;
#define MAX_SIZE 1024

int pseudo_open(struct inode* inode, struct file* file)
{
	printk("Pseudo--open method \n");
	return 0;
}
int pseudo_close(struct inode* inode, struct file* file)
{
	printk("Pseudo--release method \n");
	return 0;
}
ssize_t pseudo_read(struct file* file, char __user* ubuf, size_t usize, loff_t* off)
{
	int ret, rlen;
	rlen = usize;
	if(buflen == 0)
	{
		printk("Buffer is empty -- read method\n");
		return 0;
	}
	if (rlen > buflen)
	{
		rlen = buflen;
	}
	ret = copy_to_user(ubuf, pbuffer, rlen);
	if(ret)
	{
		printk("Copy_to_user failed");
		return -EINVAL;
	}
	rd_offset += rlen;
	buflen -= rlen;
	printk("Pseudo--read method, transferred %d bytes\n", rlen);
	return rlen;
}
ssize_t pseudo_write(struct file* file, const char __user* ubuf, size_t usize, loff_t* off)
{
	int ret, wlen, kremain;
	wlen = usize;
	kremain = MAX_SIZE - wr_offset;
	if(kremain == 0)
	{
		return -ENOSPC;
	}
	if(wlen > kremain)
	{
		wlen = kremain;
	}
	ret = copy_from_user(pbuffer, ubuf, usize);
	if(ret)
	{
		printk("Error in copy_from_user\n");
		return -EINVAL;
	}
	buflen += wlen;
	wr_offset += wlen;
	printk("Pseudo--write method \n");
	return wlen;
}

struct file_operations fops = {
	.open = pseudo_open,
	.release = pseudo_close,
	.write = pseudo_write,
	.read = pseudo_read
};

static int __init pseudo_init(void)
{
	int ret,i = 0;
	pclass = class_create(THIS_MODULE, "pseudo_class");
	ret = alloc_chrdev_region(&pdevid, 0, ndevices, "pseudo_sample3");
	if(ret)
	{
		printk("Pseudo: Failed to register driver\n");
		return -EINVAL;
	}
	pbuffer = kmalloc(MAX_SIZE, GFP_KERNEL);
	if(pbuffer == NULL)
	{
		printk("Pseudo : kmalloc failed\n");
		return -ENOMEM;
	}
	cdev_init(&cdev, &fops);
	kobject_set_name(&cdev.kobj,"pdevice = %d",i);
	ret = cdev_add(&cdev, pdevid, 1);
	printk("Successfully registered, major=%d, minor=%d\n",MAJOR(pdevid), MINOR(pdevid));
	pdev = device_create(pclass, NULL, pdevid, NULL, "psample%d",i);
	if(pdev == NULL)
	{
		printk("error creating device..");
	}
	printk("Pseudo4 driver sample.. Welcome\n");
	return 0;
}
static void __exit pseudo_exit(void)
{
	kfree(pbuffer);	
	cdev_del(&cdev);
	device_destroy(pclass, pdevid);
	unregister_chrdev_region(pdevid, ndevices);
	class_destroy(pclass);
	printk("Pseudo4 Driver Sample.. Bye\n");
}




module_init(pseudo_init);
module_exit(pseudo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arrijith");
MODULE_DESCRIPTION("parameter demo module");
