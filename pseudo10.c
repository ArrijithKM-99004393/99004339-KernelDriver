#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/wait.h>


#define MAX_SIZE 1024
dev_t pdevid;
int ndevices=1;
struct class *pclass;
unsigned char *pbuffer;


typedef struct
{
	struct cdev cdev;
	struct kfifo k1;
	struct device* pdev;
	wait_queue_head_t rd_queue;
	wait_queue_head_t wr_queue;
}PRIV_OBJ;


PRIV_OBJ* pobj;



int psuedo_open(struct inode* inode, struct file* file)
{
	printk("psuedo--open method\n");
	return 0;
}
int psuedo_close(struct inode* inode, struct file* file)
{
	printk("psuedo--release method\n");
	return 0;
}
ssize_t psuedo_read(struct file* file,char __user* ubuf, size_t usize, loff_t* off)
{
	int rcount = usize,ret;
	char *tbuf=kmalloc(rcount,GFP_KERNEL);
	if(kfifo_is_empty(&pobj->k1)) {
		//printk("pseudo read -- buffer is empty\n");  
		wait_event_interruptible(pobj->rd_queue,(kfifo_len(&pobj->k1)>0)); 

	}
	if(rcount > kfifo_len(&pobj->k1))
		rcount =kfifo_len(&pobj->k1);
	
	
	ret=copy_to_user(ubuf, tbuf, rcount);
	if(ret)
	{
		printk("copy to user failed");
		return -EFAULT;
	}
	kfifo_out(&pobj->k1, tbuf, rcount);
	wake_up_interruptible(&pobj->wr_queue);
	kfree(tbuf);
	printk("Pseudo--read method, transferred %d bytes\n",rcount);
	return rcount;
	
}
ssize_t psuedo_write(struct file* file,const char __user* ubuf, size_t usize, loff_t* off)
{
	int wcount = usize,ret;
	char *tbuf=kmalloc(wcount,GFP_KERNEL);
	if(kfifo_is_full(&pobj->k1))    
		wait_event_interruptible(pobj->wr_queue,(kfifo_len(&pobj->k1)>0)); 
	if(wcount > kfifo_avail(&pobj->k1))
		wcount =kfifo_avail(&pobj->k1);
	
	ret=copy_from_user(tbuf, ubuf, wcount);
	if(ret)
	{
		printk("copy from user failed");
		return -EFAULT;
	}
	kfifo_in(&pobj->k1, tbuf, wcount);
	wake_up_interruptible(&pobj->rd_queue);
	kfree(tbuf);
	printk("Pseudo--write method, transferred %d bytes\n",wcount);
	return wcount;
	
}

struct file_operations fops=
{
	.open = psuedo_open,
	.release = psuedo_close,
	.write = psuedo_write,
	.read = psuedo_read
};



static int __init psuedo_init(void)
{
	int ret,i=0;
	init_waitqueue_head(&pobj->rd_queue);
	init_waitqueue_head(&pobj->wr_queue);
	pbuffer = kmalloc(MAX_SIZE, GFP_KERNEL);
	pobj= kmalloc(sizeof(PRIV_OBJ), GFP_KERNEL);
	kfifo_init(&pobj->k1,pbuffer,MAX_SIZE);
	pclass = class_create(THIS_MODULE,"psuedo_class");
	ret=alloc_chrdev_region(&pdevid,0,ndevices,"psuedo_sample");
	if(ret)
	{
		printk("Psuedo:Failed to register driver\n");
		return -EINVAL;
	}
	
	cdev_init(&pobj->cdev, &fops);
	kobject_set_name(&pobj->cdev.kobj,"pdevice%d",i);
	ret=cdev_add(&pobj->cdev,pdevid,1);
	printk("Successfully registered, major=%d,minor=%d\n",MAJOR(pdevid),MINOR(pdevid));
	pobj->pdev = device_create(pclass,NULL,pdevid,NULL,"psample%d",i);
	if(pobj->pdev==NULL)
	{
	printk("error creating device");
	}
	printk("Psuedo10 driver sample..welcome\n");
	return 0;
}

static void __exit psuedo_exit(void)
{
	cdev_del(&pobj->cdev);
	device_destroy(pclass,pdevid);
	kfifo_free(&pobj->k1);
	kfree(pobj);
	unregister_chrdev_region(pdevid,ndevices);
	class_destroy(pclass);
	printk("Psuedo10 driver sample..byr\n");
	
}

module_init(psuedo_init);
module_exit(psuedo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arrijith");
MODULE_DESCRIPTION("Psuedo module");
