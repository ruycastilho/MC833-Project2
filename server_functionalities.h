#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define USERS "users.dat"
#define COURSES "courses.dat"
#define MAXDATASIZE 550
#define NAME_LENGTH 6
#define PWD_LENGTH 2
#define COMMENT_LENGTH 50
#define HEADER_LENGTH 3
#define LOST_DATA_INTERVAL 100 // in ms
#define DATAGRAM_SIZE 64000
#define TV_SECONDS 1
#define TV_USECONDS 1000



typedef struct {

    char name[NAME_LENGTH];
    char pwd[PWD_LENGTH];
    int is_prof;

} user;

typedef struct {

    char code[6];
    char name[100];
    char institute[6];
    char room[6];
    char schedule[50];
    char description[100];
    char professor[NAME_LENGTH];
    char comment[COMMENT_LENGTH];

} course;

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int repeat_send(int fd, const void *buffer, int size);
int send_func_login(int fd);
int ementa(int fd);
int infos(int fd);
int todas_infos(int fd);
int cod_titulo(int fd);
int escrever_com(user* prof, int fd);
int ler_com(int fd);
int send_menu(user* user_info, int fd);
user* validate_login(int fd);
void send_func(int fd);
course* code_search(int fd);
