#ifndef LINUX_POSIX_SPAWN_H
#define LINUX_POSIX_SPAWN_H
typedef enum {
        PSFA_OPEN = 0,
        PSFA_CLOSE = 1,
        PSFA_DUP2 = 2,
        PSFA_INHERIT = 3
} psfa_t;

struct _posix_spawnattr
{
	short psa_flags;
	unsigned int psa_sigdefault;
	unsigned int psa_sigmask;
	unsigned int psa_pgroup;
	int psa_binprefs[4];
	int psa_pcontrol;
	int psa_apptype;
	/* more fields that we don't support anyway */
};

struct _psfa_action
{
	psfa_t psfaa_type;
	int psfaa_filedes;
	struct _psfaa_open
	{
		int psfao_oflag;
		unsigned short psfao_mode;
		char psfao_path[1024];
	} psfaa_openargs;
};

struct _posix_spawn_file_actions
{
	int psfa_act_alloc;
	int psfa_act_count;
	struct _psfa_action psfa_act_acts[];
};

struct _posix_spawn_args_desc
{
	unsigned long attr_size;
	const struct _posix_spawnattr* attrp;
	unsigned long fact_size;
	const struct _posix_spawn_file_actions* factp;
	unsigned long port_actions_size;
	void* pactp;
};

long sys_posix_spawn(int* pid, const char* path, const struct _posix_spawn_args_desc* desc,
		char** argvp, char** envp);

#endif

