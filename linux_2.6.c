#include "defs.h"
#include <linux/module.h>


#include "kapi.h"

extern	int	core_init(void);
extern	void	core_cleanup(void);

static int	init(void)
{
	return core_init();
}

static void	cleanup(void)
{
	core_cleanup();
}

int	inuse(void)
{
	return 0;
}

void	incusers(void)
{
}

void	decusers(void)
{
}

module_init(init);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("H.-J.Oertel <oe@port.de>");
MODULE_DESCRIPTION("CAN fieldbus driver");


static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return can_ioctl(file->f_dentry->d_inode, file, cmd, arg);
}


static struct file_operations fops = { 
    llseek:	NULL, 
    read:	can_read,
    write:	can_write,
    readdir:	NULL, 
    poll:	can_select,
    unlocked_ioctl:	ioctl,
    mmap:	NULL, 
    open:	can_open,
    flush:	NULL, /* flush call */
    release:	can_close,
    fsync:	NULL,
    fasync:	NULL,
};

int	kapi_register_chrdev(int major, char *name)
{
	return register_chrdev(major, name, &fops);
}

void	kapi_unregister_chrdev(int major, char *name)
{
	unregister_chrdev(major, name);
}

static irqreturn_t	isr(int irq, void *dev_id) {
	if(CAN_Interrupt(irq, dev_id) == 1)
		return IRQ_HANDLED;
	return IRQ_NONE;
}

int kapi_request_irq(int irq, char *name, void *dev)
{
	return request_threaded_irq(irq, NULL, isr, 0, "Can", dev);
}
