#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_FILENAME 256
#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

void gerar_string_aleatoria(char *dest, int tamanho) {
    int charset_len = strlen(CHARSET);
    for (int i = 0; i < tamanho; ++i) {
        dest[i] = CHARSET[rand() % charset_len];
    }
    dest[tamanho] = '\0';
}

void gerar_substring(char *origem, char *dest) {
    int len = strlen(origem);
    int start = rand() % len;
    int end = start + 1 + rand() % (len - start);
    strncpy(dest, origem + start, end - start);
    dest[end - start] = '\0';
}

int main(int argc, char *argv[]) {
    int base_string_size, step, num_testes;
    char *diretorio = ".";
    int opt;

    // Parse de argumentos opcionais (-d)
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
            case 'd':
                diretorio = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s [-d diretorio] base_string_size step num_testes\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (argc - optind < 3) {
        fprintf(stderr, "Uso: %s [-d diretorio] base_string_size step num_testes\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    base_string_size = atoi(argv[optind]);
    step = atoi(argv[optind + 1]);
    num_testes = atoi(argv[optind + 2]);

    srand(time(NULL));

    char *string_a = malloc(base_string_size * num_testes * step + 1);
    char *string_b = malloc(base_string_size * num_testes * step + 1);

    if (!string_a || !string_b) {
        fprintf(stderr, "Erro de alocação de memória.\n");
        exit(EXIT_FAILURE);
    }

    gerar_string_aleatoria(string_a, base_string_size);
    gerar_substring(string_a, string_b);

    char filename[MAX_FILENAME];
    snprintf(filename, MAX_FILENAME, "%s/test%d.in", diretorio, base_string_size);
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "%s\n%s\n", string_a, string_b);
    fclose(f);
    printf("Teste 1 gerado com sucesso!\n");

    for (int i = 1; i < num_testes; ++i) {
        int test_size = base_string_size + (i * step);
        strcat(string_a, string_a);
        strcat(string_b, string_b);

        snprintf(filename, MAX_FILENAME, "%s/test%d.in", diretorio, test_size);
        f = fopen(filename, "w");
        if (!f) {
            perror("Erro ao abrir arquivo");
            exit(EXIT_FAILURE);
        }
        fprintf(f, "%s\n%s\n", string_a, string_b);
        fclose(f);
        printf("Teste %d gerado com sucesso!\n", i + 1);
    }

    printf("Todos os testes foram gerados com sucesso!\n");

    free(string_a);
    free(string_b);

    return 0;
}
