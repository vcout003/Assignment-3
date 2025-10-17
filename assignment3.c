#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Shared variables
long balance = 0;
long deposit_count = 0;
long withdrawal_count = 0;
long bonus_count = 0;

// Synchronization primitives
pthread_mutex_t balance_mutex;

// Constants
#define MAX_DEPOSITS 2000000
#define MAX_WITHDRAWALS 2000000
#define BONUS_AMOUNT 50
#define BONUS_TRIGGER 200

// -----------------------------------------
// Thread A — Deposit Operations
// -----------------------------------------
void* deposit_thread(void* arg) {
    while (1) {
        // ----- Entry Section -----
        pthread_mutex_lock(&balance_mutex);

        // ----- Critical Section -----
        if (deposit_count >= MAX_DEPOSITS) {
            pthread_mutex_unlock(&balance_mutex);
            break;  // done with deposits
        }

        // Perform one deposit
        balance += 1;
        deposit_count++;

        // Check bonus
        if (balance % BONUS_TRIGGER == 0 && deposit_count + BONUS_AMOUNT <= MAX_DEPOSITS) {
            balance += BONUS_AMOUNT;
            deposit_count += BONUS_AMOUNT;
            bonus_count++;
        }

        // ----- Exit Section -----
        pthread_mutex_unlock(&balance_mutex);

        // ----- Remainder Section -----
        // (no operation — allows other threads to run)
    }

    printf("I’m Thread A, I did %ld deposit operations and I got the bonus %ld times. balance = %ld\n",
           deposit_count, bonus_count, balance);

    pthread_exit(NULL);
}

// -----------------------------------------
// Thread B — Withdrawal Operations
// -----------------------------------------
void* withdraw_thread(void* arg) {
    while (1) {
        // ----- Entry Section -----
        pthread_mutex_lock(&balance_mutex);

        // ----- Critical Section -----
        if (withdrawal_count >= MAX_WITHDRAWALS) {
            pthread_mutex_unlock(&balance_mutex);
            break;  // done with withdrawals
        }

        balance--;
        withdrawal_count++;

        // ----- Exit Section -----
        pthread_mutex_unlock(&balance_mutex);

        // ----- Remainder Section -----
        // (no operation)
    }

    printf("I’m Thread B, I did %ld withdraw operations. balance = %ld\n",
           withdrawal_count, balance);
    pthread_exit(NULL);
}

// -----------------------------------------
// Main / Parent Thread
// -----------------------------------------
int main() {
    pthread_t threadA, threadB;

    pthread_mutex_init(&balance_mutex, NULL);

    // Create threads
    pthread_create(&threadA, NULL, deposit_thread, NULL);
    pthread_create(&threadB, NULL, withdraw_thread, NULL);

    // Wait for both threads to complete
    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    printf("From parent: Final balance = %ld\n", balance);

    pthread_mutex_destroy(&balance_mutex);

    return 0;
}
