#define TICKET_BIT_SHIFT	16

typedef struct {
	unsigned short owner;
	unsigned short next;
} __attribute__((aligned (4))) spinlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
void spin_lock_init(spinlock_t *lock);
