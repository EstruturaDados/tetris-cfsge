#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <windows.h>

#define MAX_FILA 5
#define MAX_PILHA 3

// Peças 
typedef struct {
    char nome;  // Tipo da peça: 'I', 'O', 'T', 'L'
    int id;     // Identificador único da peça
} Peca;

// Fila circular
typedef struct {
    Peca pecas[MAX_FILA];  // Array de peças
    int frente;             // Índice do elemento da frente
    int tras;               // Índice do elemento de trás
    int tamanho;            // Quantidade atual de elementos
} FilaPecas;

// Pilha de reserva
typedef struct {
    Peca pecas[MAX_PILHA];  
    int topo;                
} PilhaReserva;

// -------------------------
//   SISTEMA DE HISTÓRICO
// -------------------------
typedef struct {
    FilaPecas filaBackup;
    PilhaReserva pilhaBackup;
} Estado;

Estado historico[20];
int topoHistorico = -1;

void salvarEstado(FilaPecas *fila, PilhaReserva *pilha) {
    if (topoHistorico < 19) {
        topoHistorico++;
        historico[topoHistorico].filaBackup = *fila;
        historico[topoHistorico].pilhaBackup = *pilha;
    }
}

int desfazer(FilaPecas *fila, PilhaReserva *pilha) {
    if (topoHistorico < 0)
        return 0;
    
    *fila = historico[topoHistorico].filaBackup;
    *pilha = historico[topoHistorico].pilhaBackup;
    topoHistorico--;
    return 1;
}

// ID global incremental
int proximoId = 0;

// Gerar peça aleatória
Peca gerarPeca() {
    Peca novaPeca;
    char tipos[] = {'I', 'O', 'T', 'L'};
    novaPeca.nome = tipos[rand() % 4];
    novaPeca.id = proximoId++;
    return novaPeca;
}

// =====================================
// FUNÇÕES DA FILA 
// =====================================

// Função para inicializar a fila
void inicializarFila(FilaPecas *fila) {
    fila->frente = 0;
    fila->tras = -1;
    fila->tamanho = 0;
}

// Função para verificar se a fila está vazia
int filaVazia(FilaPecas *fila) {
    return fila->tamanho == 0;
}

// Função para verificar se a fila está cheia
int filaCheia(FilaPecas *fila) {
    return fila->tamanho == MAX_FILA;
}

// Função para inserir uma peça na fila (enqueue)
int enqueue(FilaPecas *fila, Peca peca) {
    if (filaCheia(fila)) {
        return 0;
    }
    
    fila->tras = (fila->tras + 1) % MAX_FILA;
    fila->pecas[fila->tras] = peca;
    fila->tamanho++;
    
    return 1;
}

// Função para remover uma peça da fila (dequeue)
int dequeue(FilaPecas *fila, Peca *peca) {
    if (filaVazia(fila)) {
        return 0;
    }
    
    *peca = fila->pecas[fila->frente];
    fila->frente = (fila->frente + 1) % MAX_FILA;
    fila->tamanho--;
    
    return 1;
}

// =====================================
// FUNÇÕES DA PILHA
// =====================================

// Função para inicializar a pilha
void inicializarPilha(PilhaReserva *pilha) {
    pilha->topo = -1;
}

int pilhaVazia(PilhaReserva *pilha) {
    return pilha->topo == -1;
}

int pilhaCheia(PilhaReserva *pilha) {
    return pilha->topo == MAX_PILHA - 1;
}

int push(PilhaReserva *pilha, Peca peca) {
    if (pilhaCheia(pilha))
        return 0;

    pilha->topo++;
    pilha->pecas[pilha->topo] = peca;
    return 1;
}

int pop(PilhaReserva *pilha, Peca *peca) {
    if (pilhaVazia(pilha))
        return 0;

    *peca = pilha->pecas[pilha->topo];
    pilha->topo--;
    return 1;
}

// =====================================
// FUNÇÕES DE EXIBIÇÃO
// =====================================

void exibirFila(FilaPecas *fila) {
    printf("Fila de peças: ");
    if (filaVazia(fila)) {
        printf("[VAZIA]");
    } else {
        int i, idx = fila->frente;
        for (i = 0; i < fila->tamanho; i++) {
            printf("[%c %d] ", fila->pecas[idx].nome, fila->pecas[idx].id);
            idx = (idx + 1) % MAX_FILA;
        }
    }
    printf("\n");
}

void exibirPilha(PilhaReserva *pilha) {
    printf("Pilha de reserva (Topo -> Base): ");
    if (pilhaVazia(pilha)) {
        printf("[VAZIA]");
    } else {
        for (int i = pilha->topo; i >= 0; i--) {
            printf("[%c %d] ", pilha->pecas[i].nome, pilha->pecas[i].id);
        }
    }
    printf("\n");
}

void exibirEstado(FilaPecas *fila, PilhaReserva *pilha) {
    printf("\n---------------------------------------------\n");
    printf("              ESTADO ATUAL\n");
    printf("---------------------------------------------\n");
    exibirFila(fila);
    exibirPilha(pilha);
    printf("---------------------------------------------\n");
    printf("Fila: %d/%d | Pilha: %d/%d\n",
           fila->tamanho, MAX_FILA, pilha->topo + 1, MAX_PILHA);
    printf("---------------------------------------------\n");
}

void exibirMenu() {
    printf("\n========== Tetris Stack - Controle de peças ==========\n");
    printf("1 - Jogar peça da frente da fila\n");
    printf("2 - Reservar peça (fila → pilha)\n");
    printf("3 - Usar peça reservada (pilha → fora)\n");
    printf("4 - Trocar peça da frente da fila com topo da pilha\n");
    printf("5 - Trocar as 3 primeiras da fila com as 3 da pilha\n");
    printf("6 - Desfazer última jogada\n");
    printf("7 - Inverter fila com pilha\n");
    printf("0 - Sair\n");
    printf("-----------------------------------------------------\n");
    printf("Escolha uma opção: ");
}

// ==========================================
// AÇÕES PRINCIPAIS
// ==========================================

void jogarPeca(FilaPecas *fila) {
    Peca pecaJogada;
    if (dequeue(fila, &pecaJogada)) {
        printf("\n[SUCESSO] Peça [%c %d] foi jogada!\n", pecaJogada.nome, pecaJogada.id);

        Peca nova = gerarPeca();
        enqueue(fila, nova);
        printf("[INFO] Nova peça [%c %d] adicionada.\n", nova.nome, nova.id);
    } else {
        printf("\n[ERRO] Fila vazia!\n");
    }
}

void reservarPeca(FilaPecas *fila, PilhaReserva *pilha) {
    if (pilhaCheia(pilha)) {
        printf("\n[ERRO] Pilha cheia!\n");
        return;
    }

    Peca p;
    if (dequeue(fila, &p)) {
        push(pilha, p);
        printf("\n[SUCESSO] Peça [%c %d] reservada!\n", p.nome, p.id);

        Peca nova = gerarPeca();
        enqueue(fila, nova);
        printf("[INFO] Nova peça [%c %d] adicionada.\n", nova.nome, nova.id);
    }
}

void usarPecaReservada(PilhaReserva *pilha) {
    Peca p;
    if (pop(pilha, &p)) {
        printf("\n[SUCESSO] Peça [%c %d] usada!\n", p.nome, p.id);
    } else {
        printf("\n[ERRO] Pilha vazia!\n");
    }
}

void trocarPecaAtual(FilaPecas *fila, PilhaReserva *pilha) {

    if (filaVazia(fila)) {
        printf("\n[ERRO] Fila vazia!\n");
        return;
    }

    if (pilhaVazia(pilha)) {
        printf("\n[ERRO] Pilha vazia!\n");
        return;
    }

    Peca tmpFila = fila->pecas[fila->frente];
    Peca tmpPilha = pilha->pecas[pilha->topo];

    fila->pecas[fila->frente] = tmpPilha;
    pilha->pecas[pilha->topo] = tmpFila;

    printf("\n[SUCESSO] Troca realizada!\n");
}

void trocaMultipla(FilaPecas *fila, PilhaReserva *pilha) {

    if (fila->tamanho < 3) {
        printf("\n[ERRO] Fila precisa ter 3 peças!\n");
        return;
    }

    if (pilha->topo + 1 != 3) {
        printf("\n[ERRO] Pilha precisa ter 3 peças!\n");
        return;
    }

    Peca tempFila[3], tempPilha[3];

    int idx = fila->frente;
    for (int i = 0; i < 3; i++) {
        tempFila[i] = fila->pecas[idx];
        idx = (idx + 1) % MAX_FILA;
    }

    for (int i = 0; i < 3; i++)
        tempPilha[i] = pilha->pecas[pilha->topo - i];

    idx = fila->frente;
    for (int i = 0; i < 3; i++) {
        fila->pecas[idx] = tempPilha[i];
        idx = (idx + 1) % MAX_FILA;
    }

    for (int i = 0; i < 3; i++)
        pilha->pecas[pilha->topo - i] = tempFila[i];

    printf("\n[SUCESSO] Troca múltipla realizada!\n");
}

void inverterEstruturas(FilaPecas *fila, PilhaReserva *pilha) {

    FilaPecas filaTemp = *fila;
    inicializarFila(fila);

    for (int i = 0; i < filaTemp.tamanho; i++) {
        int idx = (filaTemp.frente + i) % MAX_FILA;
        push(pilha, filaTemp.pecas[idx]);
    }

    PilhaReserva pilhaTemp = *pilha;
    inicializarPilha(pilha);

    for (int i = pilhaTemp.topo; i >= 0; i--)
        enqueue(fila, pilhaTemp.pecas[i]);

    printf("\n[SUCESSO] Fila e Pilha invertidas!\n");
}

void popularFilaInicial(FilaPecas *fila) {
    printf("\n========== INICIALIZANDO ==========\n");

    for (int i = 0; i < MAX_FILA; i++) {
        Peca p = gerarPeca();
        enqueue(fila, p);
        printf("%d. Peça [%c %d] adicionada.\n", i + 1, p.nome, p.id);
    }
}

// ==========================================
// FUNÇÃO PRINCIPAL
// ==========================================

int main() {
    setlocale(LC_ALL, "pt_BR.UTF-8");
    srand((unsigned)time(NULL));

    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    FilaPecas fila;
    PilhaReserva pilha;

    srand(time(NULL));

    inicializarFila(&fila);
    inicializarPilha(&pilha);

    popularFilaInicial(&fila);

    int opcao;

    do {
        exibirEstado(&fila, &pilha);
        exibirMenu();
        scanf("%d", &opcao);

        switch (opcao) {

            case 1:
                salvarEstado(&fila, &pilha);
                jogarPeca(&fila);
                break;

            case 2:
                salvarEstado(&fila, &pilha);
                reservarPeca(&fila, &pilha);
                break;

            case 3:
                salvarEstado(&fila, &pilha);
                usarPecaReservada(&pilha);
                break;

            case 4:
                salvarEstado(&fila, &pilha);
                trocarPecaAtual(&fila, &pilha);
                break;

            case 5:
                salvarEstado(&fila, &pilha);
                trocaMultipla(&fila, &pilha);
                break;

            case 6:
                if (desfazer(&fila, &pilha))
                    printf("\n[INFO] Ação desfeita com sucesso!\n");
                else
                    printf("\n[ERRO] Não há ações para desfazer.\n");
                break;

            case 7:
                salvarEstado(&fila, &pilha);
                inverterEstruturas(&fila, &pilha);
                break;

            case 0:
                printf("\nFinalizando...\n");
                break;

            default:
                printf("\n[ERRO] Opção inválida!\n");
                break;
        }

    } while (opcao != 0);

    return 0;
}
