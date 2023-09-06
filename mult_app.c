/* simple_mutex_app.c
 *
 * Copyright (C) 2017 Alexandre Luiz Brisighello Filho
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

////////////////////////////////////////////////////////

static struct timespec ts_start, ts_end;
static struct timeval tv_start, tv_end;
static clock_t c_start;

void mystopwatch_start()
{
    gettimeofday(&tv_start, NULL);
    c_start = clock();
}

void mystopwatch_stop()
{
    gettimeofday(&tv_end, NULL);

    double wall = (1000000.0 * (double)(tv_end.tv_sec - tv_start.tv_sec));
    wall += ((double)(tv_end.tv_usec - tv_start.tv_usec));

    double cpu = 1000000 * (double)(clock() - c_start) / CLOCKS_PER_SEC;

    printf("@WALL: %lf\n", wall);
    printf("@CPU: %lf\n", cpu);
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

pthread_mutex_t mutex;
int g_j = 0;
int np_general, np_spec;
int num_threads = 3;
int general_size_array = 9;
int init_array_a[9][9] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
};
int init_array_b[9][9] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
};
int final_array[9][9] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
};

void *do_mult(int m)
{
    m = np_spec * num_threads + m;
    int t = 0;
    for (int k = 0; k < general_size_array; k++)
    {
        t = t + init_array_a[m][k] * init_array_b[k][g_j];
    }
    pthread_mutex_lock(&mutex);
    final_array[m][g_j] = t;
    pthread_mutex_unlock(&mutex);
    // delay(100);
    return NULL;
}

void *do_mult_final(int m)
{
    // m = np_spec * num_threads + m;
    int t = 0;
    for (int k = 0; k < general_size_array; k++)
    {
        t = t + init_array_a[m][k] * init_array_b[k][g_j];
    }
    pthread_mutex_lock(&mutex);
    final_array[m][g_j] = t;
    pthread_mutex_unlock(&mutex);
    // delay(100);
    return NULL;
}

main(int argc, char **argv)
{
    mystopwatch_start();
    int *x;

    if (pthread_mutex_init(&mutex, NULL))
    {
        fprintf(stderr, "error initializing mutex");
        return 3;
    }

    if (argc > 1)
    {
        num_threads = atoi(argv[1]);
    }

    pthread_t *do_mlut_thread;
    do_mlut_thread = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    x = (int *)malloc(num_threads * sizeof(int));
    np_general = general_size_array / num_threads;

    for (int j = 0; j < general_size_array; j++)
    {
        for (int d = 0; d < np_general; d++)
        {
            int t = 0;
            g_j = j;
            np_spec = d;
            for (int m = 0; m < num_threads; m++)
            {
                if (pthread_create(&do_mlut_thread[m], NULL, do_mult, m))
                {
                    fprintf(stderr, "Error creating thread\n");
                    return 1;
                }
            }

            for (int m = 0; m < num_threads; m++)
            {
                if (pthread_join(do_mlut_thread[m], NULL))
                {
                    fprintf(stderr, "Error joining thread\n");
                    return 2;
                }
            }
            // free(do_mlut_thread);
        }
        for (int q = 0; q < general_size_array - (np_general * num_threads); q++)
        {

            int t = 0;
            g_j = j;
            np_spec = (np_general * num_threads) + q;
            pthread_t *do_mult_final_thread;
            do_mult_final_thread = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
            for (int m = np_spec; m < general_size_array; m++)
            {
                if (pthread_create(&do_mult_final_thread[m], NULL, do_mult_final, m))
                {
                    fprintf(stderr, "Error creating thread\n");
                    return 1;
                }
            }

             for (int m = np_spec; m < general_size_array; m++)
            {
                if (pthread_join(do_mult_final_thread[m], NULL))
                {
                    fprintf(stderr, "Error joining thread\n");
                    return 2;
                }
            }
        }
    }

    pthread_mutex_destroy(&mutex);
    for (int n = 0; n < general_size_array; n++)
    {
        for (int m = 0; m < general_size_array; m++)
        {
            printf("%d ", final_array[n][m]);
        }
        printf("\n");
    }
    free(x);
    // free(do_mlut_thread);
    mystopwatch_stop();
    return 0;
}
