/* Necessary includes for drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "gpio_lkm.h"

MODULE_LICENSE("Dual BSD/GPL");

#define DEVICE_NAME "seven7segment"

#define MY_MAX_MINORS 1

#define A   4
#define B   5
#define C   6
#define D   12
#define E   13
#define F   16
#define G   17
#define H   18

int all_pins[8] = {A, B, C, D, E, F, G, H};
int number_1[2] = {B, C};
int number_2[6] = {A, B, D, E, G, H};
int number_3[6] = {A, B, C, D, G, H};
int number_4[5] = {B, C, F, G, H};
int number_5[6] = {A, C, D, F, G, H};
int number_6[7] = {A, C, D, E, F, G, H};
int number_7[3] = {A, B, C};
int number_8[8] = {A, B, C, D, E, F, G, H};
int number_9[7] = {A, B, C, D, F, G, H};

#define BUF_LEN 16
static char message[BUF_LEN];
static char *msg_p;

//buffer to store data
char * memory_buffer;


static int  seven7segment_open(struct inode *inode, struct file *filp);
static int  seven7segment_release(struct inode *inode, struct file *filp);
static ssize_t  seven7segment_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t seven7segment_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static void __exit seven7segment_exit(void);
static int __init  seven7segment_init(void);


/* file access functions */
struct file_operations seven7segment_fops = {
        .read =     seven7segment_read,
        .write =    seven7segment_write,
        .open =     seven7segment_open,
        .release =  seven7segment_release
};

struct my_device_data {
    struct cdev cdev;
    /* my data starts here */
    //...
};

struct my_device_data devs[MY_MAX_MINORS];

/* Driver global variables */
/* Major number */
int seven7segment_major = 62;

module_init(seven7segment_init);
module_exit(seven7segment_exit);

static int __init  seven7segment_init(void) {
    int result;

    // register char device
    result = register_chrdev(seven7segment_major, DEVICE_NAME,
                             &seven7segment_fops);
    if(result < 0) {
        printk("[seven7segment]: error obtaining major number %d\n",
               seven7segment_major);
        return result;
    }

    printk("[seven7segment]: registered device with major number %d\n",
           seven7segment_major);

    printk("GET RESULT: GPIO_LKM_WRITE_PIN %d\n", gpio_lkm_write_pin(0, true));
    printk("[seven7segment] - Inserting module\n");
    return 0;
}

static void __exit seven7segment_exit(void) {
    unregister_chrdev(seven7segment_major, DEVICE_NAME);

    /* Freeing buffer memory */
    if (memory_buffer) {
        kfree(memory_buffer);
    }

    printk("[seven7segment] - Removing module\n");
}

static int seven7segment_open(struct inode *inode, struct file *file){
    struct my_device_data *my_data;
    my_data = container_of(inode->i_cdev, struct my_device_data, cdev);
    file->private_data = my_data;
    return 0;
}

static int seven7segment_release(struct inode *inode, struct file *filp) {
    /* Success */
    return 0;
}

static ssize_t seven7segment_read(struct file *file, char * buf,size_t size, loff_t *f_pos) {
    /* Transfering data to user space */
    copy_to_user(buf, memory_buffer,1);
    /* Changing reading position as best suits */
    if (*f_pos == 0) {
        *f_pos+=1;
        return 1;
    } else {
        return 0;
    }
}

static ssize_t seven7segment_write( struct file *filp, const char *ubuf, size_t count, loff_t *f_pos) {
    /* Buffer writing to the device */
    char *kbuf = kcalloc((count + 1), sizeof(char), GFP_KERNEL);

    if(copy_from_user(kbuf, ubuf, count) != 0) {
        kfree(kbuf);
        return -EFAULT;
    }

    kbuf[count-1] = 0;

    kfree(kbuf);

    return (ssize_t)count;
}