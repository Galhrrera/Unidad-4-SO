#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "eventList.h"
#define PERMS 0600 //Permisos de la cola de mensjaes -- hay que ver si se quedan o no
#define PORT 6666
#define BUF_SIZE 128

 struct client_t{
     int socket;
     int rxState;
 };

struct msg
{
    long msg_type;
    char msg_text[200];
};
struct msg msg_s;
struct msg msg_r;
key_t queue_key;
int queue_id;
int msg_len;
int exit_var = 1;
int l_clients[50];
int client_count = 0;
EventList *eventList = NULL;
int event_count = 0;
void *escritura(void *params)
{
    while (exit_var == 1)
    {
        char strTmp[200];
        if(fgets(strTmp, sizeof(strTmp), stdin) != NULL)
        {
            msg_len = strlen(strTmp);
            if(strTmp[msg_len-1] == '\n')
            {
                strTmp[msg_len-1] = '\0';
            }
            if(strcmp(strTmp, "exit") == 0)
            {
                strcpy(msg_s.msg_text, strTmp);
                for(int i = 0; i < client_count; i++)
                {
                    msg_s.msg_type = l_clients[i];
                    if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                    {
                        perror("Error enviando mensaje desde el servidor");
                    }
                }                
                exit_var = 0;
                break;
            }
            else
            {
                char *strTmp2 = strdup(strTmp);
                char *cmd_token = strtok(strTmp2, " ");
                char *arg_token = strdup(cmd_token);
                arg_token = strtok(NULL, " ");
                if(strcmp(cmd_token, "add") == 0)
                {
                    if(SearchEvent(eventList, strdup(arg_token)) == NULL)
                    {
                        event_count++;
                        AddEvent(eventList, CreateEvent(strdup(arg_token)));
                        printf("    | SERVIDOR: | comando add() >>\n     \"%s\" Creado, Eventos totales: %d\n", eventList->last->eventName, event_count);
                    }
                    else
                    {
                        printf("    | SERVIDOR: | comando add() >>\n     Error, nombre de evento inválido\n");
                    }
                }
                else if(strcmp(cmd_token, "remove") == 0)
                {
                    if(SearchEvent(eventList, strdup(arg_token)) == NULL)
                    {
                        printf("    | SERVIDOR: | comando remove() >>\n     Error, nombre de evento inválido\n");
                    }
                    else
                    {
                        event_count--;
                        RemoveEvent(eventList, strdup(arg_token));
                        printf("    | SERVIDOR: | comando remove() >>\n     Event \"%s\" Eliminado, Eventos totales: %d\n",arg_token, event_count);
                    }
                }
                else if(strcmp(cmd_token, "trigger") == 0)
                {
                    if(SearchEvent(eventList, strdup(arg_token)) == NULL)
                    {
                        printf("    | SERVIDOR: | comando trigger() >>\n     Error, nombre de evento inválido\n");
                    }
                    else
                    {
                        Event *tmpEventTrg = SearchEvent(eventList, strdup(arg_token));
                        if(tmpEventTrg->count_subdClients == 0)
                        {
                            printf("    | SERVIDOR: | comando trigger() >>\n     No hay clientes suscritos a este evento\n");
                        }
                        else
                        {
                            sprintf(msg_s.msg_text, "Evento '%s' ha sido lanzado", arg_token);
                            msg_len = strlen(msg_s.msg_text);
                            for (int i = 0; i < tmpEventTrg->count_subdClients; i++)
                            {
                                msg_s.msg_type = tmpEventTrg->l_subdClients[i];
                                if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                                {
                                    perror("Error enviando mensaje desde el servidor");
                                }
                            }
                        }         
                    }
                }
                else if(strcmp(cmd_token, "list") == 0)
                {
                    Event *tmpEventL = SearchEvent(eventList, strdup(arg_token));
                    printf("    | SERVIDOR: | comando list() >>\n     Clientes suscritos: ");
                    printf("\n");
                    for (int i = 0; i < tmpEventL->count_subdClients; i++)
                    {
                        printf("%d\n ", tmpEventL->l_subdClients[i]);
                    }
                    printf("\n");
                }
                else
                {
                    printf("    | SERVIDOR: | error >>\n     Ingrese un comando válido\n");
                }
            }
        }
        else
        {
            exit_var = 0;
        }
    }
    return NULL;
}
void *lectura(void *params) 
{
    eventList = CreateEventList();
    while (exit_var == 1)
    {
        if(msgrcv(queue_id, &msg_r, sizeof(msg_r.msg_text), -5, 0) == -1)
        {
            perror("Error recibiendo mensaje del cliente");
        }
        char *str_tmpMsg = strdup(msg_r.msg_text);
        char *initoken = strtok(str_tmpMsg, ";");
        char *tmpPidc = strdup(initoken);
        tmpPidc = strtok(NULL, ";");
        printf("    | CLIENTE | (#%s): \"%s\"\n", tmpPidc, initoken);
        if(msg_r.msg_type == 1)
        {
            int pid_client;
            char *str_pid_client = strdup(msg_r.msg_text);
            char *token = strtok(str_pid_client, ";");
            token = strtok(NULL, ";");
            pid_client = atoi(token);
            l_clients[client_count] = pid_client;
            client_count++;
        }
        else if(msg_r.msg_type == 2)
        {
            char *strTmp2 = strdup(msg_r.msg_text);
            char *eventTmp2 = strtok(strTmp2, " ");
            eventTmp2 = strtok(NULL, ";");
            int pidTmp2;
            char *tkpid = strdup(eventTmp2);
            tkpid = strtok(NULL, ";");
            pidTmp2 = atoi(tkpid);
            Event *fndEvent = SearchEvent(eventList, strdup(eventTmp2));
            fndEvent->l_subdClients[fndEvent->count_subdClients] = pidTmp2;
            fndEvent->count_subdClients++;
        }
        else if(msg_r.msg_type == 3)
        {
            char *strTmp3 = strdup(msg_r.msg_text);
            char *eventTmp3 = strtok(strTmp3, " ");
            eventTmp3 = strtok(NULL, ";");
            int pidTmp3;
            char *tkpid3 = strdup(eventTmp3);
            tkpid3 = strtok(NULL, ";");
            pidTmp3 = atoi(tkpid3);
            Event *fndEvent = SearchEvent(eventList, strdup(eventTmp3));
            for (int i = 0; i < fndEvent->count_subdClients; i++){
                if(fndEvent->l_subdClients[i] == pidTmp3){
                    for(int j = i; j < fndEvent->count_subdClients; j++){
                        if(j == fndEvent->count_subdClients-1){
                            fndEvent->l_subdClients[i] = 0;
                        }
                        else{
                            fndEvent->l_subdClients[i] = fndEvent->l_subdClients[i+1];
                        }
                    }
                    fndEvent->count_subdClients--;
                    break;
                }
            }
        }
        else if(msg_r.msg_type == 4){
            int pid_client;
            char *str_pid_client = strdup(msg_r.msg_text);
            char *token = strtok(str_pid_client, ";");
            token = strtok(NULL, ";");
            pid_client = atoi(token);
            msg_s.msg_type = pid_client;
            if(event_count > 0)
            {
                Event *tmpEvent = eventList->head;
                char str_events[200];
                sprintf(str_events, "Server events: ");
                do
                {
                    strcat(str_events, "'");
                    strcat(str_events, strdup(tmpEvent->eventName));
                    strcat(str_events, "' ");
                    tmpEvent = tmpEvent->next;
                } while (tmpEvent != NULL);
                sprintf(msg_s.msg_text, "%s",strdup(str_events));
                msg_len = strlen(msg_s.msg_text);
                if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                {
                    perror("Error enviando la respuesta a 'ask' desde el servidor al cliente");
                }
            }
            else
            {
                sprintf(msg_s.msg_text, "No hay eventos creados");
                msg_len = strlen(msg_s.msg_text);
                if(msgsnd(queue_id, &msg_s, msg_len+1, 0) == -1)
                {
                    perror("Error enviando la respuesta a 'ask' desde el servidor al cliente");
                }
            }
        }
        else if(msg_r.msg_type == 5)
        {
            printf("SERVIDOR FINALIZANDO...\n");
        }
    }
    return NULL;
}
int main()
{
    system("sudo touch queue.txt");
    if((queue_key = ftok("queue.txt", 'C')) == -1)
    {
        perror("Error creando la key de la cola\n");
        exit(EXIT_FAILURE);
    }
    if((queue_id = msgget(queue_key, PERMS | IPC_CREAT)) == -1)
    {
        perror("Error la cola de mensaje");
        exit(EXIT_FAILURE);
    }

    /**
     * @brief 
     * Antes de crear el servidor, de que este listo, necesitamos crear los sockets
     */



    /**
     * 
     */
    printf("SERVIDOR LISTO...\n");
    pthread_t hiloEscritura;
    pthread_t hiloLectura;
    pthread_create(&hiloEscritura, NULL, &escritura, NULL);
    pthread_create(&hiloLectura, NULL, &lectura, NULL);
    pthread_join(hiloEscritura, NULL);
    pthread_join(hiloLectura, NULL);
    if((msgctl(queue_id, IPC_RMID, NULL)) == -1)
    {
        perror("Error eliminando la cola de mensajes\n");
    }
    system("rm queue.txt");
    DestroyEventList(eventList);
    printf("SERVER FINALIZADO...\n");
    exit(EXIT_SUCCESS);
}