#include <linux/module.h>     /* Needed by all modules */
#include <linux/kernel.h>     /* Needed for KERN_INFO */
#include <linux/init.h>       /* Needed for the macros */

#include <linux/version.h> 

#include <linux/types.h> 
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h> 
#include <linux/cdev.h> 
#include <linux/uaccess.h> 
#include <linux/random.h> 

#define MAX_DEV 1   
#define MAX_LEN 256     


static struct class *chardev_class = NULL;

static struct cdev myChardev[MAX_DEV];
static int myMajor = 0;


static int myOpen(struct inode *inode, struct file *file);
static int myRelease(struct inode *inode, struct file *file);
static ssize_t myRead(struct file *file, char __user *buf, size_t len, loff_t *offset);


static const struct file_operations pugs_fops = 
{
    .owner      = THIS_MODULE,
    .open       = myOpen,
    .release    = myRelease,
    .read       = myRead
};


static int myOpen(struct inode *inode, struct file *file) 
{
    printk("My Character Device >> Open\n");
    return 0;
}

static int myRelease(struct inode *inode, struct file *file)
{
    printk("My Character Device >> Release\n");
    return 0;
}

static int randomFunc(void) 
{
    short randNum;
    get_random_bytes(&randNum, sizeof(randNum)); 
    return randNum; 
}


static ssize_t myRead(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    char data[MAX_LEN] = {0};
    
    ssize_t bytes = len < (MAX_LEN-(*offset)) ? len : (MAX_LEN-(*offset));
    
    int randNum = randomFunc();
    sprintf(data, "%d \n", randNum);

    if (copy_to_user(buf, &data, bytes)) 
    {
        return -EFAULT;
    }
    (*offset) += bytes;

    return bytes;
}


static int __init init_func(void)
{
    int i;
    dev_t myDeviceType;  

    if(alloc_chrdev_region(&myDeviceType, 0, MAX_DEV, "chardev")<0)
        return -1;
    
    chardev_class = class_create(THIS_MODULE, "chardev");
    if (chardev_class==NULL)
    {
        unregister_chrdev_region(myDeviceType, MINORMASK);
        return -1;
    }
    
    myMajor = MAJOR(myDeviceType);

    for (i = 0; i < MAX_DEV; i++)
    {
        cdev_init(&myChardev[i], &pugs_fops);
        myChardev[i].owner = THIS_MODULE;

        if(cdev_add(&myChardev[i], MKDEV(myMajor, i), 1))
        {
            device_destroy(chardev_class, myDeviceType);
            class_destroy(chardev_class);
            unregister_chrdev_region(myDeviceType, MINORMASK);
            return -1;
        }
        
        if(device_create(chardev_class, NULL, MKDEV(myMajor, i), NULL, "chardev%d", i)==NULL)
        {
            class_destroy(chardev_class);
            unregister_chrdev_region(myDeviceType, MINORMASK);
            return -1;
        }
    }
    
    printk("Initializing...\n");
    return 0;
}


static void __exit exit_func(void) 
{
    int i;
    cdev_del(myChardev);

    for (i = 0; i < MAX_DEV; i++)
    {
        device_destroy(chardev_class, MKDEV(myMajor, i));
    }
    class_destroy(chardev_class);
    unregister_chrdev_region(MKDEV(myMajor, 0), MINORMASK);
    
    printk("Removing...");
}


MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("goporo");
MODULE_DESCRIPTION("Module dung detao so ngau nhien");         
MODULE_SUPPORTED_DEVICE("Character device");            


module_init(init_func);
module_exit(exit_func); 
