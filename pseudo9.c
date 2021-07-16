#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define MAX_SIZE 1024
dev_t pdevid;
int ndevices=1;
struct cdev cdev;
struct device *pdev;
struct class *pclass;
unsigned char *pbuffer;
int rd_offset=0;
int wr_offset=0;
int buflen=0;

static DEFINE_SEMAPHORE(s1);

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
	int ret, rlen;
   	rlen=usize;
   
   if(buflen==0) {    //(wr_offset - rd_offset)==0
     printk("pseudo read -- buffer is empty\n");
     return 0;       //buffer is empty
   }    
   if(rlen > buflen)
       rlen = buflen;  // min of uspace request, buffer length
    
   ret=copy_to_user(ubuf, pbuffer, rlen);
   if(ret) {
     printk("Erorr inn copy_to_user,failed");
     return -EINVAL;
   }
   
   rd_offset += rlen;
   buflen -= rlen;
      
   printk("Pseudo--read method, transferred %d bytes\n",rlen);
   return rlen;
	
}
ssize_t psuedo_write(struct file* file,const char __user* ubuf, size_t usize, loff_t* off)
{
   int wlen, kremain,i;
   
   wlen=usize;
   kremain = MAX_SIZE - wr_offset;
   
   if(kremain==0)    // wr_offset >= MAX_SIZE
      return -ENOSPC;     //buffer is full
         
   if(wlen > kremain)
      wlen = kremain;    //min of user request, remaining space
      
   down_interruptible(&s1);
   for(i=0;i<wlen;i++)
   {
   	get_user(*(pbuffer+wr_offset),ubuf);
   	++wr_offset;
   	msleep(100);
   }
   up(&s1);
   
   printk("Pseudo--write method, transferred %d bytes\n",wlen);
   return wlen;
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
	sema_init(&s1,1);
	pbuffer = kmalloc(MAX_SIZE, GFP_KERNEL);
	pclass = class_create(THIS_MODULE,"psuedo_class");
	ret=alloc_chrdev_region(&pdevid,0,ndevices,"psuedo_sample");
	if(ret)
	{
		printk("Psuedo:Failed to register driver\n");
		return -EINVAL;
	}
	cdev_init(&cdev, &fops);
	kobject_set_name(&cdev.kobj,"pdevice%d",i);
	ret=cdev_add(&cdev,pdevid,1);
	printk("Successfully registered, major=%d,minor=%d\n",MAJOR(pdevid),MINOR(pdevid));
	pdev = device_create(pclass,NULL,pdevid,NULL,"psample%d",i);
	if(pdev==NULL)
	{
	printk("error creating device");
	}
	printk("Psuedo9 driver sample..welcome\n");
	return 0;
}

static void __exit psuedo_exit(void)
{
	cdev_del(&cdev);
	device_destroy(pclass,pdevid);
	kfree(pbuffer);
	unregister_chrdev_region(pdevid,ndevices);
	class_destroy(pclass);
	printk("Psuedo9 driver sample..byr\n");
	
}

module_init(psuedo_init);
module_exit(psuedo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arrijith");
MODULE_DESCRIPTION("Psuedo module");
