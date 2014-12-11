CC=clang

dispatch_api:
	$(CC) -o dispatch_api dispatch_api.c -lpthread -lrt -ldispatch -lBlocksRuntime -lkqueue -lpthread_workqueue

check: dispatch_api
	./dispatch_api

clean:
	rm -f dispatch_api
