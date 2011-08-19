#include "defs.h"
#include <linux/module.h>
#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif


#include "kapi.h"

extern	int	core_init(void);
extern	void	core_cleanup(void);

int	init_module(void)
{
	return core_init();
}

void	cleanup_module(void)
{
	core_cleanup();
}

int	inuse(void)
{
	return MOD_IN_USE;
}

void	incusers(void)
{
	MOD_INC_USE_COUNT;
}

void	decusers(void)
{
	MOD_DEC_USE_COUNT;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("H.-J.Oertel <oe@port.de>");
MODULE_DESCRIPTION("CAN fieldbus driver");



static struct file_operations fops = { 
    llseek:	NULL, 
    read:	can_read,
    write:	can_write,
    readdir:	NULL, 
    poll:	can_select,
    ioctl:	can_ioctl,
    mmap:	NULL, 
    open:	can_open,
    flush:	NULL, /* flush call */
    release:	can_close,
    fsync:	NULL,
    fasync:	NULL,
};

#define MAX_CHANNELS 4


#ifdef CONFIG_DEVFS_FS
devfs_handle_t can_dev_handle[MAX_CHANNELS];
static char devname[MAX_CHANNELS];
#endif


int	kapi_register_chrdev(int major, char *name)
{
#ifdef CONFIG_DEVFS_FS
	int i;

	/* If we have devfs, create /dev/canX to put files in there */
	for (i=0; i < MAX_CHANNELS; i++) {
		sprintf(devname, "can%i", i);
		can_dev_handle[i]=devfs_register(NULL, devname, 0,
			major, i, S_IFCHR | S_IRUGO | S_IWUGO,
			&fops, NULL);
	}
	devfs_register_chrdev(major, name, &fops);
	return 0;
#else /* no devfs, do it the "classic" way  */
	return register_chrdev(major, name, &fops);
#endif
}

void	kapi_unregister_chrdev(int major, char *name)
{
#ifndef CONFIG_DEVFS_FS
	if(unregister_chrdev(major, name) != 0)
		printk("can't unregister Can, device busy \n");
	else
		printk("Can successfully removed\n");
#else
	int i;

	for (i = 0; i < MAX_CHANNELS; i++) {
		devfs_unregister(can_dev_handle[i]);
	}
	devfs_unregister_chrdev(major, name);
#endif
}
