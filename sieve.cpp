#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>

typedef unsigned long long ull;

typedef struct {
    int counter_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_var_;
} barrier_t;

bool *not_prime;
int num_threads;
barrier_t barrier;

struct thread_data {
    ull start;
    ull end;
};

void barrier_init(barrier_t *barrier_) {
    barrier_->counter_ = 0;
    pthread_mutex_init(&(barrier_->mutex_), NULL);
    pthread_cond_init(&(barrier_->cond_var_), NULL);
}

void barrier_(barrier_t *barrier_) {
    pthread_mutex_lock(&(barrier_->mutex_));
    barrier_->counter_++;

    if (barrier_->counter_ == num_threads) {
        barrier_->counter_ = 0;
        pthread_cond_broadcast(&(barrier_->cond_var_));
    } else {
        while (pthread_cond_wait(&(barrier_->cond_var_), &(barrier_->mutex_)));
    }

    pthread_mutex_unlock(&(barrier_->mutex_));
}

void barrier_destroy(barrier_t *barrier_) {
    pthread_mutex_destroy(&(barrier_->mutex_));
    pthread_cond_destroy(&(barrier_->cond_var_));
}

void *do_sieve(void *arg) {
    struct thread_data *thread_info;
    thread_info = (struct thread_data *) arg;
    ull start = thread_info->start;
    ull end = thread_info->end;

    ull prime = 2;
    ull index = 0;

    while (prime * prime <= end) {
        if (prime * prime < start) {
            index = start + prime - start % prime - 1;
        } else {
            index = prime * prime - 1;
        }

        while (index <= end) {
            not_prime[index] = 1;
            index += prime;
        }
        index = prime;

        barrier_(&barrier);
        while (not_prime[index]) {
            ++index;
        }
        prime = index + 1;
    }

    pthread_mutex_lock(&barrier.mutex_);
    num_threads--;
    if (barrier.counter_ == num_threads) {
        barrier.counter_ = 0;
        pthread_cond_broadcast(&(barrier.cond_var_));
    }
    pthread_mutex_unlock(&barrier.mutex_);
    pthread_exit(NULL);
}


int main(int argc, char **argv) {
    if (argc == 1) {
        perror("Not enough arguments");
        exit(1);
    }

    int threads_number = atoi(argv[1]);
    ull limit = (argc > 2) ? atoi(argv[2]) : ULLONG_MAX;

    not_prime = (bool *) malloc(sizeof(bool) * limit);
    for (ull i = 0; i < limit; i++) {
        not_prime[i] = 0;
    }

    pthread_attr_t attr;
    barrier_init(&barrier);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    struct thread_data *threads_arg;
    threads_arg = (struct thread_data *) malloc(sizeof(struct thread_data) * threads_number);
    pthread_t *thread_id;
    thread_id = (pthread_t *) malloc(sizeof(pthread_t) * threads_number);

    ull numbers_per_thread = limit / threads_number;
    ull rest = limit % threads_number;
    ull prime = 1;
    num_threads = threads_number;
    for (size_t i = 0; i < threads_number; ++i) {
        threads_arg[i].start = prime;
        if (i < rest) {
            prime = prime + numbers_per_thread + 1;
        } else {
            prime = prime + numbers_per_thread;
        }
        threads_arg[i].end = prime - 1;
        pthread_create(&thread_id[i], &attr, do_sieve, (void *) &threads_arg[i]);
    }

    for (size_t i = 0; i < threads_number; i++) {
        pthread_join(thread_id[i], NULL);
    }

    for (ull i = 1; i < limit; ++i) {
        if (!not_prime[i]) {
            printf("%llu\n", i + 1);
        }
    }

    free(not_prime);
    pthread_attr_destroy(&attr);
    barrier_destroy(&barrier);
    pthread_exit(NULL);
}