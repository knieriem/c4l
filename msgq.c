#include "defs.h"

/* lock-free queues from `Real Time in Plan 9'
 * http://plan9.bell-labs.com/iwp9/Real-time.pdf
 *
 * one producer, one consumer only!
 */

#define lock_irqsave(lk, flags) if(lk != NULL){ spin_lock_irqsave(lk, flags); }
#define unlock_irqrestore(lk, flags) if(lk != NULL){ spin_unlock_irqrestore(lk, flags); }

int qproduce(MsgQ *q, void (*produce)(canmsg_t*, void*), void *data)
{
	unsigned long flags;

	if (q->produced - q->consumed > q->mask)
		return 0;
	produce(&q->data[q->produced & q->mask], data);

	lock_irqsave(q->lk, flags);
	q->produced++;
	unlock_irqrestore(q->lk, flags);
	return 1;
}

int qconsume(MsgQ *q, int (*consume)(canmsg_t*, void*), void *data)
{
	unsigned long flags;

	/* the lock make sure that q->idle is set properly
	 * before a new message is produced and q->idle is tested
	 */
	lock_irqsave(q->lk, flags);
	if (q->produced == q->consumed){
		q->idle = 1;
		unlock_irqrestore(q->lk, flags);
		return 0;
	}
	unlock_irqrestore(q->lk, flags);

	if (consume(&q->data[q->consumed & q->mask], data) != 0)
		q->consumed++;
	return 1;
}

int qlen(MsgQ *q)
{
	return q->produced - q->consumed;
}

int qsize(MsgQ *q)
{
	return q->mask+1;
}

int qcreate(MsgQ *q, int sz, int protect)
{
	q->data = kmalloc(sz * sizeof(canmsg_t), GFP_KERNEL);
	if (q->data == NULL)
		return -1;
	q->mask = sz-1;

	q->idle = 1;
	if (protect) {
		q->lk = &q->lock;
		spin_lock_init(q->lk);
	}
	return 0;
}

void qfree(MsgQ *q)
{
	kfree(q->data);
}
