// IPC
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdio.h> // input-output
#include <stdio_ext.h>
#include <stdlib.h> // system
#include <string.h> // text

// pid
#include <sys/types.h>
#include <unistd.h>


#define NAME_LEN 100
#define MSG_LEN 1024
#define MAX_TYPES 50

#define size(a) sizeof(a) - sizeof(long)
#define wrong_cmd() system("clear"); \
                    scanf("%*s"); \
                    printf("Nieznana komenda, sproboj ponownie\n"); \
                    continue
    
/** struktury wiadomosci **/
typedef struct message{ // wiadomosc do rozglaszania
    long type;
    int priority;
    char text[MSG_LEN];
} messege;


/**>>>>>>>>>>>>>>>>>>>>>>>>>>> logowanie na serwer <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/


typedef struct login_msg{ // wiadomosc logowania
    long type;
    pid_t pid;
    char name[NAME_LEN];
} login_msg;

typedef struct server_response{ // wiadomosc zwrotana serwera
    long type;
    char text[MSG_LEN];
} sr_msg;


void login(){
    int id = msgget(1000, 0766|IPC_CREAT);

    char name[NAME_LEN];
    printf("\nPodaj swoja nazwe: ");
    scanf("%99s", name); // pobiera jedno slowo FIXME
    
    
    login_msg lmsg;
    lmsg.type = 1;
    lmsg.pid = getpid();
    strcpy(lmsg.name, name);

    msgsnd(id, &lmsg, size(lmsg), 0); // wyslanie loginu i pid na serwer
    
    sr_msg buf;
    msgrcv(id, &buf, size(buf), 2, 0); // pobranie odpowiedzi z serwera
    
    printf("%s\n", buf.text);
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>> rejestracja nowego typu wiadomosci <<<<<<<<<<<<<<<<<<<<<<**/


typedef struct reg_massege{ // rejestracja nowego typu
    long type;
    long new_type;
    int is_correct; // uzywane przez serwer do potwierdzenia rejestracji w informacji zwrotnej
} reg_msg;


long reg_msg_t_man(){   // reczna rejestracja
    return -1;
    int id = msgget(1000, 0766|IPC_CREAT);
    while(1){
        long type;
        printf("Podaj typ wiadomosci: ");
        if(!scanf("%ld", &type)){
           wrong_cmd();
        }
        
        reg_msg rmsg;
        rmsg.type = 3;
        rmsg.new_type = type;
        
        msgsnd(id, &rmsg, size(rmsg), 0); // wyslanie nowego typu
        
        reg_msg buf;
        msgrcv(id, &buf, size(buf), 4, 0);

        if(buf.is_correct){
            printf("Pomyslnie utworzono nowy typ!\n");
            return buf.new_type;
        } else {
            while(1) {
                printf("Nie mozna utworzyc tego typu, czy chcesz sprobowac ponownie?\n1.\tTak\n2.\tNie\n");
                int choice;
                if(!scanf("%d", &choice)){
                    wrong_cmd();
             }
                switch(choice){
                    case 1:
                        break;
                    case 2:
                        return -1;
                    default:
                        wrong_cmd();
                }
            }
        }
    }
}


void reg_msg_t(int* my_types, int idx, int length){
    int done = 0;
    int type;
    while (!done){
        int choice = 0;
        printf("1.\tUtworz nowy typ wiadomosci recznie\n");
        printf("2.\tUzyskaj nowy typ wiadomosci od systemu\n");
        printf("3.\tAnuluj\n");
        if(!scanf("%d", &choice)){
            wrong_cmd();
        }
        system("clear");
        switch(choice){
            case 1:
                if((type = reg_msg_t_man()) > 0){
                    done = 1;    // zarejestruj nowy typ recznie
                    my_types[idx] = type;
                }
                break;
            case 2:
                done = 1;
//                reg_msg_t_sys();    // zarejsetruj nowy typ systemowo
                break;
            case 3:
                return;
            default:
                wrong_cmd();
        }
    }
}

int main(int argc, char* argv[]){
    int choice = 0;             // wybor do menu
    int last_type_idx = -1;     // tablica typow utworzonych przez tego uzytkownika
    int my_types[MAX_TYPES];
    // Glowna petla programu
    while(1){
        printf("Co chcesz zrobic?\n");
        printf("1.\tZaloguj sie do systemu\n");
        printf("2.\tZarejestruj sie na konkretne wiadomosci\n");
        printf("3.\tStworz nowy typ wiadomosci\n");
        printf("4.\tRozglaszaj wiadomosc\n");
        printf("5.\tOdbierz wiadomosci synchronicznie\n");
        printf("6.\tOdbierz wiadomosci asynchronicznie\n");
        printf("7.\tWyjdz z programu\n");
        if (!scanf("%d", &choice)){
            wrong_cmd();
        }
        system("clear");

        switch(choice){
            case 1:
                login();
                break;
            case 3:
                reg_msg_t(my_types, last_type_idx++, size(my_types));
                printf("%d\n", my_types[last_type_idx -1]);
                break;
            case 7:
                exit(0);
                break;
            default:
                wrong_cmd();
        }
    }

    return 0;
} 
