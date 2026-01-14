#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h>

extern char **environ;

#define MAXLI 2048
char PATH[MAXLI];
char cmd[MAXLI];
char* argv[MAXLI];
char* history[MAXLI];
int histcount = 0;
int continu = 1;
extern char **environ;
pid_t current_child = -1;


//declaration des fonctions
void mbash(char cm[MAXLI]);
void setArgs(char cm[MAXLI]);
void printHistory();
void sigint_handler(int sig);
int existFile(char* path);
void cd(char* chemin);
void executerCommande(char cm[MAXLI]);

//ETATS
#define E_FIN 100
#define E_DEBUT 0
#define E_INIT 1
#define E_ESPACE 2
#define E_ARGUMENT 3
#define E_POINT 4
#define E_POINT_SLASH 5
#define E_FORK 6
#define E_ERROR -1


int main(int argc, char** argv) {

    signal(SIGINT, sigint_handler);
    strcpy(PATH,getenv("PATH"));

    while (continu) {
        strcpy(PATH,getenv("PATH"));
        printf("Commande: ");

        if (fgets(cmd, MAXLI, stdin) == NULL) {
            // fgets interrompu par Ctrl+C ou EOF
            continue;
        }
        mbash(cmd);
    }
    return 0;
}



void mbash(char cm[MAXLI]) {

    //on retire le passage a la ligne a la fin de la commande
    int size = strlen(cm);
    cm[size - 1] = '\0';

    //on ajoute la commande à l'historique
    if(histcount < MAXLI) {
        if(cm[0] != '\0') {
            history[histcount] = strdup(cm);
            histcount++;
        }
    }

    //on lance l'automate
    char commande[MAXLI];
    int ind = 0;
    int indCommande = 0;
    int etat = E_DEBUT;
    while(etat != E_FIN && etat != E_FORK){
        char c = cm[ind];
        switch(etat){
            case E_DEBUT:
                if(c == '\0'){
                    etat = E_FIN;
                }else if(c == '.'){
                    etat = E_POINT;
                }else if(c == ' '){
                    etat = E_DEBUT;
                }else{
                    etat = E_INIT;
                    commande[indCommande] =  c;
                    indCommande++;
                    commande[indCommande] =  '\0';
                }
                break;

            case E_INIT:
                if(c == '\0'){
                    etat = E_FIN;
                }else if(c == ' '){
                    etat = E_ESPACE;
                    commande[indCommande] =  c;
                    indCommande++;
                    commande[indCommande] =  '\0';
                }else{
                    commande[indCommande] =  c;
                    indCommande++;
                    commande[indCommande] =  '\0';
                }
                break;

            case E_ARGUMENT:
                if(c == '\0'){
                    etat = E_FIN;
                }else if(c == ' '){
                    etat = E_ESPACE;
                    commande[indCommande] =  c;
                    indCommande++;
                    commande[indCommande] =  '\0';
                }else{
                    etat = E_ARGUMENT;
                    commande[indCommande] =  c;
                    indCommande++;
                    commande[indCommande] =  '\0';
                }
                break;

            case E_ESPACE:
                if(c == '\0'){
                    etat = E_FIN;
                }else if(c == ' '){
                    etat = E_ESPACE;
                }else if(c == '&'){
                    etat = E_FORK;
                }else{
                    etat = E_ARGUMENT;
                    commande[indCommande] =  c;
                    indCommande++;
                    commande[indCommande] =  '\0';
                    
                }
                break;

            case E_POINT:
                break;

            case E_POINT_SLASH:
                break;

            case E_FORK:
                if(fork() == 0){
                    executerCommande(commande);
                    exit(1);
                }
                break;

            case E_ERROR:
                break;

            default:
                etat = E_ERROR;
        }
        ind++;
    }

    if(etat == E_FIN){
        executerCommande(commande);
    }

    //on efface la commande
    int i = 0;
    while(commande[i] != '\0'){
        commande[i] = '\0';
        i++;
    }
    
}

void executerCommande(char cm[MAXLI]){
    //on met a jour la list d'arguments de la commande
    setArgs(cm);

    //on verifie si la commande est vide
    if (argv[0] == NULL) return;

    //on cherche le repertoir dans le path ou la commande existe
    char chemin[MAXLI];
    int trouvee = 0;
    char* extraitPATH = strtok(PATH,":");
    while(extraitPATH != NULL && !trouvee){
        strcpy(chemin, extraitPATH);
        strcat(chemin, "/");
        strcat(chemin, argv[0]);
        trouvee = existFile(chemin);
        if(!trouvee){
            extraitPATH = strtok(NULL, ":");
        }
         
    }


    if(trouvee){
        printf("Execute: %s\n", chemin);
    }
    

    //gestion de l'history
    if (strcmp(argv[0], "history") == 0) {
        if(argv[1] != NULL){
            printf("La commande ne prend pas d'argument\n");
        }else{
            printHistory();
        }
        
        return;
    } 

    //gestion du exit
    if (strcmp(argv[0], "exit") == 0) { 
        continu = 0;
        return;
    }

    //gestion du cd
    if(strcmp(argv[0], "cd") == 0){
        if(argv[2] != NULL){
            printf("Trop d'argument\n");
            return;
        }else if(argv[1] == NULL){
            char cheminUser[1000] = "/home/"; 
            strcat(cheminUser,getenv("USER"));
            cd(cheminUser);
            return ;
        }else{
            cd(argv[1]);
            return ;
        }
    }


    pid_t pid = fork();


    if (pid == 0) {
        signal(SIGINT, SIG_DFL);  // l’enfant réagit normalement à Ctrl+C
        execve(chemin, argv, environ);
        perror("execve");
        exit(1);
    } else if (pid > 0) {
        current_child = pid;
        waitpid(pid, NULL, 0);
        current_child = -1;
    } else {
        perror("fork");
    }
}


void setArgs(char cm[MAXLI]){
    //on vide la liste des anciens arguments
    for(int i = 0; i < MAXLI; i++) argv[i] = NULL;
    
    //on separe la commande à chaque espace avec un automate
    int indArg = 0; //numero de l'argument de la commande, le 0 correspond au nom de la commande
    int ind = 0; //indice dans la commande de l'utilisateur

    char *token = strtok(cm, " ");
    while(token != NULL){
        argv[indArg++] = token; 
        token = strtok(NULL, " ");
    }

    argv[indArg] = NULL;
}


int existFile(char* path){
    FILE *f = fopen(path,"r");
    if(f){
        fclose(f);
        return 1;
    } 
    return 0;
}


void printHistory() {
    for (int i = 0; i < histcount; i++) {
        //Affichage historique comme dans history de BASH : 'N° de commande  <commande>'
        printf("%d  %s\n", i + 1, history[i]);
    }
}

void sigint_handler(int sig) {
    if (current_child > 0) {
        write(1, "\n", 1);
    }
}

void cd(char* chemin){
    chdir(chemin);
}


