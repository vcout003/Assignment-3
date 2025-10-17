#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//synchronization
long balance = 0;
long deposit_count = 0;
long withdrawal_count = 0;
long bonus_count = 0;

pthread_mutex_t balance_mutex;
pthread_cond_t condA_done;
int a_done = 0;

#define MAX_DEPOSITS 2000000
#define MAX_WITHDRAWALS 2000000
#define BONUS_AMOUNT 50
#define BONUS_TRIGGER 200


// ------------------------------
// Thread A — Deposit Operations 
// ------------------------------
void* deposit_thread(void* arg) {
    while (deposit_count < MAX_DEPOSITS) {

        pthread_mutex_lock(&balance_mutex);

        if (deposit_count < MAX_DEPOSITS) {
            balance += 1;
            deposit_count++;

            // Bonus counts toward the 2,000,000 limit
            if (balance % BONUS_TRIGGER == 0 && deposit_count + BONUS_AMOUNT <= MAX_DEPOSITS) {
                balance += BONUS_AMOUNT;
                deposit_count += BONUS_AMOUNT;
                bonus_count++;
            }
        }

        pthread_mutex_unlock(&balance_mutex);
    }

    printf("I’m Thread A, I did %ld deposit operations and I got the bonus %ld times. balance = %ld\n", deposit_count, bonus_count, balance);

    // Signal Thread B
    pthread_mutex_lock(&balance_mutex);
    a_done = 1;
    pthread_cond_signal(&condA_done);
    pthread_mutex_unlock(&balance_mutex);

    pthread_exit(NULL);
}

// ------------------------------
// Thread B — Withdrawal Operations 
// ------------------------------
void* withdraw_thread(void* arg) { //Victoria
    while (withdrawal_count < MAX_WITHDRAWALS) {
        // ----- Entry Section -----
        pthread_mutex_lock(&balance_mutex);

        // ----- Critical Section -----
        if (withdrawal_count < MAX_WITHDRAWALS) {
            balance--;
            withdrawal_count++;
        }

        // ----- Exit Section -----
        pthread_mutex_unlock(&balance_mutex);
    }

    printf("I’m Thread B, I did %ld withdraw operations. balance = %ld\n",
           withdrawal_count, balance);
    pthread_exit(NULL);
}


// ------------------------------
// Main / Parent Thread
// ------------------------------
int main() {
    pthread_t threadA, threadB;

    pthread_mutex_init(&balance_mutex, NULL);
    pthread_cond_init(&condA_done, NULL);

    // Create threads
    pthread_create(&threadA, NULL, deposit_thread, NULL);
    pthread_create(&threadB, NULL, withdraw_thread, NULL);

    // Wait for both threads to finish
    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    printf("From parent: Final balance = %ld\n", balance);

    pthread_mutex_destroy(&balance_mutex);
    pthread_cond_destroy(&condA_done);
    return 0;
}

