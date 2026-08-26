#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define KNOTE_DEVICE_NAME "knote"
#define KNOTE_CLASS_NAME  "knote"
#define KNOTE_BUFFER_SIZE 4096


static dev_t knote_devno;
static struct cdev knote_cdev;
static struct class *knote_class;
static struct device *knote_device;
static char knote_buffer[KNOTE_BUFFER_SIZE];
static size_t knote_size;
static DEFINE_MUTEX(knote_lock);

static const struct file_operations knote_fops={
	.owner=THIS_MODULE,
	.open=knote_open,
	.release=knote_release,
	.write=knote_write,
	.read=knote_read,
	};

static int knote_open(struct inode *inode, struct file *file)
{
	pr_info("knote: open\n");
	return 0;
	}

static int knote_release(struct inode *inode, struct file *file)
{
	pr_info("knote: release\n");
	return 0;
	}

static ssize_t knote_write(struct file *file, const char __user*user_buffer,size_t count, loff_t *ppos)
{
	ssize_t ret;
	if(count>=KNOTE_BUFFER_SIZE)
		count= KNOTE_BUFFER_SIZE-1;
		
	if(mutex_lock_interruptible(&knote_lock))
		return -ERESTARTSYS;

	memset(knote_buffer,0,sizeof(knote_buffer));	
	
	if(copy_from_user(knote_buffer,user_buffer,count)){
		ret= -EFAULT;
		goto out;
		}
	knote_buffer[count]='\0';
	knote_size=count;
	*ppos += count;
	ret=count;

	pr_info("knote: write %zu bytes\n",count);

out:
	mutex_unlock(&knote_lock);
	return ret;
	}





static ssize_t knote_read(struct file *file, char __user*user_buffer,size_t count, loff_t *ppos)
{
	ssize_t ret;
	
	if(mutex_lock_interruptible(&knote_lock))
		return -ERESTARTSYS;

	if(*ppos>=knote_size){
		ret=0;
		goto out;
		}
	if(count>knote_size- *ppos)
		count=knote_size- *ppos;	
	
	if(copy_to_user(knote_buffer,user_buffer+ *ppos,count)){
		ret= -EFAULT;
		goto out;
		}
	*ppos += count;
	ret=count;

	pr_info("knote: read %zu bytes\n",count);

out:
	mutex_unlock(&knote_lock);
	return ret;
	}



static int __init knote_init(void)
{
	int ret; 
	ret= alloc_chrdev_region(&knote_devno,0,1,KNOTE_DEVICE_NAME);
	if(ret<0){
		pr_err("knote: alloc_chrdev_region fail:%d\n",ret);
		return ret;
	}
	cdev_init(&knote_cdev,&knote_fops);
	knote_cdev.owner=THIS_MODULE;

	ret=cdev_add(&knote_cdev,knote_devno,1);
	if(ret<0){
		pr_err("knote: cdev_add  fail:%d\n",ret);
		goto err_unregister_chrdev;
	}
	
	knote_class=class_create(THIS_MODULE,KNOTE_CLASS_NAME);
	if(IS_ERR(knote_class)){
		ret=PTR_ERR(knote_class);
		pr_err("knote:device_create failed:%d\n",ret);
		goto err_cdev_del;
	}

	knote_device=device_create(
		knote_class,
		NULL,
		knote_devno,
		NULL,
		KNOTE_DEVICE_NAME
	);

	if(IS_ERR(knote_device)){
		ret=PTR_ERR(knote_device);
		pr_err("knote:device_create failed:%d\n",ret);
		goto err_class_destroy;
	}
	pr_info("knote:moudle loaded\n");
	pr_info("knote:major=%d minor=%d\n",
		MAJOR(knote_devno),
		MINOR(knote_devno));
	pr_info("knote:/dev/%s created\n",KNOTE_DEVICE_NAME);
	return 0;
	
err_class_destroy:
	class_destroy(knote_class);

err_cdev_del:
	cdev_del(&knote_cdev);

err_unregister_chrdev:
	unregister_chrdev_region(knote_devno,1);

	return ret;
}

static void __exit knote_exit(void)
{
	device_destroy(knote_class,knote_devno);
	class_destroy(knote_class);
	cdev_del(&knote_cdev);
	unregister_chrdev_region(knote_devno,1);
	pr_info("knote:/dev/%s destroyed\n",KNOTE_DEVICE_NAME);
	pr_info("knote: module unload\n");	
}

module_init(knote_init);
module_exit(knote_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gold_star");
MODULE_DESCRIPTION("Minimal knote kernel moudle");
MODULE_VERSION("V26.0.2");
