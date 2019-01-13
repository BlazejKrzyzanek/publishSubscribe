#include <sys/types.h> // ipc
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <stdio.h> // input output
#include <string.h> // text
#include <stdbool.h> // true and false
#include <stdlib.h>
#include <unistd.h>

#define NAME_LEN 100
#define MSG_LEN 1024
#define LEN(x) sizeof(x) / sizeof(x[0])

typedef struct user{
    pid_t pid;
    char name[NAME_LEN];
} user;

typedef struct login_msg{
    long type;
    pid_t pid;
    char name[NAME_LEN];
}login_msg;

typedef struct server_response{
    long type;
    char text[MSG_LEN];
} sr_msg;

/** Sprawdza czy tablica zawiera juz tego uzytkownika **/
bool contains_user(user *users, user u, size_t length){        
    for (unsigned int i=0; i<length ; i++){
        if (users[i].pid == u.pid) return true;
    }
    return false;
}

/**>>>>>>>>>>>>>>>>>>>>>>> Obsaluga logowan <<<<<<<<<<<<<<<<<<<<<<<<**/


void login_handling(int id, unsigned int i, user *users, size_t length){
    login_msg m;
    if(msgrcv(id, &m, sizeof(m) - sizeof(long), 1, IPC_NOWAIT)>0){
        user u;
        u.pid = m.pid; 
        strcpy(u.name, m.name);
        if(!contains_user(users, u, length) && i < length){
            users[i] = u; 
            sr_msg response;
            response.type = 2;
            strcpy(response.text, "Pomyslnie dodano uzytkownika!");
            msgsnd(id, &response, sizeof(response) - sizeof(long), 0);
            printf("Dodano uzytkownika %s, o pid %d\n", users[i].name, users[i].pid);
            i++;
        }else{
            sr_msg response;
            response.type = 2;
            strcpy(response.text,"Odrzucono ponowna rejestracje uzytkownika!");
            msgsnd(id, &response, sizeof(response) - sizeof(long), 0);
            printf("Odrzucono ponowna rejestracje uzytkownika");
        }
    }    
}


/**>>>>>>>>>>>>>>>>>>>>>>> Glowna petla programu <<<<<<<<<<<<<<<<<<<**/


int main(int argc, char* argv[]){

    user users[3];
    int id = msgget(1000, 0766|IPC_CREAT);
    unsigned int i = 0;
    while(1){
        login_handling(id, i, users, LEN(users));
        sleep(1);
    }
    msgctl(id, IPC_RMID, 0);
    exit(0);
    //wait(NULL);


    return 0;
}

