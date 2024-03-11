/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */
//"C_Cpp.errorSquiggles": "disabled",
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h> 
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int BYE = 1;
int aesd_major = 0; // use dynamic major
int aesd_minor = 0;

MODULE_AUTHOR("Abdullah Alabd - alabd0 -"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device; // need protection.

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("Open");
    /**
     * TODO: handle open
     */

    /* now trim to 0 the length of the device if open was write-only */
    filp->private_data = &aesd_device;
    if (mutex_lock_interruptible(&aesd_device.lock))
        return -ERESTARTSYS;
    mutex_unlock(&aesd_device.lock);
    return 0; /* success */
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

/**
 * @brief reading bytes of the circular buffer every time read the next entry till all entries
 *        is done reading.
 */
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos)
{
    ssize_t retval, ready_read_bytes;

    PDEBUG("read %zu bytes with offset %lld \n", count, *f_pos);
    /**
     * TODO: handle read
     */
    struct aesd_dev *dev = filp->private_data;
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    size_t first_entry_pos = 0;
    struct aesd_buffer_entry *read_start_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->cbuffer, *f_pos, &first_entry_pos);
    if (!read_start_entry)
    {
        PDEBUG("NO MORE DATA!\n");
        retval = 0;
        *f_pos = 0;
        goto out;
    }
    ready_read_bytes = read_start_entry->size - first_entry_pos;
    if (copy_to_user(buf, read_start_entry->buffptr + first_entry_pos, ready_read_bytes))
    {
        PDEBUG("aesd_read->copy_to_user function error! ");
        retval = -EFAULT;
        goto out;
    }
    count -= ready_read_bytes;
    char *temp_ptr = NULL;
    temp_ptr = (char *)krealloc(temp_ptr, read_start_entry->size + 1, GFP_KERNEL);
    temp_ptr = memcpy(temp_ptr, read_start_entry->buffptr + first_entry_pos, read_start_entry->size);
    temp_ptr[read_start_entry->size] = '\0';
    PDEBUG("READING: %s!\n", temp_ptr);
    kfree(temp_ptr);

    PDEBUG("After READ >>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<>>>>>>><< !>>> !\n");

    *f_pos += ready_read_bytes;
    retval = ready_read_bytes;
out:
    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                   loff_t *f_pos)
{
    static char *temp_buffer = NULL;
    struct aesd_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld\n", count, *f_pos);
    /**
     * TODO: handle write
     */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (dev->n_entries == (AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED + 1))
    {
        dev->n_entries = 0;
        dev->full_buffer_size = 0;
        kfree(dev->full_buffer);
        dev->last_entry.buffptr = NULL;
        dev->last_entry.size = 0;
        dev->full_buffer = NULL;
        temp_buffer = NULL;
        aesd_circular_buffer_init(&dev->cbuffer);
    }
    dev->full_buffer_size += count;
    dev->full_buffer = (const char *)krealloc(dev->full_buffer, dev->full_buffer_size, GFP_KERNEL);
    if (!dev->full_buffer)
    {
        retval = -ENOMEM;
        goto out;
    }
    if (!temp_buffer)
    {
        temp_buffer = dev->full_buffer;
    }
    else if (temp_buffer != dev->full_buffer)
    {
        int l_n = 0;
        ssize_t l_s = 0;
        while (1)
        {
            if ((l_n != AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) && (dev->cbuffer.entry[l_n].size))
            {
                dev->cbuffer.entry[l_n].buffptr = dev->full_buffer + l_s;
            }
            else
            {
                break;
            }
            l_s += dev->cbuffer.entry[l_n].size;
            l_n++;
        }
        temp_buffer = dev->full_buffer;
    }

    if (dev->n_entries == 0) // no_entries_set_first_entry
    {
        dev->last_entry.buffptr = dev->full_buffer;
    }
    else if (dev->last_entry.buffptr == NULL)
    { // NEW_ENTRY_after\n;
        dev->last_entry.buffptr = dev->full_buffer + dev->full_buffer_size - count;
        dev->last_entry.size = 0;
    }

    copy_from_user(dev->last_entry.buffptr + dev->last_entry.size, buf, count);
    dev->last_entry.size += count;

    if (dev->last_entry.buffptr[dev->last_entry.size - 1] == '\n')
    {
        aesd_circular_buffer_add_entry(&dev->cbuffer, &dev->last_entry);
        dev->last_entry.buffptr = NULL;
        dev->last_entry.size = 0;
        dev->n_entries++;
    }
    retval = count;
out:

    if (retval > 0)
    {
        ssize_t n = 0;
        char *temp_ptr = NULL;
        while (n < dev->n_entries && n <= 9)
        {
            temp_ptr = (char *)krealloc(temp_ptr, dev->cbuffer.entry[n].size + 1, GFP_KERNEL);
            temp_ptr = memcpy(temp_ptr, dev->cbuffer.entry[n].buffptr, dev->cbuffer.entry[n].size);
            temp_ptr[dev->cbuffer.entry[n].size] = '\0';
            PDEBUG("buffer content: Entry[%d]:  %s - whith sizeof [%zu] BYTES \n", n, temp_ptr, dev->cbuffer.entry[n].size);
            n++;
        }
        kfree(temp_ptr);
    }
    mutex_unlock(&dev->lock);
    return retval;
}

struct file_operations aesd_fops = {
    .owner = THIS_MODULE,
    .read = aesd_read,
    .write = aesd_write,
    .open = aesd_open,
    .release = aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops; // NO NEED already initialized..
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
    {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}

/**
 * @brief allocating major device number for the device and set the @dev that contain that major and minor number.
 *
 * @return int
 */
int aesd_init_module(void)
{
    PDEBUG("Init Modules .......................................  %d \n", BYE);
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
                                 "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0)
    {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device, 0, sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    mutex_init(&aesd_device.lock);
    result = aesd_setup_cdev(&aesd_device);

    if (result)
    {
        unregister_chrdev_region(dev, 1);
    }
    return result;
}

void aesd_cleanup_module(void)
{
    PDEBUG("BYE BYE Modules .......................................%d \n", BYE);
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);
    if (aesd_device.full_buffer)
    {
        kfree(aesd_device.full_buffer);
    }
    kfree(&aesd_device); //*
    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
