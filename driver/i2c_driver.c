#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	
#include <linux/types.h>

#include <linux/i2c.h>

#define I2C_ADAPTER_ID 1
#define DEVICE_NAME    "I2C_DRIVER"
#define SLAVE_ADDRESS  0x3C

MODULE_LICENSE("Dual BSD/GPL");



static struct i2c_client *i2c_client_device = NULL;

static const struct i2c_device_id i2c_driver_id[] = {
  { DEVICE_NAME, 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, i2c_driver_id);

static struct i2c_board_info i2c_device_board_info = {
    I2C_BOARD_INFO(DEVICE_NAME, SLAVE_ADDRESS)
};

static int i2c_probe(struct i2c_client *client){ 

    return 0;
}

static void i2c_remove(struct i2c_client *client){

}

static struct i2c_driver i2c_device_driver = {
    .driver = {
        .name   = DEVICE_NAME,
        .owner  = THIS_MODULE,
    },
    .probe          = i2c_probe,
    .remove         = i2c_remove,
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