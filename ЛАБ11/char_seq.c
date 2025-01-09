#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "char_seq"
#define MAX_SIZE 100

static int major;
static int size = 10; // Размер последовательности (по умолчанию 10)
static int current_number = 0;
module_param(size, int, 0644);
MODULE_PARM_DESC(size, "Размер последовательности чисел");

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    char data[20];
    int data_len;

    if (current_number >= size) {
        return 0; // Конец данных
    }

    data_len = snprintf(data, sizeof(data), "%d\n", current_number);
    if (data_len < 0) {
        return -EFAULT;
    }

    if (length < data_len) {
        return -EINVAL;
    }

    if (copy_to_user(buffer, data, data_len)) {
        return -EFAULT;
    }

    current_number++;
    return data_len;
}

static int device_open(struct inode *inode, struct file *file) {
    current_number = 0; // Сбрасываем счётчик при открытии устройства
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .open = device_open,
};

static int __init char_seq_init(void) {
    if (size > MAX_SIZE) {
        pr_warn("Размер превышает MAX_SIZE, устанавливается MAX_SIZE\n");
        size = MAX_SIZE;
    }

    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Не удалось зарегистрировать устройство\n");
        return major;
    }

    pr_info("Устройство %s зарегистрировано с major номером %d\n", DEVICE_NAME, major);
    return 0;
}

static void __exit char_seq_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Устройство %s удалено\n", DEVICE_NAME);
}

module_init(char_seq_init);
module_exit(char_seq_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Sergey Fibikh");
MODULE_DESCRIPTION("The driver that outputs the sequence of numbers");