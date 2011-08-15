#include <linux/module.h>

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
