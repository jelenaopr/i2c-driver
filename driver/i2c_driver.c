#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/i2c.h>

#include "i2c_driver.h"

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

static struct i2c_client *i2c_client_device = NULL;

static const struct i2c_device_id i2c_driver_id[] = {
  { DEVICE_NAME, 0 },
  { }
};

static struct i2c_board_info i2c_device_board_info = {
    I2C_BOARD_INFO(DEVICE_NAME, SLAVE_ADDRESS)
};

MODULE_DEVICE_TABLE(i2c, i2c_driver_id);

/**
 * send_to_slave_device - Function used to send data or commands from the I2C master to the slave device.
 * @buffer: Pointer to the buffer which contains data that will be sent.
 * @len: Length of data which will be sent.
 *
 * Return: number of bytes sent on success, or a negative error code on failure.
 */
static int send_to_slave_device(unsigned char *buffer, unsigned int len)
{
    int ret = i2c_master_send(i2c_client_device, buffer, len);
    return ret;

}

/**
 * send_command_to_slave_device - Function used to send commands from the I2C master to the slave device.
 * @command: Command ID.
 *  
 * Return: number of bytes sent on success, or a negative error code on failure.
 */
static int send_command_to_slave_device(unsigned char command) {
    unsigned char buffer[] = {0x00, command};
    return send_to_slave_device(buffer, sizeof(buffer));
}

/**
 * send_data_to_slave_device - Function used to send data from the I2C master to the slave device.
 * @data: Data which will be sent to device.
 *  
 * Return: number of bytes sent on success, or a negative error code on failure.
 */
static int send_data_to_slave_device(unsigned char data) {
    unsigned char buffer[] = {0x40, data};
    return send_to_slave_device(buffer, sizeof(buffer));
}

/**
 * i2c_driver_clear_display - Function used to send data from the I2C master to the slave device.
 * @void: 
 *  
 * Return: None.
 */
static void i2c_driver_clear_display(void) {
    unsigned int i;
  
    for(i = 0; i < PAGES * SEGMENTS; i++)
    {
        send_data_to_slave_device(0x00);
    }
}

/**
 * i2c_driver_center_cursor - Function used to center the cursor.
 * @void: 
 *  
 * Return: None.
 */
static void i2c_driver_center_cursor(u8 line, u8 cursor) {
    send_command_to_slave_device(0x21);              // cmd for the column start and end address
    send_command_to_slave_device(cursor);         // column start addr
    send_command_to_slave_device(SEGMENTS-1); // column end addr

    send_command_to_slave_device(0x22);              // cmd for the page start and end address
    send_command_to_slave_device(line);            // page start addr
    send_command_to_slave_device(PAGES-1);
}

static void i2c_driver_print_char(unsigned char character) {

    character -= 0x20;
    u8 data = 0;
    u8 line = 0;

    if(character == 0x0A) {
        pr_info("new line detected\n");
        line = line & SEGMENTS;
        line ++;
        i2c_driver_center_cursor(line, 0x00);
    } else {
        for(u8 i = 0; i < FONT_SIZE; i++) {
            data = i2c_font_bitmap[character][i];
            send_data_to_slave_device(data);
        }
    }    
}


static void i2c_driver_print_string(const char *string)
{
    u8 line = 0;
    while(*string) {
        if (*string == '\n') {
            // Handle newline character: Maybe flush or change to a different line on the display.
            //i2c_driver_print_char('\n'); 
            pr_info("in print string new line");
            line ++;
           i2c_driver_center_cursor(line, 0x00);
            string++;
        } else
        {
            i2c_driver_print_char(*string++);
        }
        
    }
}
/**
 * send_to_slave_device - Function used to center the cursor.
 * @void: 
 *  
 * Return: None.
 */
static void i2c_driver_display_init(void) {
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_OFF);
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_CLOCK_DIVIDE);
    send_command_to_slave_device(0x80);
    send_command_to_slave_device(DISPLAY_CMD_MULTIPLEX_RATIO);
    send_command_to_slave_device(0x3F);
    send_command_to_slave_device(0xD3);
    send_command_to_slave_device(0x00);
    send_command_to_slave_device(0x40);
    send_command_to_slave_device(0x40);
    send_command_to_slave_device(0x8D);
    send_command_to_slave_device(0x14);
    send_command_to_slave_device(0x20);
    send_command_to_slave_device(0x00);
    send_command_to_slave_device(0xA1);
    send_command_to_slave_device(0xC8);
    send_command_to_slave_device(0xDA);
    send_command_to_slave_device(0x12);
    send_command_to_slave_device(0x81);
    send_command_to_slave_device(0x80);
    send_command_to_slave_device(0xD9);
    send_command_to_slave_device(0xF1);
    send_command_to_slave_device(0xDB);
    send_command_to_slave_device(0x20);
    send_command_to_slave_device(0xA4);
    send_command_to_slave_device(0xA6);
    send_command_to_slave_device(0x2E);
    send_command_to_slave_device(0xAF);

    i2c_driver_clear_display();

}

static int i2c_driver_probe(struct i2c_client *client){ 

    i2c_driver_display_init();
    i2c_driver_center_cursor(0x00, 0x00);
    
    pr_info("i2c_driver: driver probed!\n");
    return 0;
}

static void i2c_driver_remove(struct i2c_client *client){

}

static struct i2c_driver i2c_device_driver = {
    .driver = {
        .name   = DEVICE_NAME,
        .owner  = THIS_MODULE,
    },
    .probe          = i2c_driver_probe,
    .remove         = i2c_driver_remove,
    .id_table       = i2c_driver_id,
};

static int i2c_driver_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}


static int i2c_driver_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}


static long i2c_driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    char buffer[25];
         switch(cmd) {
                case IOCTL_CMD_WRITE:
                    if(copy_from_user(buffer ,(int32_t*) arg, sizeof(buffer)))
                    {
                        pr_err("Data Write : Err!\n");
                    }
                        pr_info("Buffer Contents: %s\n", buffer);

                        for (int i = 0; buffer[i] != '\0'; i++) {
                        pr_info("Character at %d: %c (ASCII: %d)\n", i, buffer[i], buffer[i]);

                        if (buffer[i] == '\n') {
                            pr_info("Newline character detected at position %d\n", i);
                        }
                    }
                        i2c_driver_print_string(buffer);
                    break;

                case IOCTL_CMD_CLEAR:
                    i2c_driver_center_cursor(0x00, 0x00);
                    i2c_driver_clear_display();
                    break;
                        
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
    .open = i2c_driver_open,
    .unlocked_ioctl = i2c_driver_ioctl,
    .release = i2c_driver_release
};


static int __init i2c_driver_init(void) {
    struct i2c_adapter *adapter;

    adapter = i2c_get_adapter(I2C_ADAPTER_ID);

    if(!adapter) {
        pr_err("i2c_driver: Failed to get I2C adapter\n");
        return -ENODEV;
    }

    i2c_client_device = i2c_new_client_device(adapter, &i2c_device_board_info);
    if(!i2c_client_device) {
        pr_err("i2c_driver: Failed to create I2C client\n");
        return -ENODEV;
    }

    i2c_add_driver(&i2c_device_driver);
    i2c_put_adapter(adapter);

    int ret = 0;
   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "i2c_device");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create("i2c_device_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "i2c_device");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}

    return 0;
  
	fail_2:
		device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);

    pr_info("i2c_driver: init complete!\n");

    return 0;
}

static void __exit i2c_driver_exit(void) {
    i2c_unregister_device(i2c_client_device);
    i2c_del_driver(&i2c_device_driver);
    cdev_del(my_cdev);
    device_destroy(my_class, my_dev_id);
    class_destroy(my_class);
    unregister_chrdev_region(my_dev_id,1);
    pr_info("i2c_driver: exit complete!\n");
}

module_init(i2c_driver_init);
module_exit(i2c_driver_exit);