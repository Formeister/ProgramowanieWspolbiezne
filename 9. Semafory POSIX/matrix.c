#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>


typedef struct matrix_struct {
    int dimension;
    double **items;
    double determinant;
} Matrix;

//Pobieranie macierzy z klawiatury
Matrix *get_matrix() {
    Matrix *matrix;
    int i, j;
    matrix = malloc(sizeof (Matrix));
    printf("Podaj wymiar macierzy:\n");
    scanf("%d", &matrix->dimension);
    printf("Podaj zawartość macierzy:\n");
    matrix->items = malloc(matrix->dimension * sizeof (*matrix->items));
    for (i = 0; i < matrix->dimension; i++) {
        matrix->items[i] = malloc(matrix->dimension * sizeof (**matrix->items));
	printf("\nWIERSZ #%d:\n", i+1);
        for (j = 0; j < matrix->dimension; j++) {
	printf("Wartość #%d:\n", j+1);
            scanf("%lf", &matrix->items[i][j]);
        }
    }
    return matrix;
}

//Wyswietlanie macierzy
void show_matrix(Matrix *matrix) {
    int i, j;
    for (i = 0; i < matrix->dimension; i++) {
        for (j = 0; j < matrix->dimension; j++) {
            printf("%.1lf ", matrix->items[i][j]);
        }
        printf("\n");
    }
}

//Zwracanie wyznacznika macierzy 2x2
double determinant2d(Matrix *A) {
    return A->items[0][0] * A->items[1][1] - A->items[1][0] * A->items[0][1];
}

//Zwracanie wyznacznika podanej macierzy o wymiarze 3x3 (metoda Sarrusa)
double determinant_sarrus(Matrix *A) {
    int i;
    double determinant = 0;
    for (i = 0; i < 3; i++) {
        determinant += A->items[0][i % 3] * A->items[1][(i + 1) % 3] * A->items[2][(i + 2) % 3];
    }
    for (i = 0; i < 3; i++) {
        determinant -= A->items[2][i % 3] * A->items[1][(i + 1) % 3] * A->items[0][(i + 2) % 3];
    }
    return determinant;
}


//Przycinanie macierzy, minor
Matrix *get_minor(Matrix *matrix, int i, int j) {
    Matrix *cut;
    int i_cut, j_cut, i_matrix, j_matrix;
    cut = malloc(sizeof (Matrix));
    cut->dimension = matrix->dimension - 1;
    cut->items = malloc(cut->dimension * sizeof (*cut->items));
    for (i_cut = 0; i_cut < cut->dimension; i_cut++) {
        cut->items[i_cut] = malloc(cut->dimension * sizeof (**cut->items));
        i_matrix = i_cut < i ? i_cut : i_cut + 1;
        for (j_cut = 0; j_cut < cut->dimension; j_cut++) {
            j_matrix = j_cut < j ? j_cut : j_cut + 1;
            cut->items[i_cut][j_cut] = matrix->items[i_matrix][j_matrix];
        }
    }
    return cut;
}

//Obliczanie wyznacznika
void *determinant_thread(void *arg) {
    Matrix *matrix;
    int k;
    int result;
    matrix = (Matrix *)arg;
    if (matrix->dimension == 1) {
        matrix->determinant = matrix->items[0][0];
    } else if (matrix->dimension == 2) {
        matrix->determinant = determinant2d(matrix);
    } else if (matrix->dimension == 3) {
        matrix->determinant = determinant_sarrus(matrix);
    } else {
        //Obcinanie, zmniejszanie macierzy, minor
        //Laplace'a według pierwszego wiersza
        Matrix *minors[matrix->dimension];
        for (k = 0; k < matrix->dimension; k++) {
            minors[k] = get_minor(matrix, 0, k);
        }

	//Rekurencyjne obliczanie wyznaczników minorów
        pthread_t threads[matrix->dimension];
        for (k = 0; k < matrix->dimension; k++) {
            result = pthread_create(
                    &threads[k], NULL, determinant_thread, (void *)minors[k]);
            if (result != 0) {
                perror("Błąd podczas tworzenia wątku");
                exit(EXIT_FAILURE);
            }
        }
        for (k = 0; k < matrix->dimension; k++) {
            void *thread_result;
            result = pthread_join(threads[k], &thread_result);
            if (result != 0) {
                perror("Błąd podczas łączenia wątków.");
                exit(EXIT_FAILURE);
            }
        }

        //Obliczanie wyznacznika z rozwinięcia Laplace'a
        matrix->determinant = 0;
        for (k = 0; k < matrix->dimension; k++) {
            double foo = pow(-1, 0 + k) * minors[k]->determinant;
            //Rozwinięcie według pierwszego wiersza
            matrix->determinant += matrix->items[0][k] * foo;
        }
        //Zwalnianie pamięci
        for (k = 0; k < matrix->dimension; k++) {
            free(minors[k]);
        }
    }
    return NULL;
}


void set_determinant(Matrix *matrix) {
    determinant_thread(matrix);
}

int main() {
    Matrix *matrix;

    matrix = get_matrix();
    printf("\n");
    printf("Wprowadzona macierz:\n");
    show_matrix(matrix);
    printf("\n");
    set_determinant(matrix);
    printf("Wyznacznik macierzy jest równy %.1lf\n", matrix->determinant);

    return EXIT_SUCCESS;
}
