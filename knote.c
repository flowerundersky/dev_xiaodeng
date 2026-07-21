#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

#define KNOTE_DEVICE_NAME "knote"
#define KNOTE_CLASS_NAME  "knote"

static dev_t knote_devno;
static struct cdev knote_cdev;
static struct class *knote_class;
static struct device *knote_device;

static const struct file_operations knote_fops={
	.owner=THIS_MODULE,
	};

static int __init knote_init(void)
{
	int ret; 
	ret= alloc_chadev_region(&knote_devno,0,1,KNOTE_DEVICE_NAME);
	if(ret<0){
		pr_err("knote: alloc_chrdev_region fail:%d\n",ret);
		return ret;
	}
	cdev-init(&knote_cdev,&knote_fops);
	knote_cdev.owner=THIS_MODULE;

	ret=cdev_add(&knote_cdev,knote,1);
	if(ret<0){
		pr_err("knote: cdev_add  fail:%d\n",ret);
		goto err_unregister_chrdev;
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
		MONOR(knote_devno));
	pr_info("knote:/dev/%s created\n",KNOTE_DEVICE_NAME);
	return 0;
	
err_class_destroy:
	class_destory(knote_class);

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
MODULE_AUTHOR("your_nmae");
MODULE_DESCRIPTION("Minimal knote kernel moudle");
MODULE_VERSION("0.2");
