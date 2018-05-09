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
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include "server_functionalities.h"

struct timeval tv1, tv2;
struct sockaddr_storage their_addr; // connector's address information
struct addrinfo *p;

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);
        errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int repeat_receive(int sockfd, void * recv_buffer, int recv_buffer_size) {
    int addr_len = sizeof their_addr;
    int numbytes;
    char datagram[DATAGRAM_SIZE];

    if ((numbytes = recvfrom(sockfd, &datagram, DATAGRAM_SIZE , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("datagram'%s'\n\n", datagram);

    memcpy(recv_buffer, datagram, recv_buffer_size);

    return numbytes;


}

int repeat_send(int fd, const void *buffer, int size) {
    char datagram[DATAGRAM_SIZE];
    int numbytes;
    // printf("recebido'%s'\n\n", ((char*)buffer));

    memcpy(datagram, buffer, size);
    // printf("datagram'%s'\n\n", datagram);

    if ((numbytes = sendto(fd, &datagram, sizeof(datagram), 0,
             (struct sockaddr *)&their_addr,sizeof(their_addr))) == -1) {
        perror("talker: sendto");
        exit(1);
    }


    return 1;
}

course* code_search(int fd) {
    char string[] = "\nDigite o código da disciplina:\n";
    char buffer[MAXDATASIZE];
    int numbytes;

    if (repeat_send(fd, string, sizeof(string)) == -1) {
        perror("send");
        return NULL;

    }

    // receive the code number
    numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);
    if ( numbytes == -1) {
        perror("recv");
        return NULL;
    }
    // buffer[numbytes] = '\0';

    // start the timer
    gettimeofday(&tv1, NULL);


    FILE* courses_f ;
    int status = 0;

    if (courses_f = fopen(COURSES, "rb")) {
        course* existing_course = (course*)malloc(sizeof(course));

        while ( fread(existing_course, sizeof(course), 1, courses_f) ) {

            if (strcmp(existing_course->code, buffer) == 0 ) {
                status = 1;
                if (repeat_send(fd, &status, sizeof(int)) == -1) {
                    perror("send");
                    return NULL;
                }

                fclose(courses_f);
                return existing_course;
            }
        }

        if (repeat_send(fd, &status, sizeof(int)) == -1) {
            perror("send");
            return NULL;
        }

        free(existing_course);
        fclose(courses_f);

        char notfound_error[] = "\nDisciplina não encontrada.\n";

        if (repeat_send(fd, notfound_error, sizeof(notfound_error)) == -1) {
            perror("send");
            fclose(courses_f);
            return NULL;
        }
    }

    if (repeat_send(fd, &status, sizeof(int)) == -1) {
        perror("send");
        return NULL;
    }

    char file_error[] = "\nErro ao abrir arquivo.\n";

    if (repeat_send(fd, file_error, sizeof(file_error)) == -1) {
        perror("send");
        return NULL;
    }

    return NULL;

}

int ementa(int fd) {
    course* course_info = code_search(fd);
    int numbytes;

    if (course_info != NULL) {

        gettimeofday(&tv2, NULL);
        printf("Tempo total da operação: %.2f usecs\n", (float)(tv2.tv_usec - tv1.tv_usec));


        if (repeat_send(fd, course_info, sizeof(course)) == -1) {
            perror("send");
            free(course_info);
            return -1;
        }

        return 1;
    }

    free(course_info);

    return -1;

}

int infos(int fd) {

    course* course_info = code_search(fd);
    int numbytes;

    if (course_info != NULL) {

        gettimeofday(&tv2, NULL);
        printf("Tempo total da operação: %.2f usecs\n", (float)(tv2.tv_usec - tv1.tv_usec));

        if (repeat_send(fd, course_info, sizeof(course)) == -1) {
            perror("send");
            free(course_info);
            return -1;
        }
        return 1;
    }

    free(course_info);

    return -1;

}

int todas_infos(int fd) {
    FILE* courses_f ;
    course* existing_course = (course*)malloc(sizeof(course));
    int numbytes, status = 1;

    if (courses_f = fopen(COURSES, "rb")) {

        if (repeat_send(fd, &status, sizeof(status)) == -1) {
            perror("send");
            free(existing_course);
            fclose(courses_f);
            return -1;
        }

        while ( fread(existing_course, sizeof(course), 1, courses_f) ) {
  
            if (repeat_send(fd, &status, sizeof(status)) == -1) {
                perror("send");
                free(existing_course);
                fclose(courses_f);
                return -1;
            }

            if (repeat_send(fd, existing_course, sizeof(course)) == -1) {
                perror("send");
                free(existing_course);
                fclose(courses_f);
                return -1;
            }

        }

        free(existing_course);
        fclose(courses_f);
        status = 0;
        if (repeat_send(fd, &status, sizeof(status)) == -1) {
            perror("send");
            return -1;
        }

        return 1;

    }

    free(existing_course);
    status = 0;

    if (repeat_send(fd, &status, sizeof(status)) == -1) {
        perror("send");
        return -1;
    }

    char file_error[] = "\nErro ao abrir arquivo.\n";

    if (repeat_send(fd, file_error, sizeof(file_error)) == -1) {
        perror("send");
        return -1;
    }

    return -1;

}

int escrever_com(user* prof, int fd) {
    char string[] = "\nDigite o código da disciplina:\n";
    char buffer[MAXDATASIZE];
    int numbytes, found_course = 0, counter=0;

    if (repeat_send(fd, string, sizeof(string)) == -1) {
        perror("send");
        return -1;

    }

    numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);

    if ( numbytes == -1) {
        perror("recv");
        return -1;
    }
    // buffer[numbytes] = '\0';

    // start the timer
    gettimeofday(&tv1, NULL);

    FILE* courses_f ;
    course* existing_course = (course*)malloc(sizeof(course));
    int status = 0;

    if (courses_f = fopen(COURSES, "rb+")) {

        while ( fread(existing_course, sizeof(course), 1, courses_f) ) {

            if (strcmp(existing_course->code, buffer) == 0 ) {

                found_course = 1;
                break;

            }

            counter++;

        }

        // stop the timer and print elapsed time
        gettimeofday(&tv2, NULL);
        printf("tempo total da operação: %.2f usecs\n", (float)(tv2.tv_usec - tv1.tv_usec));


        if (!found_course) {
            if (repeat_send(fd, &status, sizeof(int)) == -1) {
                perror("send");
                return -1;
            }

            free(existing_course);
            fclose(courses_f);

            char notfound_error[] = "\nDisciplina não encontrada.\n";

            if (repeat_send(fd, notfound_error, sizeof(notfound_error)) == -1) {
                perror("send");
                return -1;
            }   

            return 1;
        }
        else if (strcmp(existing_course->professor, prof->name) != 0) {
            if (repeat_send(fd, &status, sizeof(int)) == -1) {
                perror("send");
                return -1;
            }

            char login_error[] = "\nVoce nao tem permissao para alterar comentarios dessa disciplina.\n";
            free(existing_course);
            fclose(courses_f);

            if (repeat_send(fd, login_error, sizeof(login_error)) == -1) {
                perror("send");
                return -1;
            }

            return 1;

        }

    }
    else {
        if (repeat_send(fd, &status, sizeof(int)) == -1) {
            perror("send");
            return -1;
        }
        char file_error[] = "\nErro ao abrir arquivo.\n";

        if (repeat_send(fd, file_error, sizeof(file_error)) == -1) {
            perror("send");
            return -1;
        }

    }
    status = 1;
    if (repeat_send(fd, &status, sizeof(int)) == -1) {
        perror("send");
        return -1;
    }

    char new_comment[COMMENT_LENGTH];
    fseek(courses_f, counter*sizeof(course), SEEK_SET);

    char comment_question[] = "\nDigite o comentario:\n";

    if (repeat_send(fd, comment_question, sizeof(comment_question)) == -1) {
        perror("send");
        return -1;
    }

    numbytes = repeat_receive(fd, new_comment, MAXDATASIZE-1);
    if (numbytes == -1) {
        perror("recv");
        return -1;
    }
    // buffer[numbytes] = '\0';

    strcpy(existing_course->comment, new_comment);
    fwrite(existing_course, sizeof(course), 1, courses_f);
    fseek(courses_f, counter*sizeof(course), SEEK_SET);
    fclose(courses_f);

    if ( fwrite != 0 ) {
        char feedback[] = "\nComentario adicionado.\n";
        free(existing_course);

        if (repeat_send(fd, feedback, sizeof(feedback)) == -1) {
            perror("send");
            return -1;
        }

        return 1;
    }
    else {
        char feedback[] = "\nErro ao escrever no arquivo.\n";
        free(existing_course);
        
        if (repeat_send(fd, feedback, sizeof(feedback)) == -1) {
            perror("send");
            return -1;
        }

        return -1;
    }
    
}

int ler_com(int fd) {

    course* course_info = code_search(fd);

    if (course_info != NULL) {

        // stop the timer and print elapsed time
        gettimeofday(&tv2, NULL);
        printf("Tempo total da operação: %.2f usecs\n", (float)(tv2.tv_usec - tv1.tv_usec));

        if (repeat_send(fd, course_info, sizeof(course)) == -1) {
            perror("send");
            free(course_info);
            return -1;
        }
        return 1;
    }

    free(course_info);

    return -1;

}

int send_func_login(int fd) {

    char string[] = "\nBoas vindas ao Sistema de Disciplinas da UNICAMP\nSe deseja logar digite 1. Se deseja sair, digite 2.\n";
    char buffer[MAXDATASIZE];
    int numbytes;

    // send options menu
    if (repeat_send(fd, string, sizeof(string)) == -1) {
        perror("send");
        return -1;
    }

    // receive the selected option

    numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);
    if ( numbytes == -1) {
        perror("recv");
        return -1;
    }
    // buffer[numbytes] = '\0';

    char erro[] = "\nPor favor, digite 1 ou 2.\n";

    while (strcmp(buffer, "1") != 0 && strcmp(buffer, "2") != 0) {
        if (repeat_send(fd, erro, sizeof(erro)) == -1) {
            perror("send");
            return -1;
        }

        numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);
        if ( numbytes== -1) {
            perror("recv");
            return -1;
        }
        // buffer[numbytes] = '\0';

    }

    if ( strcmp(buffer, "1") == 0) {
    
        char feedback[] = "\n\t\tLogin";

        if (repeat_send(fd, feedback, sizeof(feedback)) == -1) {
            perror("send");
            return -1;
        }
        return 1;

    }

    return -1;

}

int send_menu(user* user_info, int fd) {

    char string_prof[] = "\n\t\tMenu Principal\nDigite o número da funcionalidade que deseja:\n1) Receber ementa de uma disciplina a partir do seu código\n2) Receber todas as informações de uma disciplina a partir do seu código\n3) Listar todas as informações de todas as disciplinas\n4) Listar todos os códigos de disciplinas com seus respectivos títulos\n5) Receber o comentário da próxima aula de uma disciplina a partir de seu código\n6) Escrever comentário sobre próxima aula de uma de suas disciplinas\n7) Fechar conexão\n";
    char string_stud[] = "\n\t\tMenu Principal\nDigite o número da funcionalidade que deseja:\n1) Receber ementa de uma disciplina a partir do seu código\n2) Receber todas as informações de uma disciplina a partir do seu código\n3) Listar todas as informações de todas as disciplinas\n4) Listar todos os códigos de disciplinas com seus respectivos títulos\n5) Receber o comentário da próxima aula de uma disciplina a partir de seu código\n6) Fechar conexão\n";  
    
    char buffer[MAXDATASIZE];
    int numbytes;

    if (user_info->is_prof) {
        if (repeat_send(fd, string_prof, sizeof(string_prof)) == -1) {
            perror("send");
            return -1;

        }
    }
    else {
        if (repeat_send(fd, string_stud, sizeof(string_stud)) == -1) {
            perror("send");
            return -1;

        }

    }

    numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);
    if ( numbytes == -1) {
        perror("recv");
        return -1;
    }
    // buffer[numbytes] = '\0';

    char error_prof[] = "\nPor favor, digite números de 1 a 7.\n";
    char error_stud[] = "\nPor favor, digite números de 1 a 6.\n";

    if (user_info->is_prof) {

        while (strcmp(buffer, "1") != 0 && strcmp(buffer, "2") != 0 && strcmp(buffer, "3") != 0 && strcmp(buffer, "4") != 0 && strcmp(buffer, "5") != 0 && strcmp(buffer, "6") != 0 && strcmp(buffer, "7") != 0) {
            if (send(fd, error_prof, sizeof(error_prof), 0) == -1) {
                perror("send");
                return -1;

            }

            numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);
            if ( numbytes== -1) {
                perror("recv");
                return -1;
            }
            // buffer[numbytes] = '\0';

        }
    }
    else {

        while (strcmp(buffer, "1") != 0 && strcmp(buffer, "2") != 0 && strcmp(buffer, "3") != 0 && strcmp(buffer, "4") != 0 && strcmp(buffer, "5") != 0 && strcmp(buffer, "6") != 0) {
            if (repeat_send(fd, error_stud, sizeof(error_stud)) == -1) {
                perror("send");
                return -1;

            }

            numbytes = repeat_receive(fd, buffer, MAXDATASIZE-1);
            if ( numbytes == -1) {
                perror("recv");
                return -1;
            }
            // buffer[numbytes] = '\0';

        }

    }


    // start timer
    gettimeofday(&tv1, NULL);

    switch (buffer[0] - '0') {

        case 1:
            ementa(fd);
            break;
        case 2:
            infos(fd);
            break;
        case 3:
            todas_infos(fd);
            break;
        case 4:
            todas_infos(fd);
            break;
        case 5:
            ler_com(fd);
            break;
        case 6:
            if (user_info->is_prof) {
                escrever_com(user_info, fd);

            }
            else {
                return -1;
            }
            break;
        case 7:
            return -1;

    }

    return 1;
}

user* validate_login(int fd) {

    char string[] = "\nDigite seu nome e senha, por favor.\n";
    int numbytes;
    user* user_logging = (user*)malloc(sizeof(user)), *existing_user = (user*)malloc(sizeof(user));

    // send 'digite nome e senha em linhas separadas'
    if (repeat_send(fd, string, sizeof(string)) == -1) {
        perror("send");
        return NULL;
    }

    // receive username and password
    numbytes = repeat_receive(fd, user_logging, sizeof(user));

    if ( numbytes == -1 ) {
        perror("recv");
        return NULL;
    }
    printf("server: user:'%s' pwd:'%s'\n", user_logging->name, user_logging->pwd);


    FILE* users_f ;
    int status = 0;

    if (users_f = fopen(USERS, "rb")) {
   
        fseek(users_f, 0, SEEK_SET);
        while ( fread(existing_user, sizeof(user), 1, users_f) ) {

            if (strcmp(existing_user->name, user_logging->name) == 0 && strcmp(existing_user->pwd, user_logging->pwd) == 0 ) {
                free(user_logging);
                status = 1;

                // send the status
                if (repeat_send(fd, &status, sizeof(int)) == -1) {
                    perror("send");
                    return NULL;
                }

                // send the user struct
                if (repeat_send(fd, existing_user, sizeof(user)) == -1) {
                    perror("send");
                    return NULL;
                }

                return existing_user;
            }
        }


        char string_erro1[] = "\nUsuario nao encontrado.\n";

        status = 0;
        // send the status
        if (repeat_send(fd, &status, sizeof(int)) == -1) {
            perror("send");
            return NULL;
        }

        if (repeat_send(fd, string_erro1, sizeof(string_erro1)) == -1) {
            perror("send");
            return NULL;
        }

        return NULL;

    }

    free(existing_user);
    free(user_logging);
    fclose(users_f);

    status = 0;
    // send the status
    if (repeat_send(fd, &status, sizeof(int)) == -1) {
        perror("send");
        return NULL;
    }

    char string_erro2[] = "\nErro na validacao.\n";

    if (repeat_send(fd, string_erro2, sizeof(string_erro2)) == -1) {
        perror("send");
        return NULL;
    }

    return NULL;

}

void get_send_addr(struct sockaddr_storage *addr, struct addrinfo* p) {

    switch (addr->ss_family)
    {
        case AF_INET:
            p->ai_family = AF_INET;
            p->ai_addr = (struct sockaddr*) &(((struct sockaddr_in*)addr)->sin_addr);
            break;
        case AF_INET6:
            p->ai_family = AF_INET6;        
            p->ai_addr = (struct sockaddr*) &(((struct sockaddr_in6*)addr)->sin6_addr);

            break;
    }

}

void send_func(int fd) {

    char buffer[MAXDATASIZE];

    printf("antes do initial recv\n");
    int numbytes = repeat_receive(fd, buffer, sizeof(buffer));

    printf("depois do initial recv. Received: '%s'\n", buffer);

    p = (struct addrinfo*)malloc(sizeof(struct addrinfo));

    get_send_addr(&their_addr, p);
    p->ai_addrlen = sizeof(p->ai_addr);
    p->ai_socktype = SOCK_DGRAM;
    p->ai_protocol = 0;

    user *user_info;
    int login;

    do {
        login = send_func_login(fd);

        if (login == 1) {
    
            // Returns user info or null pointer
            user_info = validate_login(fd);

            if (user_info != NULL) {
                do {
                    login = send_menu(user_info, fd);
                } while (login == 1);
            }
        }

        // If login fails, returns to initial menu

    } while (login == 1);

    free(p);
    free(user_info);
}
