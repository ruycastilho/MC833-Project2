#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server_functionalities.h"

#define NUM_USERS 10
#define NUM_COURSES 5

// Creates new user and returns pointer
user* create_user(char name[NAME_LENGTH], char pwd[PWD_LENGTH], int is_prof) {

    user *new_user = (user*)malloc(sizeof(user));
    strcpy(new_user->name, name);
    strcpy(new_user->pwd, pwd);
    new_user->is_prof = is_prof;

    return new_user;
}

// Creates new course and returns pointer
course* create_course(char code[6], char name[100], char institute[6], char room[6],
                    char schedule[50], char description[100], char professor[NAME_LENGTH], char comment[50]) {

    course *new_course = (course *)malloc(sizeof(course));
    strcpy(new_course->code, code);
    strcpy(new_course->name, name);
    strcpy(new_course->institute, institute);
    strcpy(new_course->room, room);
    strcpy(new_course->schedule, schedule);
    strcpy(new_course->description, description);
    strcpy(new_course->professor, professor);
    strcpy(new_course->comment, comment);

    return new_course;
}


int main() {

    FILE *users_f, * courses_f;
    
    // Setting up
    users_f = fopen(USERS, "wb+");
    courses_f = fopen(COURSES, "wb+");
    user** users = (user**)malloc(sizeof(user*)*NUM_USERS);
    course** courses = (course**)malloc(sizeof(course*)*NUM_COURSES);

    users[0] = create_user("user1", "1", 0);
    users[1] = create_user("user2", "2", 0);
    users[2] = create_user("user3", "3", 0);
    users[3] = create_user("user4", "4", 0);
    users[4] = create_user("user5", "5", 0);
    users[5] = create_user("user6", "6", 0);
    users[6] = create_user("user7", "7", 0);
    users[7] = create_user("user8", "8", 0);
    users[8] = create_user("prof1", "1", 1);
    users[9] = create_user("prof2", "2", 1);

    courses[0] = create_course("MC102", "Algoritmos e Programação de Computadores", "IC", "CB10", "SEGUNDA 14:00 QUARTA 14:00", "Introducao a computacao", "prof1", "Boas vindas");
    courses[1] = create_course("BE310", "Ciencias do Ambiente", "IB", "IB02", "QUARTA 08:00", "Ecologia", "prof2", "Ola!");
    courses[2] = create_course("MC202", "Estruturas de Dados", "IC", "CB04", "TERCA 10:00 QUINTA 10:00", "Introducao a estruturas de dados", "prof1", "Arvores!");
    courses[3] = create_course("MC302", "Orientacao a Objetos", "IC", "CB06", "SEGUNDA 14:00 QUARTA 14:00", "Introducao a POO", "prof1", "Java!");
    courses[4] = create_course("MC558", "Analise e Projeto de Algoritmos II", "IC", "CB07", "SEGUNDA 19:00 QUARTA 21:00", "Analise e projeto de algoritmos em grafos", "prof1", "Grafos!");

    // Mem dealloc
    for(int i=0; i < NUM_USERS; i++) {
        fwrite(users[i], sizeof(user), 1, users_f);
        free(users[i]);

    }
    // fseek(users_f, 0, SEEK_SET);
    // user aux;
    // fread(&aux, sizeof(user), 1, users_f);
    // printf("%s %s\n", aux.name, aux.pwd);
    free(users);
    fclose(users_f);

    for(int i=0; i < NUM_COURSES; i++) {
        fwrite(courses[i], sizeof(course), 1, courses_f);
        free(courses[i]);
    }
    free(courses);
    fclose(courses_f);

}