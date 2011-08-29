#include "defs.h"
#include <linux/module.h>

#include "kapi.h"

#define nelem(x) (sizeof(x)/sizeof((x)[0]))

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
	return can_ioctl(file->f_path.dentry->d_inode, file, cmd, arg);
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
    owner: THIS_MODULE,
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

struct inode* kapi_fileinode(struct file *file)
{
	return file->f_dentry->d_inode;
}


/*
 * measure Δt, i.e. time between `t0' and `tprint'
 */
static int	tcmpval[NTMeas];
static ktime_t t0[NTMeas];
static int	it[NTMeas];

void kapi_t0(int name, int cmpval)
{
	tcmpval[name] = cmpval;
	t0[name] = ktime_get();
}

void kapi_tprint(int name, int period)
{
	struct timeval d;

	if (it[name] == period) {
		it[name] = 0;
		d = ktime_to_timeval(ktime_sub(ktime_get(), t0[name]));
		printk("tmeas %d: Δt=%ld.%03ldms\t(cmp: %d)\n", name, d.tv_sec*1000+d.tv_usec/1000, d.tv_usec%1000, tcmpval[name]);
	}
	it[name]++;
}


/*
 * utility for scheduling delayed work, using high resolution timers
 */
typedef struct Delayed Delayed;
struct Delayed {
	struct hrtimer timer;
	void (*f)(void*);
	void *data;
};
static Delayed dly[MAX_CHANNELS];
static int idly;

static enum hrtimer_restart dlyworker(struct hrtimer *t)
{
	Delayed *d = container_of(t, Delayed, timer);

//	kapi_tprint(TMeasDelay, 70);
	d->f(d->data);
	return HRTIMER_NORESTART;
}

void kapi_schedule_delayed(void **v, void (*f)(void*), int usec, void *data)
{
	Delayed *d;

//	kapi_t0(TMeasDelay, usec);
	if (*v == NULL) {
		/* initialize */
		if (idly==nelem(dly)) {
			printk("can: kapi: out of timers (static Delayed dly[%d])\n", idly);
			return;
		}
		*v = &dly[idly];
		d = *v;
		idly++;
		hrtimer_init(&d->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		d->timer.function = dlyworker;
	} else
		d = *v;

	d->f = f;
	d->data = data;
	hrtimer_start(&d->timer, ktime_set(0, usec*1000), HRTIMER_MODE_REL);
}
