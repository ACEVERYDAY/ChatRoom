CC = gcc
SERVER_DIR = Server/
CLIENT_DIR = Client/
DAEMON_DIR = Daemon/
SERVER_PATH = $(SERVER_DIR)server.o $(SERVER_DIR)varifyLogin.o $(SERVER_DIR)getChatRecord.o $(SERVER_DIR)viewSystemLog.o
CLIENT_PATH = $(CLIENT_DIR)client.o
DAEMON_PATH = $(DAEMON_DIR)daemon.o

server := $(SERVER_DIR)server
client := $(CLIENT_DIR)client
daemon := $(DAEMON_DIR)daemon

server : $(SERVER_PATH)
	$(CC) -o $(SERVER_DIR)server $(SERVER_PATH)
%.o : %.c
	$(CC) -c $^ -o $@

client : $(CLIENT_PATH)
	$(CC) -o $(CLIENT_DIR)client $(CLIENT_PATH)
%.o : %.c
	$(CC) -c $^ -o $@

daemon : $(DAEMON_PATH)
	$(CC) -o $(DAEMON_DIR)daemon $(DAEMON_PATH)

clean:
	-rm -rf $(SERVER_PATH) $(CLIENT_PATH) $(DAEMON_PATH)
