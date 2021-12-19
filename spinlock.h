typedef struct {
   volatile unsigned int lock;
} spinlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
void spin_lock_init(spinlock_t *lock);
