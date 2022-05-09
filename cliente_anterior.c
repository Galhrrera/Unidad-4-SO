#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>
#define PERMS 0600

struct msg msg_s;
struct msg msg_r;
key_t queue_key;
int queue_id;
int msg_len;
int exit_var = 1;
char subed_events[200][20];
int count_subed_events = 0;

struct msg
{
    long msg_type;
    char msg_text[200];
};

void *lectura(void *params)
{
    int toexit;
    while(exit_var == 1)
    {
        if(msgrcv(queue_id, &msg_r, sizeof(msg_r.msg_text), getpid(), 0) == -1)
        {
            perror("Error al recibir los mensajes en cliente");
        }
        printf("    | SERVIDOR |>> \"%s\"\n", msg_r.msg_text);
        toexit = strcmp(msg_r.msg_text, "exit");       
        if(toexit == 0)
        {
            exit_var = 0;
        }
    }
    return NULL;
}

void *escritura(void *params)
{
    while(exit_var == 1)
    {
        char strTmp[200];
        if(fgets(strTmp, sizeof(strTmp), stdin) != NULL)
        {
            msg_len = strlen(strTmp);
            if(strTmp[msg_len-1] == '\n')
            {
                strTmp[msg_len-1] = '\0';
            }

            if(strcmp(strTmp, "list") == 0)
            {
                if(count_subed_events == 0)
                {
                    printf("    | CLIENTE | comando list() >>\n     El cliente tiene 0 suscripciones\n");
                }
                else
                {
                    printf("    | CLIENTE | comando list() >>\n     Suscrito a los eventos: ");
                    printf("\n");
                    for (int i = 0; i < count_subed_events; i++)
                    {
                        printf("\"%s\"\n ",subed_events[i]);
                    }
                    printf("\n");
                }
            }
            else if(strcmp(strTmp, "ask") == 0)
            {
                char * strTmp2 = strdup(strTmp);
                char pid_string[10];
                sprintf(pid_string, ";%d", getpid());
                strcpy(msg_s.msg_text, strcat(strTmp2, pid_string));
                msg_s.msg_type = 4;
                msg_len = strlen(msg_s.msg_text);
                if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                {
                    perror("Error enviando mensaje de 'ask' desde el cliente hacia el servidor");
                }
            }
            else
            {
                char pid_string[10];
                sprintf(pid_string, ";%d", getpid());
                char *strTmp2 = strdup(strTmp);
                char *cmd_token = strtok(strdup(strTmp2), " ");
                char *strArg = strdup(cmd_token);
                strArg = strtok(NULL, ";");
                if(strcmp(cmd_token, "sub") == 0)
                {
                    strcpy(msg_s.msg_text, strcat(strTmp2, pid_string));
                    msg_s.msg_type = 2;
                    msg_len = strlen(msg_s.msg_text);
                    if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                    {
                        perror("Error enviando mensaje de 'sub' desde el cliente al servidor");
                    }
                    sprintf(subed_events[count_subed_events], "%s", strArg);
                    count_subed_events++;
                }
                else if(strcmp(cmd_token, "unsub") == 0)
                {
                    strcpy(msg_s.msg_text, strcat(strTmp2, pid_string));
                    msg_s.msg_type = 3;
                    msg_len = strlen(msg_s.msg_text);
                    if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                    {   
                        perror("Error enviando mensaje de 'unsub' desde el cliente al servidor");
                    }
                    for (int i = 0; i < count_subed_events; i++)
                    {
                        if(strcmp(subed_events[i], strArg) == 0)
                        {
                            for(int j = i; j < count_subed_events; j++)
                            {
                                if(j == count_subed_events-1)
                                {
                                    sprintf(subed_events[j], " ");
                                }
                                else
                                {
                                    sprintf(subed_events[j], "%s", strdup(subed_events[j+1]));
                                }
                            }
                            count_subed_events--;
                            break;
                        }
                    }
                }
                else
                {
                    printf("Comando inválido\n");
                }
            }   
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[])
{
    if((queue_key = ftok("queue.txt", 'C')) == -1)
    {
        perror("Error creando key de la cola\n");
        exit(EXIT_FAILURE);
    }

    if((queue_id = msgget(queue_key, PERMS)) == -1)
    {
        perror("Error creando / conectando a la cola de mensajes\n");
        exit(EXIT_FAILURE);
    }

    msg_s.msg_type = 1; 
    char initial_str[20];
    sprintf(initial_str, "Cliente (%d) creado", getpid());
    char pid_string[10];
    sprintf(pid_string, ";%d", getpid());
    strcpy(msg_s.msg_text, strcat(initial_str, pid_string));
    msg_len = strlen(msg_s.msg_text);
    if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
    {
        perror("Error enviando mensaje inicial desde el cliente al servidor");
        exit(EXIT_FAILURE);
    }

    printf("CLIENTE #%d LISTO\n", getpid());

    pthread_t hiloLectura;
    pthread_t hiloEscritura;

    pthread_create(&hiloLectura, NULL, &lectura, NULL);
    pthread_create(&hiloEscritura, NULL, &escritura, NULL);

    pthread_join(hiloLectura, NULL);

    
    msg_s.msg_type = 5;
    sprintf(msg_s.msg_text,"exit2;%d", getpid());
    msg_len = strlen(msg_s.msg_text);
    if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
    {
        perror("Error enviando mensaje de terminación desde el cliente al servidor");
        exit(EXIT_FAILURE);
    }

    printf("CLIENT FINALIZADO\n");
    
    exit(EXIT_SUCCESS);
}