// IPC
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdio.h> // input-output
#include <stdio_ext.h>
#include <stdlib.h> // system
#include <string.h> // text
#include <signal.h> // sygnaly

// pid
#include <sys/types.h>
#include <unistd.h>


#define PASSWD_LEN 100
#define MSG_LEN 1024
#define MAX_TYPES 50
#define MAX_BUF 20

#define LEN(x) sizeof(x) / sizeof(x[0])
#define size(a) sizeof(a) - sizeof(long)
#define wrong_cmd() system("clear"); \
                    printf("Nieznana komenda, sproboj ponownie\n"); \
                    continue;
                    




/** struktury wiadomosci **/
typedef struct message{ // wiadomosc do rozglaszania
    long type;
    long stype; // typ ktory bedzie rozglaszany
    pid_t pid; // pid autora - do blokowania przez innych uzytkownikow
    int priority;   // priorytet
    char text[MSG_LEN];
} Message;

typedef struct msg {
    long type;
    int priority;
    pid_t author_pid;
    int notify;
    char text[MSG_LEN];
} Msg;

typedef struct notification {
    long type;
    long stype;
    pid_t pid;
    pid_t author_pid;
} Notif;


typedef struct login_msg{ // wiadomosc logowania
    long type;
    pid_t pid;
    char passwd[PASSWD_LEN];
} login_msg;

typedef struct sreg_massege{ // odpowiedz serewra
    long type;
    int is_correct; // uzywane przez serwer do potwierdzenia rejestracji w informacji zwrotnej
    char text[MSG_LEN];
} sreg_msg;

typedef struct signing_up{
    long type;
    int a;
} sig_msg;

typedef struct signing_resp {
    long type;
    int ipc_id; // serwer zwroci id kolejki do komunikacji,
    int ipc_msg; // oraz do odbierania wiadomosci 
} sig_resp;

typedef struct ipc_ids{
    int ipc_id;
    int ipc_msg;
} ids;

typedef struct reg_massege{ // rejestracja nowego typu
    long type;
    long new_type;
    pid_t pid; // sluzy do uwierzytelnienia
} reg_msg;


typedef struct reg_message_2{
    long type;
    pid_t pid;
} reg_msg2;

typedef struct sreg_message_2{
    long type;
    long new_type;
    int is_correct;
    char text[MSG_LEN];
}sreg_msg2;



/**>>>>>>>>>>>>>>>>>> Rejestracja <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/


ids sign_up(){
    int id = msgget(1000,0766|IPC_CREAT); // główna kolejka do informowania serwera o checi rejestracji
    sig_msg sms;
    sms.type = 1;
    sms.a = 1;
    msgsnd(id, &sms, size(sms), 0);
    sig_resp srs;
    msgrcv(id, &srs, size(srs), 2, 0); // odbior informacji z serwera


    ids my_ids;
    my_ids.ipc_id = msgget(srs.ipc_id, 0766|IPC_CREAT);
    my_ids.ipc_msg = msgget(srs.ipc_msg, 0766|IPC_CREAT);

    login_msg lmsg;
    lmsg.type = 98;
    lmsg.pid = getpid();
    
    printf("Utworz haslo do swojego konta.\n");
    printf("\tHASLO: ");
    scanf("%99s", lmsg.passwd);

    msgsnd(my_ids.ipc_id, &lmsg, size(lmsg), 0);
    
    sreg_msg srmsg;
    msgrcv(my_ids.ipc_id, &srmsg, size(srmsg), 99, 0);
    system("clear");
    printf("%s", srmsg.text);

    return my_ids;
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>> logowanie na serwer <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/



void login(int ipc_id){

    char passwd[PASSWD_LEN];
    printf("\tHASLO: ");
    scanf("%99s", passwd); // pobiera jedno slowo FIXME
    
    
    login_msg lmsg;
    lmsg.type = 1;
    lmsg.pid = getpid();
    strcpy(lmsg.passwd, passwd);

    msgsnd(ipc_id, &lmsg, size(lmsg), 0); // wyslanie loginu i pid na serwer
    
    sreg_msg buf;
    msgrcv(ipc_id, &buf, size(buf), 2, 0); // pobranie odpowiedzi z serwera
    system("clear");
    printf("%s\n", buf.text);
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>> rejestracja nowego typu wiadomosci <<<<<<<<<<<<<<<<<<<<<<**/



int reg_msg_t_man(int ipc_id){   // reczna rejestracja
    while(1){
        long type;
        printf("Podaj typ wiadomosci: ");
        if(!scanf("%ld", &type)){
            scanf("%*s");
            wrong_cmd();
        }
        
        reg_msg rmsg;
        rmsg.type = 3;
        rmsg.new_type = type;
        rmsg.pid = getpid();
        
        msgsnd(ipc_id, &rmsg, size(rmsg), 0); // wyslanie nowego typu
        
        sreg_msg buf;
        msgrcv(ipc_id, &buf, size(buf), 4, 0);  // odbieranie wiadomosci od serwera czy operacja sie powiodla

        system("clear");
        printf("%s", buf.text); // wiadomosc zwrotna od serwera
        if(buf.is_correct){
            return 0;
        } else {
            int try_again = 0;
            while(!try_again) {
                printf("Nie mozna utworzyc tego typu, czy chcesz sprobowac ponownie?\n1.\tTak\n2.\tNie\n");
                int choice;
                if(!scanf("%d", &choice)){
                    scanf("%*s");
                    wrong_cmd();
                }
                else if(choice == 1){
                    system("clear");
                    break;
                }
                else if(choice ==  2){
                    system("clear");
                    return -1;
                }
                else {
                    wrong_cmd();
                }
            }
        }
    }
}


int reg_msg_t_sys(int ipc_id){   // rejestracja nowego typu wiadomosci przez serwer
    reg_msg2 rmsg2;
    rmsg2.type = 5;
    rmsg2.pid = getpid();
    msgsnd(ipc_id, &rmsg2, size(rmsg2), 0);
    
    sreg_msg2 srmsg2;
    msgrcv(ipc_id, &srmsg2, size(srmsg2), 6, 0);

    system("clear");
    printf("%s", srmsg2.text);
    if(!srmsg2.is_correct)
        return -1;
    else
        return 0;
}
    

void reg_msg_t(int ipc_id){
    int done = 0;
    while (!done){
        int choice = 0;
        printf("1.\tUtworz nowy typ wiadomosci recznie\n");
        printf("2.\tUzyskaj nowy typ wiadomosci od systemu\n");
        printf("3.\tAnuluj\n");
        if(!scanf("%d", &choice)){
            scanf("%*s");
            wrong_cmd();
        }
        system("clear");
        switch(choice){
            case 1:
                if(reg_msg_t_man(ipc_id) != -1){
                    done = 1;    // zarejestruj nowy typ recznie
                }
                break;
            case 2:
                if(reg_msg_t_sys(ipc_id) != -1){ // zarejsetruj nowy typ systemowo
                    done = 1;
                }
                break;
            case 3:
                return;
            default:
                wrong_cmd();
        }
    }
}


/**>>>>>>>>>>>>>>>>>>>>>>>> Subskrybcja wiadomosci <<<<<<<<<<<<<<<<**/

typedef struct subscribe_message{
    long type;
    long stype;
    time_t time;
    int notify;
} sub_msg;


int subscribe(int ipc_id){
    sub_msg smsg;
    smsg.type = 7;
    smsg.time = -1;
    int choice;
    while(1){
        printf("Jaki typ wiadomosci chcesz zasubskrybowac?: ");
        if(!scanf("%ld", &smsg.stype)){
            scanf("%*s");
            wrong_cmd();
        }
        break;
    }
    while(smsg.time < 0){
        while(1){
            printf("Czy ma to byc subskrybcja trawala?\n");
            printf("\t1. tak\n");
            printf("\t2. nie\n");
            if(!scanf("%d", &choice)){
                scanf("%*s");
                wrong_cmd();
            }
            break;
        }
        switch (choice) {
            case 1:
                smsg.time = 0;
                break;
            case 2:
                while(1) {
                    printf("Na ile sekund chcesz zasubskrybowac wiadomosci typu %ld?: ", smsg.stype);
                    if(!scanf("%ld", &smsg.time)){
                        scanf("%*s");
                        wrong_cmd();
                    }
                    break;
                }
                break;
            default:
                wrong_cmd();
        }
    }
    while(1){
        printf("Czy chcesz powiadamiać serwer ze odebrales wiadomosc?\n");
        printf("\t1. tak\n");
        printf("\t2. nie\n");
        if(!scanf("%d", &choice)){
            scanf("%*s");
            wrong_cmd();
        }
        if (choice == 1){
            smsg.notify = 1;
            break;
        }
        else if (choice == 2){
            smsg.notify = 0;
            break;
        }
        else
            continue;
    }

    msgsnd(ipc_id, &smsg, size(smsg), 0);

    sreg_msg srmsg;
    msgrcv(ipc_id, &srmsg, size(srmsg), 8, 0);

    system("clear");
    printf("%s", srmsg.text);
    if(srmsg.is_correct)
        return 0;
    else
        return -1;
        
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>> wysylanie wiadomosci <<<<<<<<<<<<<<<<<<<<<<<<<<**/


int send_message(int ipc_id){
    Message msg;
    msg.type = 9;
    while(1){
        printf("Podaj typ wiadomosci:\n");
        if(!scanf("%ld", &msg.stype)){
            scanf("%*s");
            wrong_cmd();
        }
    break;
    }
    while(1){
        printf("Podaj priorytet wiadomosci:\n");
        if(!scanf("%d", &msg.priority)){
            scanf("%*s");
            wrong_cmd();
        }
    break;
    }
    printf("Podaj tresc wiadomosci: \n");
    size_t msg_len = 0;
    char* line = NULL;
    for(int i=0; i<2; i++){ //FIXME pierwszy getline nie czekal na input
        int X = getline(&line, &msg_len, stdin);
        if(X>0)
            strcpy(msg.text, line);
    }
    free(line);
    msg.pid = getpid();
    msgsnd(ipc_id, &msg, size(msg), 0);   
   
    sreg_msg srmsg;
    msgrcv(ipc_id, &srmsg, size(srmsg), 10, 0);
    
    system("clear");
    printf("%s", srmsg.text);
    if(srmsg.is_correct)
        return 0;
    else
        return -1;
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>> odbieranie wiadomosci synchronicznie <<<<<<<<<<<<<<**/

int compare_priority(const void *a, const void *b){
    Msg _a = *(Msg*)a;
    Msg _b = *(Msg*)b;
    return (_a.priority - _b.priority); // najwyzszy piorytet najpierw;
}


int receive_message(int ipc_id, int ipc_msg, Msg* msg_buf){
    Msg msg;

    int i = 0;
    while(msgrcv(ipc_msg, &msg, size(msg), 0, IPC_NOWAIT)>0){
        if(msg.notify){
            Notif n;
            n.type = 11;
            n.stype = msg.type;
            n.pid = getpid();
            n.author_pid = msg.author_pid;
            msgsnd(ipc_id, &n, size(n), IPC_NOWAIT);
        }
        msg_buf[i].type = msg.type;
        msg_buf[i].priority = msg.priority;
        msg_buf[i].author_pid = msg.author_pid;
        strcpy(msg_buf[i].text, msg.text);
       
        printf("%d\n", msg_buf[i].author_pid);
        i++;
    }

    qsort(msg_buf, MAX_BUF, sizeof(msg), compare_priority); // sortowanie od najwyzszego do najnizszego priorytetu

    for(int j = 0; j < MAX_BUF; j++){
        if(msg_buf[j].type == -1) continue;
        printf("Wiadomosc %ld priorytecie %d, od %d o tresci\n\t%s\n",
                msg_buf[j].type, msg_buf[j].priority, msg_buf[j].author_pid, msg_buf[j].text);
        msg_buf[j].type = -1;
        msg_buf[j].priority = -1;
        msg_buf[j].author_pid = -1;
        strcpy(msg_buf[j].text, "null");
    }
    return 0;
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> odbieranie wiadomosci asynchronicznie <<<<<<<<<<<<<<<<<<<**/


void receive_asynchr(int ipc_id, int ipc_msg){
    Msg msg;
    if(msgrcv(ipc_msg, &msg, size(msg), 0, IPC_NOWAIT)>0){
         if(msg.notify){
              Notif n;
              n.type = 11;
              n.stype = msg.type;
              n.pid = getpid();
              n.author_pid = msg.author_pid;
              msgsnd(ipc_id, &n, size(n), IPC_NOWAIT);
         }
         printf("Odebrano wiadomosc na temat %ld, od %d o tresci\n\t%s\n", 
                 msg.type, msg.author_pid, msg.text);
    }
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Blokowanie uzytkownika <<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/

typedef struct block_message{
    long type;
    pid_t pid;
} Block_msg;

void block(int ipc_id){
   Block_msg msg;
   msg.type = 12;
    while(1){
        printf("Podaj pid uzytkownika do zablokowania:\n");
        if(!scanf("%d", &msg.pid)){
            scanf("%*s");
            wrong_cmd();
            continue;
        }
        break;
    }
    msgsnd(ipc_id, &msg, size(msg), 0);
}
    


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MAIN <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/
int main(int argc, char* argv[]){
    int asynchr = 0;
    if(argc > 1)
        if(strcmp(argv[0], "-a"))
                asynchr = 1;
                
         
    int choice = 0;             // wybor do menu
    ids my_ids = sign_up(); // rejestracja
    Msg buf[MAX_BUF];
    for (int i=0; i<MAX_BUF; i++){
        buf[i].type = -1;
        buf[i].priority = -1;
        buf[i].author_pid = -1;
        strcpy(buf[i].text, "null");
    }
    int child_pid;
    if(asynchr)
        if((child_pid = fork()) == 0)
            while(1){
                sleep(1); // raz na sekunde odbiera wiadomosc
                if(getppid() == 1)
                    exit(0);
                receive_asynchr(my_ids.ipc_id, my_ids.ipc_msg);
            }

    // Glowna petla programu
    while(1){
        printf("Co chcesz zrobic?\n");
        printf("\t1. Zaloguj sie do systemu\n");
        printf("\t2. Zarejestruj sie na konkretne wiadomosci\n");
        printf("\t3. Zablokuj użytkownika\n");
        printf("\t4. Stworz nowy typ wiadomosci\n");
        printf("\t5. Rozglaszaj wiadomosc\n");
        if(!asynchr)
            printf("\t6. Odbierz wiadomosci synchronicznie\n");        
        printf("\t0. Wyjdz z programu\n");
        if (!scanf("%d", &choice)){
            scanf("%*s");
            wrong_cmd();
        }
        system("clear");

        switch(choice){
            case 1:
                login(my_ids.ipc_id);
                break;
            case 2:
                subscribe(my_ids.ipc_id);
                break;
            case 3:
                block(my_ids.ipc_id);
                break;
            case 4:
                reg_msg_t(my_ids.ipc_id);
                break;
            case 5:
                send_message(my_ids.ipc_id);
                break;
            case 6:
                if(asynchr){
                    wrong_cmd();
                    break;
                }
                receive_message(my_ids.ipc_id, my_ids.ipc_msg, buf);
                break;                   
            case 0:
                if(asynchr)
                    kill(child_pid, SIGKILL);
                exit(0);
                break;
            default:
                wrong_cmd();
        }
    }
    return 0;
} 
