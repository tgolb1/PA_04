#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include "buffer.h"

struct buffer_item buffer[BUFFER_SIZE]; //shared from buffer.h
int insert_pos = 0, remove_pos = 0;
sem_t empty, full;
pthread_mutex_t mutex;

uint16_t checksum(char *addr, uint32_t count);


int insert_item(struct buffer_item item){
        sem_wait(&empty);
        pthread_mutex_lock(&mutex); //start critical section, lock mutex

        /* insert item into buffer */
        buffer[insert_pos] = item;
        insert_pos = (insert_pos + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); //end critical section, unlock mutex
        sem_post(&full);
        return 1;
}

int remove_item (struct buffer_item *item){

        sem_wait(&full);
        pthread_mutex_lock(&mutex); //start critical section, lock mutex

        /* Remove item from buffer */

        item = &buffer[remove_pos];
        remove_pos = (remove_pos + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); //unlock mutex, end critical section
        sem_post(&empty);
        return 1;
}



void *producer(void *param) {
    BUFFER_ITEM item;

    while(1){
        sleep(rand() % 3); //sleep for random time

        for (int i = 0; i < 30; i++) {
            item.data[i] = (uint8_t)(rand() % 256); //generate rand data
        }
        item.cksum = checksum((char *)item.data, sizeof(item.data)); //calculate checksum

        if (insert_item(item) == 1){
            printf("producer produced %d\n", item);
        } else {
            printf("error occurred\n");
        }
    }
    return NULL;
}

void *consumer(void *param) {
    while (1) { //while true
        BUFFER_ITEM item;
        sleep(rand() % 3); //sleep briefly

        uint16_t calculated_cksum = checksum((char *)item.data, sizeof(item.data));

        if (insert_item(item) == 1){
            printf("consumer consumed %d\n", item);
        } else {
            printf("error occurred\n");
        }
    }
    return NULL;
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <delay> <#producer threads> <#consumer threads>\n", argv[0]);
        return -1;
    }

    int delay = atoi(argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);

    if (delay <= 0 || num_producers <= 0 || num_consumers <= 0) {
        printf("Invalid arguments\n");
        return -1;
    }

    //initialize semaphores and mutex
    sem_t *empty = sem_open("/empty_sem", O_CREAT, 0644, BUFFER_SIZE);
    sem_t *full = sem_open("/full_sem", O_CREAT, 0644, 0);

    pthread_mutex_init(&mutex, NULL);

    /* create producer and consumer threads */
    pthread_t producers[num_producers], consumers[num_consumers];
    for (int i = 0; i < num_producers; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }
    for (int i = 0; i < num_consumers; i++) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    sleep(delay);

    //exiting...
    printf("Terminating...\n");
    sem_close(empty);
    sem_close(full);
    sem_unlink("/empty_sem");
    sem_unlink("/full_sem");
    pthread_mutex_destroy(&mutex);
    printf("done\n");
    return 0;
}