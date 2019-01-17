#include <sys/types.h> // ipc
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <stdio.h> // input output
#include <string.h> // text
#include <stdbool.h> // true and false
#include <stdlib.h>
#include <unistd.h>
#include <time.h>   // czas
#include <signal.h> // sygnaly

#define PASSWD_LEN 100
#define MSG_LEN 1024
#define MAX_TYPES 10
#define MAX_USERS 10
#define MAX_SUBS 10
#define MAX_BAD_LOGIN 3
#define LEN(x) sizeof(x) / sizeof(x[0])
#define size(x) sizeof(x) - sizeof(long)

typedef struct subscription{
    long type;
    int time;
    int notify;
} sub;

typedef struct user{
    pid_t pid;
    int is_logged;
    char passwd[PASSWD_LEN];
    int ipc_id;
    int ipc_msg;
    int bad_login;
    sub subs[MAX_SUBS];
    pid_t blocked[MAX_USERS];
} user;

typedef struct message{
    long type;
    long stype;
    pid_t pid;
    int priority;
    char text[MSG_LEN];
} Message;

typedef struct msg{
    long type;
    int priority;
    pid_t author_pid;
    int notify;
    char text[MSG_LEN];
} Msg;

typedef struct notification{
    long type;
    long stype;
    pid_t pid;
    pid_t author_pid;
} Notif;

typedef struct login_msg{
    long type;
    pid_t pid;
    char passwd[PASSWD_LEN];
}login_msg;

typedef struct sreg_messege{
    long type;
    int is_correct;
    char text[MSG_LEN];
} sreg_msg;


/**>>>>>>>>>>>>>>>> Zmienne globalne do sprzatania po zamknieciu <<<<<<<<<<<**/


    int ipc_req[MAX_USERS]; // tablica wszystkich kolejek do komunikacji  z serwerem 
    int ipc_msg[MAX_USERS]; // tablica wszystkich kolejek do wysylania wiadomosci
    int id = -1;// Glowna kolejka do przyjmowania nowych uzytkownikow
    int types[MAX_TYPES]; // tablica typow wiadomosci dostepnych dla uzytkownikow
    user users[MAX_USERS]; // tablica uzytkownikow

    


/**>>>>>>>>>>>>>>>>>>>>>>> Obsaluga logowania <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/


void login_handling(int ipc_id){
    login_msg m;
    if(msgrcv(ipc_id, &m, size(m), 1, IPC_NOWAIT)>0){
        sreg_msg srmsg;
        srmsg.type = 2;
        for(int i = 0; i < MAX_USERS; i++){
            if(users[i].pid == m.pid && !users[i].is_logged){
                if (users[i].bad_login == MAX_BAD_LOGIN - 1){
                    strcpy(srmsg.text, "KONTO ZABLOKOWANE");
                    srmsg.is_correct = 1;
                    msgsnd(ipc_id, &srmsg, size(srmsg), 0);
                    return;
                }
                else if (strcmp(users[i].passwd, m.passwd) == 0){ // sprawdzenie czy uzytkownik podal dobre haslo
                    users[i].is_logged = 1;                  
                    strcpy(srmsg.text, "Zalogowano pomyslnie!\n");
                    srmsg.is_correct = 1;
                    msgsnd(ipc_id, &srmsg, size(srmsg), 0);
                    return;
                } 
                else {
                    if(users[i].bad_login < MAX_BAD_LOGIN - 1){
                       users[i].bad_login++;
                       sprintf(srmsg.text,         
                                "Wprowadziles zle haslo, jeszcze %d prob/y i zablokujemy konto!\n",
                                MAX_BAD_LOGIN - users[i].bad_login);
                        srmsg.is_correct = 0;
                        msgsnd(ipc_id, &srmsg, size(srmsg), 0);
                        return;
                    }
                }
            } 
            else if(users[i].pid == m.pid && users[i].is_logged){
                sprintf(srmsg.text,"Jestes juz zalogowany twoje pid: %d\n",
                        users[i].pid);
                srmsg.is_correct = 1;
                msgsnd(ipc_id, &srmsg, size(srmsg), 0);
                return;
            }
        }
    }    
}


/**>>>>>>>>>>>>>>>> Obsluga rejestracji uzytkownika <<<<<<<<<<<<<<<<<<<<<<<<**/


typedef struct signing_up{
    long type;
    int a;
} sig_msg;

typedef struct signing_resp {
    long type;
    int ipc_id;
    int ipc_msg;
} sig_resp;


void sign_up_handling(user* users){
    sig_msg sms;
   if(msgrcv(id, &sms, size(sms), 1, IPC_NOWAIT)>0){
        sig_resp srs;
        srs.type = 2;
        for (int i = 0; i < MAX_USERS; i++){
            if (ipc_req[i] == -1){
               srs.ipc_id = 2000 + i;   // tworzenie unikalnych kolejek dla uzytkownika
               ipc_req[i] = msgget(2000 + i, 0766|IPC_CREAT);  // do komunikacji z serwerem
               srs.ipc_msg = 3000 + i;  // oraz do odbierania wiadomosci z subskrybcji
               ipc_msg[i] = msgget(3000 + i, 0766|IPC_CREAT);
               msgsnd(id, &srs, size(srs), 0);
            }
        }
   }
   for(int i =0; i < MAX_USERS; i++){
       if(users[i].pid == -1 && ipc_req[i] != -1){            
           login_msg lmsg;  // FIXME A co jak uzytkownik sie tutaj rozlaczy?
           if(msgrcv(ipc_req[i], &lmsg, size(lmsg), 98, IPC_NOWAIT)>0){
               users[i].pid = lmsg.pid;
               users[i].is_logged = 0;
               strcpy(users[i].passwd, lmsg.passwd);
               users[i].ipc_id = ipc_req[i];
               users[i].ipc_msg = ipc_msg[i];
               users[i].bad_login = 0;

               sreg_msg srmsg;
               srmsg.type = 99;
               srmsg.is_correct = 1;
               sprintf(srmsg.text, "Pomyslnie utworzono konto uzytkownika %d\nhaslo: %s\n", lmsg.pid, lmsg.passwd);
               msgsnd(users[i].ipc_id, &srmsg, size(srmsg), 0);               
            }
        }
    }
}




/**>>>>>>>>>>>>>>>> Obsluga rejestracji typu wiadomosci <<<<<<<<<<<<<<<<<<<<**/


typedef struct reg_messege{
    long type;
    long new_type;
    pid_t pid;
} reg_msg;




void reg_msg_handling(int ipc_id, int* types, user* users){
    
    reg_msg rmsg;
    sreg_msg srmsg;
    int is_user_logged = 0;
    if(msgrcv(ipc_id, &rmsg, size(rmsg), 3, IPC_NOWAIT) > 0){
        srmsg.type = 4;
        srmsg.is_correct = 0;

        for(int i=0; i < MAX_USERS; i++) // Sprawdzenie czy uzytkownik jest zalogowany
            if(users[i].pid == rmsg.pid && users[i].is_logged){
                is_user_logged = 1;
                break;
            }
        if (!is_user_logged){ // Jezeli uzytkownik jest niezalogowany to powiadamiamy go o tym
            strcpy(srmsg.text,"Nie jestes zalogowany!\n");
            msgsnd(ipc_id, &srmsg, size(srmsg), 0);
            return;
        }

        if(rmsg.new_type < 1){ // Sprawdzenie czy typ  > 0
            strcpy(srmsg.text, "Typ musi byc > 0!\n");
            msgsnd(ipc_id, &srmsg, size(srmsg), 0);
            return;
        }

        for(int i=0; i < MAX_TYPES; i++)
            if(types[i] == rmsg.new_type){ // Sprawdzenie czy podany typ jest unikalny
                strcpy(srmsg.text, "Typ juz istnieje!\n");
                msgsnd(ipc_id, &srmsg, size(srmsg), 0);
                return;
            }

        for(int i=0; i < MAX_TYPES + 1; i++){ 
            if (types[i] == -1){
                srmsg.is_correct = 1;    // Zapisanie nowego typu
                types[i] = rmsg.new_type;
                strcpy(srmsg.text, "Pomyslnie utworzono nowy typ!\n");
                break;
            }
            else if(i == MAX_TYPES) {    // Sprawdzenie czy nie przepelnimy tablicy typow i powiadomienie uzytkownika
                strcpy(srmsg.text, "Brak miejsca na nowe typy, skorzystaj z juz istniejacych\n");
                break;
            }
        }
        msgsnd(ipc_id, &srmsg, size(srmsg), 0); // Wysylanie ostatecznej informacji
    }
}

typedef struct reg_message_2{
    long type;
    pid_t pid;
}reg_msg2;

typedef struct sreg_message_2{
    long type;
    long new_type;
    int is_correct;
    char text[MSG_LEN];
} sreg_msg2;

int compare(const void * a, const void * b){    // funkcja porownujaca do qsort()
        int _a = *(int*)a;
        int _b = *(int*)b;
        if(_a < _b) return -1;
        else if(_a == _b) return 0;
        else return 1;
}

void reg_msg_sys_handling(int ipc_id, int* types, user* users){ // utworzenie nowego typu wiadomosci
    reg_msg2 rmsg2;
    sreg_msg2 srmsg2;
    srmsg2.type = 6;
    if(msgrcv(ipc_id, &rmsg2, size(rmsg2), 5, IPC_NOWAIT) > 0){
        for(int i = 0; i < MAX_USERS; i++){
            if(users[i].pid == rmsg2.pid){  // przeszukujemy uztkownikow az znajdziemy tego ktory wysyla wiadomosc
                if(users[i].is_logged){ // jesli jest zalogowany to generujemy dla niego nowy typ
                    qsort(types, MAX_TYPES, sizeof(int), compare); // sortujemy wszystkie typy
                    long new_type = 1;
                    if (types[0] == -1){
                        for (int j = 1; j < MAX_TYPES; j++){
                            if(types[j] == -1) continue;
                            else if (types[j] == new_type) new_type++;
                            else if (types[j] > new_type) break;
                        }
                        srmsg2.new_type = new_type;
                        types[0] = new_type;
                        char buf[MSG_LEN];
                        sprintf(buf, "Pomyslnie utworzono nowy typ = %ld\n", new_type);
                        strcpy(srmsg2.text, buf);
                        srmsg2.is_correct = 1;      // wysylamy nowy typ
                        msgsnd(ipc_id, &srmsg2, size(srmsg2), 0);
                        return;
                    }
                }
                else    // Jezeli uzytkownij jest niezalogowany to przerywamy petle
                    break;
            }
        }
        printf("jestes niezalogowany\n");
        srmsg2.new_type = -1;
        srmsg2.is_correct = 0;  // blad
        strcpy(srmsg2.text, "Jestes niezalogowany!\n");  
        msgsnd(ipc_id, &srmsg2, size(srmsg2), 0);
    }
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>> Subskrybcje <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/


typedef struct subscribe_message{
    long type;
    long stype;
    time_t time;
    int notify;
} sub_msg;

void subscribe_handling(user* usr){
    sub_msg smsg;
    if(msgrcv((*usr).ipc_id, &smsg, size(smsg), 7, IPC_NOWAIT) > 0){
        sreg_msg srmsg;
        srmsg.type = 8;
        if((*usr).is_logged){
            if(smsg.stype > 0) {
                for(int i=0; i < MAX_SUBS; i++){
                    if((*usr).subs[i].type == -1){
                        (*usr).subs[i].type = smsg.stype;
                        if(smsg.time > 0){
                            time_t t;
                            time(&t);
                            (*usr).subs[i].time = t + smsg.time;  // kiedy zakonczyc subskrybcje
                        }
                        else
                            (*usr).subs[i].time = 0;  // subskrybcja trwala
                        (*usr).subs[i].notify = smsg.notify; // powiadomienia
                        srmsg.is_correct = 1;
                        sprintf(srmsg.text, "Pomyslnie zasubskrybowano temat %ld\n", smsg.stype);
                        msgsnd((*usr).ipc_id, &srmsg, size(srmsg), 0);
                        return;
                    }
                }
            }
            else {
                srmsg.is_correct = 0;
                strcpy(srmsg.text, "Typ powinien byc > 0!\n");
                msgsnd((*usr).ipc_id, &srmsg, size(srmsg), 0);
                return;
            }

        }
        srmsg.is_correct = 0;
        strcpy(srmsg.text, "Nie jestes zalogowany!\n");
        msgsnd((*usr).ipc_id, &srmsg, size(srmsg), 0);
    }
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Odbieranie wiadomosci <<<<<<<<<<<<<<<<<<<<<**/


void message_handling(user* usr, user* users){
    Message rmsg;
    if(msgrcv((*usr).ipc_id, &rmsg, size(rmsg), 9, IPC_NOWAIT) > 0){
        sreg_msg srmsg;
        srmsg.type = 10;
        if((*usr).is_logged){
            int type_exist = 0;
            for(int i=0; i<MAX_TYPES; i++){
                if(types[i] == rmsg.stype){
                    type_exist = 1;
                    break;
                }
            }
            if(rmsg.stype > 0 && type_exist){                
                for (int i=0; i < MAX_USERS; i++){
                    if(users[i].pid != -1 && 
                            users[i].is_logged &&   // FIXME sprawdzenie czy typ istnieje 
                            users[i].pid != rmsg.pid){ // wysylamy do innych zalogowanych uzytkownikow
                        for (int j = 0; j < MAX_TYPES; j++){
                                int is_blocked = 0;
                                for(int k=0; k < MAX_USERS; k++){   // Sprawdzenie czy autor wiadomosci nie jest zablokowany
                                    if(users[i].blocked[k] == rmsg.pid){
                                        is_blocked = 1;
                                        break;
                                    }
                                }
                            if(!is_blocked && users[i].subs[j].type == rmsg.stype){
                                time_t t;
                                time(&t);
                                if(users[i].subs[j].time != 0 && users[i].subs[j].time < t){
                                    users[i].subs[j].type = -1;
                                    users[i].subs[j].time = -1;
                                    users[i].subs[j].notify = 0;
                                    break;
                                }
                                Msg new_msg;
                                new_msg.type = rmsg.stype;
                                new_msg.priority = rmsg.priority;
                                new_msg.notify = users[i].subs[j].notify;
                                strcpy(new_msg.text, rmsg.text);
                                new_msg.author_pid = rmsg.pid;
                                // Wyslanie bez czekania w razie przepelnienia,
                                msgsnd(users[i].ipc_msg, &new_msg, size(new_msg), IPC_NOWAIT); // To wina klienta ze nie odbiera
                                break;
                            }
                        }
                    }
                }
                strcpy(srmsg.text, "Wiadomosc zostala rozeslana!\n");
                srmsg.is_correct = 1;
                msgsnd((*usr).ipc_id, &srmsg, size(srmsg), 0);
                return;
            }
            else {
                strcpy(srmsg.text, "Typ wiadomosci jest mniejszy od 1 lub nie istnieje!\n");
                srmsg.is_correct = 0;
                msgsnd((*usr).ipc_id, &srmsg, size(srmsg), 0);
                return;

            }

        }
        else {
            strcpy(srmsg.text, "Nie jestes zalogowany!\n");
            srmsg.is_correct = 0;
            msgsnd((*usr).ipc_id, &srmsg, size(srmsg), 0);
            return;
        }
    }
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Odbieranie powiadomien <<<<<<<<<<<<<<<<<<<<**/


void notify(int ipc_id){
    Notif n;
    if(msgrcv(ipc_id, &n, size(n), 11, IPC_NOWAIT)>0)
        printf("Wiadomosc typu %ld, od klienta %d zostala dostarczona do %d\n", n.stype, n.author_pid, n.pid);
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Obsluga sygnalu <<<<<<<<<<<<<<<<<<<<<<<<<<<**/


void signal_handling(){
    msgctl(id, IPC_RMID, 0);
    for(int i=0; i < MAX_USERS; i++){
        msgctl(ipc_req[i], IPC_RMID, 0);
        msgctl(ipc_msg[i], IPC_RMID, 0);
    }
    exit(0);
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Glowna petla programu <<<<<<<<<<<<<<<<<<<<<**/


int main(int argc, char* argv[]){
    signal(SIGINT, signal_handling);
    id  = msgget(1000, 0766|IPC_CREAT); 
    // wypelnienie tablicy uzytkownikow wartosciami domyslnymi
    for(int i=0; i<MAX_USERS; i++){
        users[i].is_logged = 0;
        users[i].pid = -1;
        strcpy(users[i].passwd, "null");
        users[i].ipc_id = -1;
        users[i].ipc_msg = -1;
        users[i].bad_login = 0;
        for (int j = 0; j < MAX_SUBS; j++){
            users[i].subs[j].type = -1;
            users[i].subs[j].time = -1;
            users[i].subs[j].notify = 0;
        }
        for (int j = 0; j < MAX_USERS; j++)
            users[i].blocked[j] = -1;
    }

                  // wypelnienie tablicy typow, -1 to nieistniejacy typ
    for (int i=0; i<MAX_TYPES; i++)
        types[i] = -1;
    for (int i=0; i< MAX_USERS; i++) // wypelnienie kolejek wartosciami domyslnymi
        ipc_req[i] = -1;
    for (int i=0; i< MAX_USERS; i++)
        ipc_msg[i] = -1;


    
    while(1){
        sign_up_handling(users);
        for (int i=0; i<MAX_USERS; i++){
            if(users[i].pid != -1){
                login_handling(users[i].ipc_id);
                reg_msg_handling(users[i].ipc_id, types, users);
                reg_msg_sys_handling(users[i].ipc_id, types, users);
                subscribe_handling(&users[i]);
                message_handling(&users[i], users);
                notify(users[i].ipc_id);
            }
        }
        sleep(0.3);
    }
    exit(0);


    return 0;
}

