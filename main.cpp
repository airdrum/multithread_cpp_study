#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <unistd.h>
#include <vector>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
using namespace std;


vector<int> buffer, consume_buffer;
int index = 0;

sem_t full, empty;
pthread_mutex_t mutex;

void* produce(void* arg){
    while(1){
        sleep(1);
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        int item = rand()%100;
        buffer.push_back(item);
        cout << " *** Produced " << item << endl;
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
}

void* consume(void* arg){
    while(1){
        sleep(1);
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int item = buffer.front();
        buffer.erase (buffer.begin());
        cout << "Consumed " << item << endl;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        consume_buffer.push_back(item);
        if(consume_buffer.size()==10){
        	consume_buffer.empty();
        }
        std::cout << "Size : " << consume_buffer.size()<< endl;
    }
}

int main(){
    pthread_t producer, consumer;
    sem_init(&empty, 0, 65535);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&producer, NULL, produce, NULL);

    pthread_create(&consumer, NULL, consume, NULL);
    pthread_exit(NULL);
}
