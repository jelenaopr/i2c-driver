#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	
#include <linux/types.h>

#include <linux/i2c.h>

#define I2C_ADAPTER_ID 1
#define DEVICE_NAME    "I2C_DRIVER"
#define SLAVE_ADDRESS  0x3C

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
 * send_to_slave_device - Function used to send data from the I2C master to the slave device.
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

static int send_command_to_slave_device(unsigned char command) {
    unsigned char buffer[2];
    int ret = 0;
    buffer[0] = 0x00;
    buffer[1] = command;

    ret = send_to_slave_device(buffer, 2);
    return ret;
}

static int send_data_to_slave_device(unsigned char data) {
    unsigned char buffer[2];
    int ret = 0;
    buffer[0] = 0x40;
    buffer[1] = data;

    ret = send_to_slave_device(buffer, 2);
    return ret;
}

static void i2c_driver_clear_display(void) {
    unsigned int total  = 128 * 8;  // 8 pages x 128 segments x 8 bits of data
    unsigned int i      = 0;
  
    for(i = 0; i < total; i++)
    {
        send_data_to_slave_device(0x00);
    }
}

static void i2c_driver_center_cursor(void) {
    send_command_to_slave_device(0x21);              // cmd for the column start and end address
    send_command_to_slave_device(0x00);         // column start addr
    send_command_to_slave_device(128-1); // column end addr

    send_command_to_slave_device(0x22);              // cmd for the page start and end address
    send_command_to_slave_device(0x00);            // page start addr
    send_command_to_slave_device(7);
}

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
    i2c_driver_center_cursor();
    
    send_data_to_slave_device(0x14);
    send_data_to_slave_device(0x7f);
    send_data_to_slave_device(0x14);
    send_data_to_slave_device(0x7f);
    send_data_to_slave_device(0x14);
    
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

    pr_info("i2c_driver: init complete!\n");

    return 0;
}

static void __exit i2c_driver_exit(void) {
    i2c_unregister_device(i2c_client_device);
    i2c_del_driver(&i2c_device_driver);
    pr_info("i2c_driver: exit complete!\n");
}

module_init(i2c_driver_init);
module_exit(i2c_driver_exit);