#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uacce.h>
#include <linux/acpi.h>

#define WMI_BUFFER_OFFSET 25
#define HPWMI_FOURZONE_COLOR_SET 3
#define HPWMI_FOURZONE 131081

#define HPWMI_BIOS_GUID "5FB7F034-2C63-45e9-BE91-3D44E2C707E4"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TitanHZZ");
MODULE_DESCRIPTION("HP Omen four zone rgb laptop keyboard driver.");

static dev_t dev;
static struct cdev cdev;
static struct class *dev_class;

struct bios_args
{
    u32 signature;
    u32 command;
    u32 commandtype;
    u32 datasize;
    u8 data[128];
};

static void hp_wmi_perform_query(int query, int command, void *buffer, int insize)
{
    struct bios_args args = {
        .signature = 0x55434553,
        .command = command,
        .commandtype = query,
        .datasize = insize,
        .data = {0},
    };

    // WMI input buffer
    struct acpi_buffer input = {sizeof(struct bios_args), &args};

    // copy input data buffer to bios args data buffer
    memcpy(&args.data[0], buffer, insize);

    // make the WMI 'request'
    wmi_evaluate_method(HPWMI_BIOS_GUID, 0, 3, &input, NULL);
}

static ssize_t dev_write(struct file *fp, const char *buffer, size_t len, loff_t *offset)
{
    // check if we have the correct amount of data
    if (len == 36)
    {
        int res, red_color, green_color, blue_color;

        u8 state[128];            // wmi buffer
        char *p = (char *)buffer; // loop over data pointer
        int zone_offset = 0;      // state buffer zone offset

        while (p < buffer + len - 1)
        {
            char red[4] = {0, 0, 0, 0};
            char green[4] = {0, 0, 0, 0};
            char blue[4] = {0, 0, 0, 0};

            res |= copy_from_user(red, p, 3);
            p += 3;
            res |= copy_from_user(green, p, 3);
            p += 3;
            res |= copy_from_user(blue, p, 3);
            p += 3;

            res |= kstrtoint(red, 10, &red_color);
            res |= kstrtoint(green, 10, &green_color);
            res |= kstrtoint(blue, 10, &blue_color);

            // something is wrong with the colors or the str to int convertion
            if (res != 0 || red_color < 0 || red_color > 255 || green_color < 0 || green_color > 255 || blue_color < 0 || blue_color > 255)
                return len;

            // bios wmi buffer for the zones start at offset 25
            state[WMI_BUFFER_OFFSET + zone_offset + 0] = (u8)red_color;
            state[WMI_BUFFER_OFFSET + zone_offset + 1] = (u8)green_color;
            state[WMI_BUFFER_OFFSET + zone_offset + 2] = (u8)blue_color;

            // get to next zone in the buffer
            zone_offset += 3;
        }

        // set the new color to the zone
        hp_wmi_perform_query(HPWMI_FOURZONE_COLOR_SET, HPWMI_FOURZONE, &state, sizeof(state));
    }

    return len;
}

static ssize_t phantom_dev_read(struct file *fp, char *buffer, size_t len, loff_t *offset) { return 0; }
static int phantom_dev_open_release(struct inode *inode, struct file *fp) { return 0; }

const struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = dev_write,
    .read = phantom_dev_read,
    .open = phantom_dev_open_release,
    .release = phantom_dev_open_release,
};

static int hp_omen_module_init(void)
{
    // check if the current system has hardware support for this driver
    int result = wmi_has_guid(HPWMI_BIOS_GUID);

    // exit with error code if there is no support
    if (!result)
    {
        printk("[Omen laptop driver] Driver is not supported by the hardware!");
        return -ENODEV;
    }

    result = alloc_chrdev_region(&dev, 0, 1, "omendrv");

    if (result < 0)
    {
        printk("[Omen laptop driver] Device registration failed.");
        return -1;
    }

    // if ((dev_class = class_create(THIS_MODULE, "omendrv")) == NULL) --> code before linux 6.4
    if ((dev_class = class_create("omendrv")) == NULL)
    {
        printk("[Omen laptop driver] Device class creation failed.");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    if (device_create(dev_class, NULL, dev, NULL, "omendrv%d", 0) == NULL)
    {
        printk("[Omen laptop driver] Device creation failed.");
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    cdev_init(&cdev, &fops);
    result = cdev_add(&cdev, dev, 1);

    if (result < 0)
    {
        printk("[Omen laptop driver] Device addition failed.");
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    printk("HP Omen four zone rgb laptop keyboard driver loaded!");
    return 0;
}

static void hp_omen_module_exit(void)
{
    // cleanup
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    unregister_chrdev_region(dev, 1);

    printk("HP Omen wmi driver unloaded!");
}

module_init(hp_omen_module_init);
module_exit(hp_omen_module_exit);
