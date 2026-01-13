#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#define MAXLI 2048
char PATH[MAXLI];
char cmd[MAXLI];
char path[MAXLI] = "/usr/bin/";
int pathidx;
char* argv[MAXLI];
//declaration des fonctions
void mbash(char cm[MAXLI]);
void setArgs(char cm[MAXLI]);

//ETATS
const int E_CMD = 1;
const int E_ESPACE = 2;
const int E_ARGUMENT = 3;
const int E_FIN = 0;

int main(int argc, char** argv) {

    strcpy(PATH,getenv("PATH"));

    if(PATH != NULL){
        printf("path : %s", PATH);
    }

  while (1) {
    strcpy(PATH,getenv("PATH"));
    printf("Commande: ");
    fgets(cmd, MAXLI, stdin);
    mbash(cmd);
  }
  return 0;
}


void mbash(char cm[MAXLI]) {

    //on retire le passage a la ligne a la fin de la commande
    int size = strlen(cm);
    cm[size - 1] = '\0';

    //on met a jour la list d'arguments de la commande
    setArgs(cm);

    //on construit le chemin dans /usr/bin et la commande
    char chemin[MAXLI];
    //strcpy(chemin, path);
    //strcat(chemin, argv[0]);

    //on cherche le repertoir dans le path ou la commande existe
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


    printf("Execute: %s\n", chemin);

    

    //Exécution de la commande dans un processus enfant
    if (fork() == 0) {
        execve(chemin, argv, NULL);
        perror("execve");
        exit(1);
    } else {
        wait(1000);
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

