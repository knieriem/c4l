typedef struct MsgQ MsgQ;
struct MsgQ {
	int produced, consumed, mask;
	canmsg_t *data;

	/* lock protects idle;
	 * lk == &lock if lock is used,
	 * otherwise lk == NULL
	 */
	spinlock_t lock, *lk;
	int idle;

};

extern	int	qcreate(MsgQ*, int sz, int protect);
extern	void	qfree(MsgQ*);

extern	int	qproduce(MsgQ*, void (*f)(canmsg_t*, void*), void*);
extern	int	qconsume(MsgQ*, int (*f)(canmsg_t*, void*), void*);
extern	int	qlen(MsgQ*);
extern	int	qsize(MsgQ*);

extern	MsgQ	txqueues[MAX_CHANNELS];
extern	MsgQ	rxqueues[MAX_CHANNELS];
