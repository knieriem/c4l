extern	int	kapi_register_chrdev(int, char*);
extern	void	kapi_unregister_chrdev(int, char*);
extern	int	kapi_request_irq(int irq, char *name, void *dev);
extern	struct inode*	kapi_fileinode(struct file*);
