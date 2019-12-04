/* Necessary includes for drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/kthread.h>    // Using kthreads for the flashing functionality


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sofiia Tesliuk <sofiia.tesliuk@gmail.com>");
MODULE_DESCRIPTION("Seven segment driver - Linux device driver for Raspberry Pi");


#define DEVICE_NAME "seven7segment"

#define MY_MAX_MINORS 1
#define PINS_NUMBER 8

#define A   4
#define B   5
#define C   6
#define D   12
#define E   13
#define F   16
#define G   17
#define H   18


int all_pins[8] = {A, B, C, D, E, F, G, H};
int number_0[2] = {A, B, C, D, E, F};
int number_1[2] = {B, C};
int number_2[6] = {A, B, D, E, G, H};
int number_3[6] = {A, B, C, D, G, H};
int number_4[5] = {B, C, F, G, H};
int number_5[6] = {A, C, D, F, G, H};
int number_6[7] = {A, C, D, E, F, G, H};
int number_7[3] = {A, B, C};
int number_8[8] = {A, B, C, D, E, F, G, H};
int number_9[7] = {A, B, C, D, F, G, H};

int* numbers[10] = {number_0, number_1, number_2, number_3, number_4,
                  number_5, number_6, number_7, number_8, number_9};

bool active_mode = true; // Default mode is active
int current_digit = 0; // Default number is 0


void sleep_mode(void);
void set_digit(int digit);

static ssize_t seven7segment_show_digit(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    return sprintf(buf, "%d\n", current_digit);
}

static ssize_t seven7segment_store_digit(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
    if (strncmp(buf, "0", count-1)==0) { set_digit(0); }
    else if (strncmp(buf,"1",count-1)==0) { set_digit(1);}
    else if (strncmp(buf,"2",count-1)==0) { set_digit(2);}
    else if (strncmp(buf,"3",count-1)==0) { set_digit(3);}
    else if (strncmp(buf,"4",count-1)==0) { set_digit(4);}
    else if (strncmp(buf,"5",count-1)==0) { set_digit(5);}
    else if (strncmp(buf,"6",count-1)==0) { set_digit(6);}
    else if (strncmp(buf,"7",count-1)==0) { set_digit(7);}
    else if (strncmp(buf,"8",count-1)==0) { set_digit(8);}
    else if (strncmp(buf,"9",count-1)==0) { set_digit(9);}
    else{
        printk("[seven7segment] - Invalid digit.\n");
    }
    return count;
}

static ssize_t seven7segment_show_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    if (active_mode)
        return sprintf(buf, "active\n");
    else
        return sprintf(buf, "sleep\n");
}

static ssize_t seven7segment_store_mode(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
    if (strncmp(buf, "a", count-1)==0) { active_mode = true; set_digit(current_digit); }
    else if (strncmp(buf,"s",count-1)==0) { active_mode = false; sleep_mode(); }
    else{
        printk("[seven7segment] - Invalid mode.\n");
    }
    return count;
}

static struct kobj_attribute digit_attr = __ATTR(digit, 0660, seven7segment_show_digit, seven7segment_store_digit);
static struct kobj_attribute mode_attr = __ATTR(mode, 0660, seven7segment_show_mode, seven7segment_store_mode);

static struct attribute *seven7segment_attrs[] = {
        &digit_attr.attr,
        &mode_attr.attr,
        NULL,
};

static struct attribute_group attr_group = {
        .name  = DEVICE_NAME,                        // The name is generated in ebbLED_init()
        .attrs = seven7segment_attrs,                      // The attributes array defined just above
};

static struct kobject *seven7segment_kobj;            // The pointer to the kobject


void sleep_mode(){
    int g;
    for (g = 0; g < PINS_NUMBER; g++){
        gpio_set_value(all_pins[g], false);
    }
}

void set_digit(int digit){
    if ((digit < 0) || (digit > 9)){
        printk("[seven7segment] - Digit is out of range.\n");
        return;
    }

    sleep_mode();

    current_digit = digit;

    if (active_mode){
        int* cur_num = numbers[digit];

        while(*cur_num){
            gpio_set_value(*cur_num, true);
            cur_num++;
        }

        printk("[seven7segment] - Set digit %d\n", digit);
    }
}


static int __init  seven7segment_init(void);
static void __exit seven7segment_exit(void);


static int __init  seven7segment_init(void) {
    int result = 0;

    seven7segment_kobj = kobject_create_and_add(DEVICE_NAME, kernel_kobj->parent);

    if(!seven7segment_kobj) {
        printk("[seven7segment]: error creating kobj\n");
        return -ENOMEM;
    }

    result = sysfs_create_group(seven7segment_kobj, &attr_group);
    if(result) {
        printk(KERN_ALERT "[seven7segment]: failed to create sysfs group\n");
        kobject_put(seven7segment_kobj);                // clean up -- remove the kobject sysfs entry
        return result;
    }


    int g;
    char *pins_name = { "A", "B", "C", "D", "E", "F", "G", "H" };
    for (g = 0; g < PINS_NUMBER; g++) {
        gpio_request(all_pins[g], &pins_name[g]);
    }
    for (g = 0; g < PINS_NUMBER; g++) {
        gpio_direction_output(all_pins[g], 0);
    }

    set_digit(0);

    printk("[seven7segment] - Inserting module.\n");
    return 0;
}

static void __exit seven7segment_exit(void) {
    sleep_mode();

    kobject_put(seven7segment_kobj);

    int g;
    for (g = 0; g < PINS_NUMBER; g++) {
        gpio_free(all_pins[g]);
    }

    printk("[seven7segment] - Removing module.\n");
}

module_init(seven7segment_init);
module_exit(seven7segment_exit);
