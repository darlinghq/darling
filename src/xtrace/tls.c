#include <pthread/pthread.h>
#include <stdlib.h>
#include <darling/emulation/ext/for-xtrace.h>
#include "tls.h"
#include "malloc.h"
#include "lock.h"
#include <darling/emulation/simple.h>

#ifndef XTRACE_TLS_DEBUG
	#define XTRACE_TLS_DEBUG 0
#endif

#if XTRACE_TLS_DEBUG
	#define xtrace_tls_debug(x, ...) xtrace_printf(x "\n", ## __VA_ARGS__)
#else
	#define xtrace_tls_debug(x, ...)
#endif

// 100 TLS vars should be enough, right?
#define TLS_TABLE_MAX_SIZE 100

typedef struct tls_table* tls_table_t;
struct tls_table {
	size_t size;
	void* table[TLS_TABLE_MAX_SIZE][2];
};

static xtrace_once_t tls_key_initialized = XTRACE_ONCE_INITIALIZER;
static pthread_key_t tls_key;

static void tls_table_destructor(void* _table) {
	tls_table_t table = _table;
	xtrace_tls_debug("destroying table %p", table);
	for (size_t i = 0; i < table->size; ++i) {
		xtrace_tls_debug("freeing value %p for key %p", table->table[i][1], table->table[i][0]);
		xtrace_free(table->table[i][1]);
	}
	xtrace_tls_debug("freeing table %p", table);
	xtrace_free(table);
};

static void tls_key_initializer(void) {
	xtrace_tls_debug("initializing tls key");
	if (pthread_key_create(&tls_key, tls_table_destructor) != 0) {
		_abort_with_payload_for_xtrace(0, 0, NULL, 0, "xtrace: failed pthread key creation", 0);
	}
};

void* xtrace_tls(void* key, size_t size) {
	xtrace_tls_debug("looking up tls variable for key %p", key);

	xtrace_once(&tls_key_initialized, tls_key_initializer);
	tls_table_t table = pthread_getspecific(tls_key);

	xtrace_tls_debug("got %p as table pointer from pthread", table);

	// if the table doesn't exist yet, create it
	if (table == NULL) {
		xtrace_tls_debug("table is NULL, creating now...");
		table = xtrace_malloc(sizeof(struct tls_table));
		if (table == NULL) {
			_abort_with_payload_for_xtrace(0, 0, NULL, 0, "xtrace: failed TLS table memory allocation", 0);
		}
		table->size = 0;
		pthread_setspecific(tls_key, table);
	}

	// check if the key is already present
	for (size_t i = 0; i < table->size; ++i) {
		if (table->table[i][0] == key) {
			xtrace_tls_debug("found entry in table for key %p with value %p", key, table->table[i][1]);
			return table->table[i][1];
		}
	}

	// otherwise, create it
	xtrace_tls_debug("creating new entry in table for key %p", key);
	size_t index = table->size++;
	table->table[index][0] = key;
	table->table[index][1] = xtrace_malloc(size);
	if (table->table[index][1] == NULL) {
		_abort_with_payload_for_xtrace(0, 0, NULL, 0, "xtrace: failed TLS variable memory allocation", 0);
	}
	xtrace_tls_debug("new table entry created for key %p with value %p", key, table->table[index][1]);
	return table->table[index][1];
};
