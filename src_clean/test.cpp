#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>


#include "coerse.cpp" // Use the correct implementation

using namespace std;

int globalVertexID;
double operationDurationSeconds;
struct timeval startTime, currentTime;
TIME_DIFF *elapsedTime;
int numberOfThreads, totalOperationsPerformed;

pthread_mutex_t lock;

typedef struct ThreadInfo {
    long threadId;
    Graph graph;
} ThreadInfo;

void* performGraphOperations(void* threadInfo) {
    ThreadInfo *info = (ThreadInfo*) threadInfo;
    Graph localGraph = info->graph;
    int sourceVertex, destinationVertex;
    int operationResult;

    long long int totalOperations = 10000000000;
    long long int addEdgeOperations = totalOperations * 0.25;
    long long int addVertexOperations = totalOperations * 0.25;
    long long int removeVertexOperations = totalOperations * 0.1;
    long long int removeEdgeOperations = totalOperations * 0.1;
    long long int containsVertexOperations = totalOperations * 0.15;
    long long int containsEdgeOperations = totalOperations * 0.15;

    long long int remainingOperations = addEdgeOperations + addVertexOperations + removeVertexOperations + removeEdgeOperations + containsVertexOperations + containsEdgeOperations;

    while(remainingOperations > 0) {
        gettimeofday(&currentTime, NULL);
        elapsedTime = my_difftime(&startTime, &currentTime);

        if(elapsedTime->secs >= operationDurationSeconds)
            break;

        int operationType = rand() % 6;
        switch (operationType) {
            case 0: // Add Edge
                if(addEdgeOperations > 0) {
                    do {
                        sourceVertex = rand() % globalVertexID;
                        destinationVertex = rand() % globalVertexID;
                    } while(sourceVertex == destinationVertex || sourceVertex == 0 || destinationVertex == 0);
                    operationResult = localGraph.AddE(sourceVertex, destinationVertex);
                    addEdgeOperations--;
                    remainingOperations--;
                    totalOperationsPerformed++;
                }
                break;
            case 1: // Add Vertex
                if(addVertexOperations > 0) {
                    destinationVertex = globalVertexID;
                    globalVertexID++;
                    operationResult = localGraph.AddV(destinationVertex);
                    addVertexOperations--;
                    remainingOperations--;
                    totalOperationsPerformed++;
                }
                break;
            case 2: // Remove Vertex
                if(removeVertexOperations > 0) {
                    do {
                        destinationVertex = rand() % globalVertexID;
                    } while(destinationVertex == 0);
                    operationResult = localGraph.RemoveV(destinationVertex);
                    removeVertexOperations--;
                    remainingOperations--;
                    totalOperationsPerformed++;
                }
                break;
            case 3: // Remove Edge
                if(removeEdgeOperations > 0) {
                    do {
                        sourceVertex = rand() % globalVertexID;
                        destinationVertex = rand() % globalVertexID;
                    } while(sourceVertex == destinationVertex || sourceVertex == 0 || destinationVertex == 0);
                    operationResult = localGraph.RemoveE(sourceVertex, destinationVertex);
                    removeEdgeOperations--;
                    remainingOperations--;
                    totalOperationsPerformed++;
                }
                break;
            case 4: // Contains Vertex
                if(containsVertexOperations > 0) {
                    do {
                        sourceVertex = rand() % globalVertexID;
                    } while(sourceVertex == 0);
                    operationResult = localGraph.ContainsV(sourceVertex);
                    containsVertexOperations--;
                    remainingOperations--;
                    totalOperationsPerformed++;
                }
                break;
            case 5: // Contains Edge
                if(containsEdgeOperations > 0) {
                    do {
                        sourceVertex = rand() % globalVertexID;
                        destinationVertex = rand() % globalVertexID;
                    } while(sourceVertex == destinationVertex || sourceVertex == 0 || destinationVertex == 0);
                    operationResult = localGraph.ContainsE(sourceVertex, destinationVertex);
                    containsEdgeOperations--;
                    remainingOperations--;
                    totalOperationsPerformed++;
                }
                break;
        }
    }
    return NULL;
}


int main(int argc, char* argv[]) {
    if(argc < 3) {
        cout << "Usage: [program] [#threads] [#initial vertices] [#duration in seconds]" << endl;
        return 0;
    }

    numberOfThreads = atoi(argv[1]);
    int initialVertexCount = atoi(argv[2]);
    operationDurationSeconds = atoi(argv[3]);

    pthread_mutex_init(&lock, NULL);

    globalVertexID = initialVertexCount + 1;
    Graph sharedGraph;
    sharedGraph.initGraph(initialVertexCount);

    cout << "Number of Threads: " << numberOfThreads << endl;
    cout << "Initial graph with " << initialVertexCount << " vertices created." << endl;

    pthread_t *threads = new pthread_t[numberOfThreads];
    pthread_attr_t threadAttributes;
    pthread_attr_init(&threadAttributes);
    pthread_attr_setdetachstate(&threadAttributes, PTHREAD_CREATE_JOINABLE);

    gettimeofday(&startTime, NULL);
    cout << "Timer started . . ." << endl;

    for (int i = 0; i < numberOfThreads; i++) {
        ThreadInfo *info = (ThreadInfo*) malloc(sizeof(ThreadInfo));
        info->threadId = i;
        info->graph = sharedGraph;
        pthread_create(&threads[i], &threadAttributes, performGraphOperations, (void*) info);
    }

    for (int i = 0; i < numberOfThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    cout << operationDurationSeconds << " seconds elapsed" << endl;
    cout << "Total operations performed: " << totalOperationsPerformed << endl;

    gettimeofday(&currentTime, NULL);
    elapsedTime = my_difftime(&startTime, &currentTime);

    return 0;
}
