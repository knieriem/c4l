extern	int	kapi_register_chrdev(int, char*);
extern	void	kapi_unregister_chrdev(int, char*);
extern	int	kapi_request_irq(int irq, char *name, void *dev);
extern	struct inode*	kapi_fileinode(struct file*);


enum {
	TMeasSend,
	TMeasDelay,
	NTMeas,
};
extern	void	kapi_t0(int, int cmpval);
extern	void	kapi_tprint(int, int period);


extern	void	kapi_schedule_delayed(void **v, void (*f)(void*), int, void*);
