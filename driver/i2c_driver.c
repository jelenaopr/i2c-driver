#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/i2c.h>

#define I2C_ADAPTER_ID 1
#define DEVICE_NAME    "I2C_DRIVER"
#define SLAVE_ADDRESS  0x3C
#define PAGES          8
#define SEGMENTS       128
#define FONT_SIZE      5

#define IOCTL_CMD_WRITE 0x00
#define IOCTL_CMD_CLEAR 0x01

#define DISPLAY_CMD_LOWER_COLUMN_START_ADDRESS              0x00        /**< command lower column start address */
#define DISPLAY_CMD_HIGHER_COLUMN_START_ADDRESS             0x10        /**< command higher column start address */
#define DISPLAY_CMD_MEMORY_ADDRESSING_MODE                  0x20        /**< command memory addressing mode */
#define DISPLAY_CMD_SET_COLUMN_ADDRESS                      0x21        /**< command set column address */
#define DISPLAY_CMD_SET_PAGE_ADDRESS                        0x22        /**< command set page address */
#define DISPLAY_CMD_SET_FADE_OUT_AND_BLINKING               0x23        /**< command set fade out and blinking */
#define DISPLAY_CMD_RIGHT_HORIZONTAL_SCROLL                 0x26        /**< command right horizontal scroll */
#define DISPLAY_CMD_LEFT_HORIZONTAL_SCROLL                  0x27        /**< command left horizontal scroll */
#define DISPLAY_CMD_VERTICAL_RIGHT_HORIZONTAL_SCROLL        0x29        /**< command vertical right horizontal scroll */
#define DISPLAY_CMD_VERTICAL_LEFT_HORIZONTAL_SCROLL         0x2A        /**< command vertical left horizontal scroll */
#define DISPLAY_CMD_DEACTIVATE_SCROLL                       0x2E        /**< command deactivate scroll */
#define DISPLAY_CMD_ACTIVATE_SCROLL                         0x2F        /**< command activate scroll */
#define DISPLAY_CMD_DISPLAY_START_LINE                      0x40        /**< command display start line */
#define DISPLAY_CMD_CONTRAST_CONTROL                        0x81        /**< command contrast control */
#define DISPLAY_CMD_CHARGE_PUMP_SETTING                     0x8D        /**< command charge pump setting */
#define DISPLAY_CMD_COLUMN_0_MAPPED_TO_SEG0                 0xA0        /**< command column 0 mapped to seg 0 */
#define DISPLAY_CMD_COLUMN_127_MAPPED_TO_SEG0               0xA1        /**< command column 127 mapped to seg 0 */
#define DISPLAY_CMD_VERTICAL_SCROLL_AREA                    0xA3        /**< command vertical scroll area */
#define DISPLAY_CMD_ENTIRE_DISPLAY_OFF                      0xA4        /**< command entire display off */ 
#define DISPLAY_CMD_ENTIRE_DISPLAY_ON                       0xA5        /**< command entire display on */ 
#define DISPLAY_CMD_NORMAL_DISPLAY                          0xA6        /**< command normal display */ 
#define DISPLAY_CMD_INVERSE_DISPLAY                         0xA7        /**< command inverse display */ 
#define DISPLAY_CMD_MULTIPLEX_RATIO                         0xA8        /**< command multiplex ratio */ 
#define DISPLAY_CMD_DISPLAY_OFF                             0xAE        /**< command display off */ 
#define DISPLAY_CMD_DISPLAY_ON                              0xAF        /**< command display on */ 
#define DISPLAY_CMD_PAGE_ADDR                               0xB0        /**< command page address */ 
#define DISPLAY_CMD_SCAN_DIRECTION_COM0_START               0xC0        /**< command scan direction com 0 start */ 
#define DISPLAY_CMD_SCAN_DIRECTION_COMN_1_START             0xC8        /**< command scan direction com n-1 start */ 
#define DISPLAY_CMD_DISPLAY_OFFSET                          0xD3        /**< command display offset */ 
#define DISPLAY_CMD_DISPLAY_CLOCK_DIVIDE                    0xD5        /**< command display clock divide */ 
#define DISPLAY_CMD_SET_ZOOM_IN                             0xD6        /**< command set zoom in */ 
#define DISPLAY_CMD_PRE_CHARGE_PERIOD                       0xD9        /**< command pre charge period */ 
#define DISPLAY_CMD_COM_PINS_CONF                           0xDA        /**< command com pins conf */ 
#define DISPLAY_CMD_COMH_DESLECT_LEVEL                      0xDB        /**< command comh deslect level */ 
#define DISPLAY_CMD_NOP                                     0xE3        /**< command nop */

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
#define IOCTL_WRITE_STRING _IOW('a', 0, char *)

const unsigned char i2c_font_bitmap[][FONT_SIZE] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},   // space
    {0x00, 0x00, 0x2f, 0x00, 0x00},   // !
    {0x00, 0x07, 0x00, 0x07, 0x00},   // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14},   // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},   // $
    {0x23, 0x13, 0x08, 0x64, 0x62},   // %
    {0x36, 0x49, 0x55, 0x22, 0x50},   // &
    {0x00, 0x05, 0x03, 0x00, 0x00},   // '
    {0x00, 0x1c, 0x22, 0x41, 0x00},   // (
    {0x00, 0x41, 0x22, 0x1c, 0x00},   // )
    {0x14, 0x08, 0x3E, 0x08, 0x14},   // *
    {0x08, 0x08, 0x3E, 0x08, 0x08},   // +
    {0x00, 0x00, 0xA0, 0x60, 0x00},   // ,
    {0x08, 0x08, 0x08, 0x08, 0x08},   // -
    {0x00, 0x60, 0x60, 0x00, 0x00},   // .
    {0x20, 0x10, 0x08, 0x04, 0x02},   // /

    {0x3E, 0x51, 0x49, 0x45, 0x3E},   // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},   // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},   // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},   // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},   // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},   // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},   // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},   // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},   // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},   // 9

    {0x00, 0x36, 0x36, 0x00, 0x00},   // :
    {0x00, 0x56, 0x36, 0x00, 0x00},   // ;
    {0x08, 0x14, 0x22, 0x41, 0x00},   // <
    {0x14, 0x14, 0x14, 0x14, 0x14},   // =
    {0x00, 0x41, 0x22, 0x14, 0x08},   // >
    {0x02, 0x01, 0x51, 0x09, 0x06},   // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E},   // @

    {0x7C, 0x12, 0x11, 0x12, 0x7C},   // A
    {0x7F, 0x49, 0x49, 0x49, 0x36},   // B
    {0x3E, 0x41, 0x41, 0x41, 0x22},   // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},   // D
    {0x7F, 0x49, 0x49, 0x49, 0x41},   // E
    {0x7F, 0x09, 0x09, 0x09, 0x01},   // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A},   // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},   // H
    {0x00, 0x41, 0x7F, 0x41, 0x00},   // I
    {0x20, 0x40, 0x41, 0x3F, 0x01},   // J
    {0x7F, 0x08, 0x14, 0x22, 0x41},   // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},   // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},   // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},   // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},   // O
    {0x7F, 0x09, 0x09, 0x09, 0x06},   // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E},   // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},   // R
    {0x46, 0x49, 0x49, 0x49, 0x31},   // S
    {0x01, 0x01, 0x7F, 0x01, 0x01},   // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},   // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},   // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F},   // W
    {0x63, 0x14, 0x08, 0x14, 0x63},   // X
    {0x07, 0x08, 0x70, 0x08, 0x07},   // Y
    {0x61, 0x51, 0x49, 0x45, 0x43},   // Z

    {0x00, 0x7F, 0x41, 0x41, 0x00},   // [
    {0x55, 0xAA, 0x55, 0xAA, 0x55},   // Backslash (Checker pattern)
    {0x00, 0x41, 0x41, 0x7F, 0x00},   // ]
    {0x04, 0x02, 0x01, 0x02, 0x04},   // ^
    {0x40, 0x40, 0x40, 0x40, 0x40},   // _
    {0x00, 0x03, 0x05, 0x00, 0x00},   // `

    {0x20, 0x54, 0x54, 0x54, 0x78},   // a
    {0x7F, 0x48, 0x44, 0x44, 0x38},   // b
    {0x38, 0x44, 0x44, 0x44, 0x20},   // c
    {0x38, 0x44, 0x44, 0x48, 0x7F},   // d
    {0x38, 0x54, 0x54, 0x54, 0x18},   // e
    {0x08, 0x7E, 0x09, 0x01, 0x02},   // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C},   // g
    {0x7F, 0x08, 0x04, 0x04, 0x78},   // h
    {0x00, 0x44, 0x7D, 0x40, 0x00},   // i
    {0x40, 0x80, 0x84, 0x7D, 0x00},   // j
    {0x7F, 0x10, 0x28, 0x44, 0x00},   // k
    {0x00, 0x41, 0x7F, 0x40, 0x00},   // l
    {0x7C, 0x04, 0x18, 0x04, 0x78},   // m
    {0x7C, 0x08, 0x04, 0x04, 0x78},   // n
    {0x38, 0x44, 0x44, 0x44, 0x38},   // o
    {0xFC, 0x24, 0x24, 0x24, 0x18},   // p
    {0x18, 0x24, 0x24, 0x18, 0xFC},   // q
    {0x7C, 0x08, 0x04, 0x04, 0x08},   // r
    {0x48, 0x54, 0x54, 0x54, 0x20},   // s
    {0x04, 0x3F, 0x44, 0x40, 0x20},   // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C},   // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C},   // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C},   // w
    {0x44, 0x28, 0x10, 0x28, 0x44},   // x
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C},   // y
    {0x44, 0x64, 0x54, 0x4C, 0x44},   // z

    {0x00, 0x10, 0x7C, 0x82, 0x00},   // {
    {0x00, 0x00, 0xFF, 0x00, 0x00},   // |
    {0x00, 0x82, 0x7C, 0x10, 0x00},   // }
    {0x00, 0x06, 0x09, 0x09, 0x06}    // ~ (Degrees)
};


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