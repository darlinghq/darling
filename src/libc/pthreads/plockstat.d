typedef struct _opaque_pthread_mutex_t pthread_mutex_t;
typedef struct _opaque_pthread_rwlock_t pthread_rwlock_t;

provider plockstat {
  probe mutex__acquire(pthread_mutex_t *mutex, int recursive, int spin_count);
  probe mutex__release(pthread_mutex_t *mutex, int recursive);
  probe mutex__error(pthread_mutex_t *mutex, int errno);
  probe mutex__block(pthread_mutex_t *mutex);
  probe mutex__blocked(pthread_mutex_t *mutex, int successful);
  probe mutex__spin(pthread_mutex_t *mutex);
  probe mutex__spun(pthread_mutex_t *mutex, int successful, int spin_count);

  probe rw__acquire(pthread_rwlock_t *rwlock, int write_lock);
  probe rw__block(pthread_rwlock_t *rwlock, int write_lock);
  probe rw__blocked(pthread_rwlock_t *rwlock, int write_lock, int successful);
  probe rw__release(pthread_rwlock_t *rwlock, int write_lock);
  probe rw__error(pthread_rwlock_t *rwlock, int write_lock, int error);
};

#pragma D attributes Evolving/Evolving/ISA provider plockstat provider
#pragma D attributes Private/Private/Unknown provider plockstat module
#pragma D attributes Private/Private/Unknown provider plockstat function
#pragma D attributes Evolving/Evolving/ISA provider plockstat name
#pragma D attributes Evolving/Evolving/ISA provider plockstat args
