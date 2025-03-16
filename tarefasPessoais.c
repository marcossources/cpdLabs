#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 50

typedef struct Tarefa {
    int prioridade;
    char id[MAX_ID_LEN];
    struct Tarefa* prox;
} Tarefa;

Tarefa* inicio = NULL;

// Função para criar uma nova tarefa
Tarefa* criar_tarefa(int prioridade, char* id) {
    Tarefa* nova = (Tarefa*)malloc(sizeof(Tarefa));
    if (!nova) {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }
    nova->prioridade = prioridade;
    strcpy(nova->id, id);
    nova->prox = NULL;
    return nova;
}

// Insere uma tarefa na lista de forma ordenada por prioridade e tempo de criação
void adicionar_tarefa(int prioridade, char* id) {
    Tarefa* nova = criar_tarefa(prioridade, id);
    if (!inicio || prioridade > inicio->prioridade) {
        nova->prox = inicio;
        inicio = nova;
        return;
    }

    Tarefa* atual = inicio;
    while (atual->prox && atual->prox->prioridade >= prioridade)
        atual = atual->prox;

    nova->prox = atual->prox;
    atual->prox = nova;
}

// Lista tarefas com prioridade >= prioridade_min dada
void listar_tarefas(int prioridade_min) {
    Tarefa* atual = inicio;
    while (atual) {
        if (atual->prioridade >= prioridade_min)
            printf("Prioridade: %d, ID: %s\n", atual->prioridade, atual->id);
        atual = atual->prox;
    }
}

// Remove uma tarefa pelo ID
void completar_tarefa(char* id) {
    if (!inicio) {
        printf("TAREFA INEXISTENTE\n");
        return;
    }

    Tarefa* atual = inicio;
    Tarefa* anterior = NULL;

    while (atual && strcmp(atual->id, id) != 0) {
        anterior = atual;
        atual = atual->prox;
    }

    if (!atual) {
        printf("TAREFA INEXISTENTE\n");
        return;
    }

    if (!anterior) 
        inicio = inicio->prox;  // Remove a primeira tarefa
    else 
        anterior->prox = atual->prox;

    free(atual);
}

// Processa comandos do usuário
void processar_comando(char* comando) {
    char acao[10], id[MAX_ID_LEN];
    int prioridade;
    
    if (sscanf(comando, "new %d %s", &prioridade, id) == 2) {
        if (prioridade < 0 || prioridade > 5) {
            printf("Prioridade inválida!\n");
            return;
        }
        adicionar_tarefa(prioridade, id);
    }
    else if (sscanf(comando, "list %d", &prioridade) == 1) {
        listar_tarefas(prioridade);
    }
    else if (sscanf(comando, "complete %s", id) == 1) {
        completar_tarefa(id);
    }
    else {
        printf("Comando inválido!\n");
    }
}

int main() {
    char comando[100];

    while (1) {
        printf("$ ");
        if (!fgets(comando, sizeof(comando), stdin)) 
            break;
        processar_comando(comando);
    }

    return 0;
}
