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
#define LINE_LEN 137
#define MAXLI 2048
#define MAX_VAR 100



//declaration structure 
typedef struct {
    char name[64];
    char value[256];
} shell_var;

//declaration des variables
char *GOKU_PATH;
char PATH[MAXLI];
char cmd[MAXLI];
char* argv[MAXLI];
char* history[MAXLI];
int histcount = 0;
int continu = 1;
extern char **environ;
pid_t current_child = -1;
int animation_running = 0;
shell_var vars[MAX_VAR];
int var_count = 0;


//declaration des fonctions
void mbash(char cm[MAXLI]);
void setArgs(char cm[MAXLI]);
void printHistory();
void sigint_handler(int sig);
int existFile(char* path);
void cd(char* chemin);
void executerCommande(char cm[MAXLI]);
void goku();
char *get_var(const char *name);
void set_var(const char *name, const char *value);




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
    goku_path_absolute();
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
    printf("Les commandes fonctionnelles sont :\n");
    printf(" - Celle presente dans le PATH\n");
    printf(" - exit\n");
    printf(" - history\n");
    printf(" - goku\n");
    printf(" - gohan\n");
    printf(" - ./ (éxécution d'un fichier)\n");
    printf(" - cd <répertoire>\n");
    
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

    char *eq = strchr(cm, '=');
    if (eq && cm[0] != '=') {
        *eq = '\0';
        char *name = cm;
        char *value = eq + 1;

        if (value[0] == '$') {
            char *v = get_var(value + 1);
            if (v) value = v;
        }

        set_var(name, value);
        return;
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

    for (int i = 0; argv[i]; i++) {
    if (argv[i][0] == '$') {
        char *val = get_var(argv[i] + 1);
        if (val) {
            argv[i] = val;
        }
    }
}

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


   if (strcmp(argv[0], "goku") == 0) {
        signal(SIGINT, sigint_handler);
        goku();
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
"################  ################   # #############  #####################\n"
"###############  ##############   #   ############### #####################\n"
"##############  ################          ###########  ####################\n"
"#############  ##############      ##   ###-  +####### ####################\n"
"#############  #############   ######## #  ###########    #################\n"
"##############  # #######.   ##########  ##  #########  #  ################\n"
"###############     ##   #. ###########  ###  ########  - -################\n"
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

void set_var(const char *name, const char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            strcpy(vars[i].value, value);
            return;
        }
    }

    if (var_count < MAX_VAR) {
        strcpy(vars[var_count].name, name);
        strcpy(vars[var_count].value, value);
        var_count++;
    }

}


char *get_var(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    return NULL;
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
    if (animation_running) {
        animation_running = 0; // On demande l'arrêt de la boucle
    }
    else if (current_child > 0) {
        write(1, "\n", 1);
    }
}

void cd(char* chemin){
    chdir(chemin);
}
void goku_path_absolute(){
    char *cwd = getcwd(NULL, 0); // getcwd alloue un buffer
    if (!cwd) {
        perror("getcwd");
        exit(1);
    }

    GOKU_PATH = malloc(strlen(cwd) + strlen("/ascii-animation.txt") + 1); // allocation pour le chemin complet
    if (!GOKU_PATH) {
        perror("malloc");
        free(cwd);
        exit(1);
    }

    strcpy(GOKU_PATH, cwd);          // copie le répertoire courant
    strcat(GOKU_PATH, "/ascii-animation.txt"); // concatène le nom du fichier

    printf("Chemin GOKU_PATH = %s\n", GOKU_PATH);

    free(cwd);
}
void goku() {
    FILE *file = fopen(GOKU_PATH, "r");
    if (!file) { printf("File not found\n"); return; }

    static char frames[FRAMES][LINES][LINE_LEN];
    memset(frames, 0, sizeof(frames));

    for (int f = 0; f < FRAMES; f++) {
        for (int l = 0; l < LINES; l++) {
            if (!fgets(frames[f][l], LINE_LEN, file)) break;
            size_t len = strlen(frames[f][l]);
            if (len > 0 && frames[f][l][len - 1] == '\n') 
                frames[f][l][len - 1] = '\0';
        }
    }
    fclose(file);

    printf("\033[?25l"); 
    animation_running = 1;
    signal(SIGINT, sigint_handler);
    for (int loop = 0; loop < 3; loop++) {
        for (int f = 0; f < FRAMES && animation_running; f++) {
            printf("\033[H\033[2J"); 

            for (int l = 0; l < LINES && animation_running; l++) {
                if (frames[f][l][0] != '\0') {
                    printf("%s\n", frames[f][l]);
                } else {
                    printf("\n");
                }
            }
            fflush(stdout);
            usleep(100000); 
        }
    }
    animation_running = 0;
    printf("\033[?25h"); 
}