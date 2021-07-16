#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>


struct cdev cdev;
struct device *pdev;
struct class *pclass;
struct kfifo k1;
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
	if(kfifo_is_empty(&k1))
	{
		printk("Buffer is empty -- read method\n");
		return 0;
	}
	if (rlen > kfifo_len(&k1))
	{
		rlen = kfifo_len(&k1);
	}
	char *tbuf = kmalloc(rlen, GFP_KERNEL);
	kfifo_out(&k1, tbuf, rlen);
	ret = copy_to_user(ubuf, tbuf, rlen);
	if(ret)
	{
		printk("Copy_to_user failed");
		return -EINVAL;
	}
	kfree(tbuf);
	printk("Pseudo--read method, transferred %d bytes\n", rlen);
	return rlen;
}
ssize_t pseudo_write(struct file* file, const char __user* ubuf, size_t usize, loff_t* off)
{
	int wlen, ret;
	wlen = usize;
	if(kfifo_is_full(&k1))
	{
		printk("Buffer is full\n");
		return -ENOSPC;
	}
	if(wlen > kfifo_avail(&k1))
	{
		wlen = kfifo_avail(&k1);
	}
	char *tbuf = kmalloc(wlen, GFP_KERNEL);
	ret = copy_from_user(tbuf, ubuf, usize);
	if(ret)
	{
		printk("Error in copy_from_user\n");
		return -EINVAL;
	}
	kfifo_in(&k1, tbuf, wlen);
	kfree(tbuf);
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
	kfifo_init(&k1, pbuffer, MAX_SIZE);
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
	printk("Pseudo5 driver sample.. Welcome\n");
	return 0;
}
static void __exit pseudo_exit(void)
{
	kfifo_free(&k1);	
	cdev_del(&cdev);
	device_destroy(pclass, pdevid);
	unregister_chrdev_region(pdevid, ndevices);
	class_destroy(pclass);
	printk("Pseudo5 Driver Sample.. Bye\n");
}




module_init(pseudo_init);
module_exit(pseudo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arrijith");
MODULE_DESCRIPTION("parameter demo module");
