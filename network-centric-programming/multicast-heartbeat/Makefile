all: log_example heartbeat_app
log_example:
	gcc $(CFLAGS) log_example.c logger.c -o main

heartbeat_app: heartbeat_app.c logger.c
	gcc heartbeat_app.c logger.c -o heartbeat_app

clean:
	rm -f main
