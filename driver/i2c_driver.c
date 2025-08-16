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
 * i2c_driver_set_cursor - Function used to set the cursor.
 * @line: Cursor will be set starting from this column address.
 * @cursor: Cursor will be set starting from this page address.
 *  
 * Return: None.
 */
static void i2c_driver_set_cursor(u8 line, u8 cursor) {
    send_command_to_slave_device(0x21);              // cmd for the column start and end address
    send_command_to_slave_device(cursor);         // column start addr
    send_command_to_slave_device(SEGMENTS-1); // column end addr

    send_command_to_slave_device(0x22);              // cmd for the page start and end address
    send_command_to_slave_device(line);            // page start addr
    send_command_to_slave_device(PAGES-1);
}

/**
 * i2c_driver_print_char - Function used to print a single char on display.
 * @character: Single char which will be printed on screen by matching it's ASCII value to the font bitmap. 
 *  
 * Return: None.
 */
static void i2c_driver_print_char(unsigned char character) {

    u8 data = 0;
    
    /* Adjust ASCII value of character to map it to correct index in font bitmap */
    character -= 0x20;

    for(u8 i = 0; i < FONT_SIZE; i++) {
        data = i2c_font_bitmap[character][i];
        send_data_to_slave_device(data);
    }    
}

/**
 * i2c_driver_print_string - Function used to print a complete string on display.
 * @string: Pointer to a string which will be sent to slave device. 
 *  
 * Return: None.
 */
static void i2c_driver_print_string(const char *string)
{
    u8 line = 0;
    while(*string) {
        if (*string == '\n') {
            line ++;
            string++;
            i2c_driver_set_cursor(line, 0x00);
        } else
        {
            i2c_driver_print_char(*string++);
        }
    }
}

static void i2c_driver_set_brightness(uint8_t percentage) 
{
    send_command_to_slave_device(0x81);
    send_command_to_slave_device(percentage);
}

/**
 * i2c_driver_display_init - Function used to initialize the display.
 * @void:
 *  
 * Return: None.
 */
static void i2c_driver_display_init(void) {
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_OFF);
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_CLOCK_DIVIDE);
    send_command_to_slave_device(DISPLAY_CMD_DFLT_CLK_OSCI_FREQ);
    send_command_to_slave_device(DISPLAY_CMD_MULTIPLEX_RATIO);
    send_command_to_slave_device(DISPLAY_CMD_DFLT_COM_LINES);
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_OFFSET);
    send_command_to_slave_device(DISPLAY_CMD_LOWER_COLUMN_START_ADDRESS);
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_START_LINE);
    send_command_to_slave_device(DISPLAY_CMD_CHARGE_PUMP_SETTING);
    send_command_to_slave_device(DISPLAY_CMD_EN_CHARGE);
    send_command_to_slave_device(DISPLAY_CMD_MEMORY_ADDRESSING_MODE);
    send_command_to_slave_device(DISPLAY_CMD_LOWER_COLUMN_START_ADDRESS);
    send_command_to_slave_device(DISPLAY_CMD_COLUMN_127_MAPPED_TO_SEG0);
    send_command_to_slave_device(DISPLAY_CMD_SCAN_DIRECTION_COMN_1_START);
    send_command_to_slave_device(DISPLAY_CMD_COM_PINS_CONF);
    send_command_to_slave_device(DISPLAY_CMD_ALT_PIN_CONFIG);
    send_command_to_slave_device(DISPLAY_CMD_CONTRAST_CONTROL);
    send_command_to_slave_device(DISPLAY_CMD_DFLT_CLK_OSCI_FREQ);
    send_command_to_slave_device(DISPLAY_CMD_PRE_CHARGE_PERIOD);
    send_command_to_slave_device(DISPLAY_CMD_PHASE_1_PHASE_2);
    send_command_to_slave_device(DISPLAY_CMD_COMH_DESLECT_LEVEL);
    send_command_to_slave_device(DISPLAY_CMD_MEMORY_ADDRESSING_MODE);
    send_command_to_slave_device(DISPLAY_CMD_ENTIRE_DISPLAY_OFF);
    send_command_to_slave_device(DISPLAY_CMD_NORMAL_DISPLAY);
    send_command_to_slave_device(DISPLAY_CMD_DEACTIVATE_SCROLL);
    send_command_to_slave_device(DISPLAY_CMD_DISPLAY_ON);

    i2c_driver_clear_display();
}

static int i2c_driver_probe(struct i2c_client *client){ 

    i2c_driver_display_init();
    i2c_driver_set_cursor(0x00, 0x00);
    
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
        switch(cmd)
        {
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
                if (strncmp(buffer, "brightness=", 11) == 0) {
                    int value;
                    uint8_t perc = 0;
                    int ret = 0;
                    ret = kstrtoint(buffer + 11, 10, &value);  // parse the number after '='
    
                    if (ret < 0 || value < 0 || value > 256) {
                        pr_err("Invalid brightness value\n");
                        return -EINVAL;
                    }
                    else
                    {
                        pr_err("i2c_driver: set brighness as %d\n", value);
                        perc = (uint8_t)value;
                        i2c_driver_set_brightness(value);
                    }
                }
                else
                {
                    i2c_driver_print_string(buffer);
                }
                break;

            case IOCTL_CMD_CLEAR:
                i2c_driver_set_cursor(0x00, 0x00);
                i2c_driver_clear_display();
                break;
                    
            default:
                pr_err("i2c_driver: Default state, should not enter here!\n");
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
    int ret = 0;

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

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "i2c_device");
   if (ret) {
      pr_err("i2c_driver: failed to register char device\n");
      return ret;
   }
   pr_info("i2c_driver:char device region allocated\n");

   my_class = class_create("i2c_device_class");
   if (my_class == NULL) {
      pr_err("i2c_driver: failed to create class\n");
      unregister_chrdev_region(my_dev_id, 1);
   }
   pr_info("i2c_driver: class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "i2c_device");
   if (my_device == NULL) {
        pr_err("i2c_driver: failed to create device\n");
        class_destroy(my_class);
   }
   pr_info("i2c_driver: device created\n");

    my_cdev = cdev_alloc();	
    my_cdev->ops = &my_fops;
    my_cdev->owner = THIS_MODULE;
    ret = cdev_add(my_cdev, my_dev_id, 1);
    if(ret) {
        pr_err("i2c_driver: failed to add cdev\n");
        device_destroy(my_class, my_dev_id);
    }

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