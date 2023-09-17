// Escreva um programa C/C++ usando sockets para simular um sorteio da seguinte forma. Existem um processo pai e seus respectivos dez processos filhos. O processo pai aguarda por conexões TCP em uma porta arbitrária (você escolhe alguma porta a partir de 1024). Cada processo filho se conecta à porta do pai, informa seu PID e aguarda por uma mensagem informando o PID sorteado. Após receber todos os PIDs, o processo pai sorteia um PID entre aqueles informados, imprime na tela "PID sorteado: <pid>" , onde <pid> é o PID sorteado, e informa o PID sorteado aos fillhos através da conexão TCP anteriormente estabelecida. Os filhos recebem o PID sorteado e encerram a conexão TCP. Adicionalmente, o filho sorteado imprime na tela "<pid>: fui sorteado" onde pid é seu PID.

// Instruções:

// a) use o comando fork para criar processos filhos.

// b) use processos ao invés de threads

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>

using namespace std;

#define PORT 8080
#define QTD_JOGADORES 10

void loteria(int pid)
{
    cout << "Eu, PID " << pid << ", sou a loteria" << endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Erro ao criar socket no servidor");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Erro ao fazer bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, QTD_JOGADORES) < 0)
    {
        perror("Erro ao fazer listen");
        exit(EXIT_FAILURE);
    }

    int lista_sockets_jogadores[QTD_JOGADORES] = {0};
    int lista_pids_jogadores[QTD_JOGADORES] = {0};

    int index = 0;
    int new_socket;
    int addrlen = sizeof(serv_addr);
    while (index < (QTD_JOGADORES))
    {
        new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t *)&addrlen);
        if (new_socket >= 0)
        {
            // cout << "Jogador " << index << " conectado" << endl;
            lista_sockets_jogadores[index] = new_socket;
            index++;
            char buffer[1024] = {0};
            int val_read = read(new_socket, buffer, 1024);
            // printf("msg lida no serv: %s\n", buffer);
            lista_pids_jogadores[index] = atoi(buffer);
        }
        else
        {
            perror("Erro em accept");
        }
    }
    cout << "todo mundo se conectou!! Sorteando.." << endl;
    sleep(1); // só pelo suspense kkk
    int sorteado = rand() % QTD_JOGADORES;

    cout << "O sorteado foi: " << lista_pids_jogadores[sorteado] << "!!! Avisando aos jogadores..." << endl;

    for (int i = 0; i < QTD_JOGADORES; i++)
    {

        char *str_sorteado = (char *)malloc(sizeof(char) * 1024);
        sprintf(str_sorteado, "%d", lista_pids_jogadores[sorteado]);

        send(lista_sockets_jogadores[i], str_sorteado, strlen(str_sorteado), 0);

        // fecha o socket
        close(lista_sockets_jogadores[i]);
    }

    // desliga o servidor
    shutdown(server_fd, SHUT_RDWR);
    return;
}

void jogador(int pid)
{
    cout << "Eu, PID " << pid << ", sou um jogador comprando um ticket da loteria" << endl;
    sleep(1);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Erro ao converter IP");
        exit(EXIT_FAILURE);
    }

    int status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        perror("Erro ao conectar");
        exit(EXIT_FAILURE);
    }

    char *str_pid = (char *)malloc(sizeof(char) * 1024);
    sprintf(str_pid, "%d", pid);

    send(client_fd, str_pid, strlen(str_pid), 0);

    char buffer[1024] = {0};
    int val_read = read(client_fd, buffer, 1024);

    // verifica se o pid sorteado é o dele
    if (atoi(buffer) == atoi(str_pid))
    {
        cout << "Eu, PID " << pid << ", fui sorteado!!!! 😎😎💰🎉" << endl;
    }
    else
    {
        cout << "Eu, PID " << pid << ", perdi... tristeza ;-; 😢😭" << endl;
    }

    // fecha o socket
    close(client_fd);
    return;
}

int main()
{

    pid_t pid_atual = fork();

    if (pid_atual > 0)
    {
        // cout << "eu ainda sou o pai" << endl;
        loteria(getpid());
    }
    else
    {

        for (int i = 0; i < QTD_JOGADORES; i++) // Começa de 1 porque o primeiro filho já foi criado
        {
            pid_atual = fork();
            if (pid_atual == 0) // Processo filho
            {
                // cout << "pai" << endl;
                jogador(getpid());
                exit(0); // Encerra o processo filho após ele ter terminado
            }
            else
            {
                // cout << "filho" << endl;
                sleep(1);
            }
        }
    }

    // delay de 1 segundo
    sleep(1);
    return 0;
}