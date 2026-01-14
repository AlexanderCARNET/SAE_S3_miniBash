#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h>

extern char **environ;
#define FRAMES 23
#define LINES 100
#define LINE_LEN 134
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
void kirua();

//ETATS
const int E_CMD = 1;
const int E_ESPACE = 2;
const int E_ARGUMENT = 3;
const int E_FIN = 0;

int main(int argc, char** argv) {

    signal(SIGINT, sigint_handler);
    strcpy(PATH,getenv("PATH"));
    printf("Bonjour %s\n", getenv("USER"));
    printf("         X     X         XXXXX                     X\n");
    printf("        XXX  XXXX        X   XXX                   X\n");
    printf("        X XX X  X        X     XX  XXXXXX   XXXXXX X\n");
    printf("       X   XX   X        X    XXX X     XX XX      X\n");
    printf("       X        X        XXXXXX  X       X XX      XXXXXX\n");
    printf(" X    XX        X   XX   X    XX   XXXXXXX  XXXXX  X    XX\n");
    printf("XX    X         X    XX  X    XXX  XX  XXX      XX X     X\n");
    printf(" XXXXXX          XXXX    XXXXXX     XXXX XXXXXXXXX X     X\n");
    printf("\n");
    printf("Bienvenue dans mbash, bash minimaliste en C.\n");
    printf("Taper 'exit' pour quitter.\n");
    printf("Taper 'history' pour afficher l'historique des commandes.\n");



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
    

    //gestion de l'historique
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


    if (cmd[0] == '\f') {
        write(1, "\033[H\033[J", 6);
        return;
    }


    if (strchr(argv[0], '/')) {
        execve(argv[0], argv, environ);
        perror("execve");
        return;
}


   if (strcmp(argv[0], "kirua") == 0) {
        kirua();
        return;
}

    if(strcmp(argv[0],"gohan") == 0){
        printf("Gohan est un personnage de Dragon Ball\n");
        printf("###########################################################################\n"
"###########################################################################\n"
"########################     ######         .##############################\n"
"####################    ####        #####      ############################\n"
"##################  ######################## #    #########################\n"
"################          ####################     ########################\n"
"##################  ############################# .########################\n"
"################  ################################ ########################\n"
"############### #   ############### ########## ###  #######################\n"
"#############    ###    #########   ##### ### #####  ######################\n"
"##################  ###########  # ####   ###  ####. ######################\n"
"################- #####  # ##      ##  #  #    #  #  ######################\n"
"###############         ### # #    #  ##    #-   #  .######################\n"
"############### ######  ##     ###  ########      ###     #################\n"
"######################     # # ####  ##  ### #         ####################\n"
"#################### ### ##  #  ############    # #####  ##################\n"
"###################.+  ##  #  #   -## ###  ## ## #    ## -#################\n"
"####################  ### #    ####      ###   -# ####    #################\n"
"#################### #     ###  ###.    ##-  ####    #  ###################\n"
"###################-  #    ##+      ###    .##### ###  ####################\n"
"#######################  #  ######     ########  ##  #  ###################\n"
"#######################  # #   #.   ####      # ## ####  ##################\n"
"####################### #  #  ###       ### ## ## #######  ################\n"
"######################  ## ####   ######      ### ######### ###############\n"
"#####################  ### ######       ### #####   ####      #############\n"
"#####################  ### ##############  ######   ##  ### #  -###########\n"
"###########     #    #   #   ##########+######## ###      ##    ###########\n"
"########## ##     ##+ ######    ##           ### ####      #   ############\n"
"##########  ##  # ##  ####                +#     #####    #   #############\n"
"###########  ## # #   .   ##    +   ###      #   #######    ###############\n"
"############     #########  ### .##        #   # ##########################\n"
"#########################  ### ####  ######## ##  #########################\n"
"#######################       #####  ######### ##  ########################\n"
"###################    ########### # #############  #######################\n"
"#################   ############## # ##############  ######################\n"
"################  ################   # #############  #######################\n"
"###############  ##############   #   ############### #######################\n"
"##############  ################          ###########  #######################\n"
"#############  ##############      ##   ###-  +####### #######################\n"
"#############  #############   ######## #  ###########    ###################\n"
"##############  # #######.   ##########  ##  #########  #  ###################\n"
"###############     ##   #. ###########  ###  ########  - -###############\n"
"###################    ###  ############# #############  #  ###############\n"
"##################   -##   ##############  #  ########## ## +##############\n"
"##################      ##################  ##  .######. ## ###############\n"
"###################  ######################-  ###    ##  #  ###############\n"
"############################################  #  ###### #    ##############\n"
"############################################.  ### ###       ##############\n"
"##############################################           ###  #############\n"
"##################################################  ###### -  #############\n"
"####################################################           ############\n"
"######################################################  ###   + ###########\n"
"#######################################################- .##     ##########\n"
"#########################################################  ##    ##########\n"
"###########################################################       #########\n");
return;
    }



    //gestion du cd
    if(strcmp(argv[0], "cd") == 0){
        if(argv[2] != NULL){
            printf("Trop d'argument\n");
            return;
        }else if(argv[1] == NULL){
            printf("Il manque 1 argument\n");
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

void kirua() {
    FILE *file = fopen("ascii-animation.txt", "r");
    if (!file) {
        printf("Erreur : fichier 'ascii-animation.txt' introuvable dans le dossier courant.\n");
        return;
    }

    // Static to avoid stack overflow (300KB is large for stack)
    static char frames[FRAMES][LINES][LINE_LEN];

    // Initialize memory
    memset(frames, 0, sizeof(frames));

    // Loading
    for (int f = 0; f < FRAMES; f++) {
        // Read the separator line if present, or just lines
        // Based on previous file, we assume pure lines. 
        // If the file has metadata lines, fget logic might need adjustment.
        for (int l = 0; l < LINES; l++) {
            if (fgets(frames[f][l], LINE_LEN, file) == NULL) break;
            
            // remove newline for cleaner printing if needed, 
            // but your loop uses fputs which expects null-termination
            size_t len = strlen(frames[f][l]);
            if(len > 0 && frames[f][l][len-1] == '\n') {
                frames[f][l][len-1] = '\0';
            }
        }
    }
    fclose(file);

    printf("\033[?25l"); // Hide cursor

    for (int loop = 0; loop < 3; loop++) {
        for (int f = 0; f < FRAMES; f++) {
            printf("\033[H\033[2J"); // Clear screen
            
            // Skip the first line if it contains metadata like 
            int start_line = (strncmp(frames[f][0], "[source", 7) == 0) ? 1 : 0;

            for (int l = start_line; l < LINES; l++) {
                if (frames[f][l][0] != '\0') {
                    printf("%s\n", frames[f][l]);
                }
            }
            fflush(stdout);
            usleep(100000); // 100ms
        }
    }

    printf("\033[?25h"); // Show cursor
}
