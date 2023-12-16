ifndef LLIST_H_ 
#define LLIST_H_

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/time.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <atomic>
using namespace std;

struct ListNode {
    int value;  // Data
    atomic<ListNode*> nextVertex; // Pointer to the next entry
    atomic<ListNode*> nextEdge;   // Pointer to the next adjacency list
};

struct TimeDifference {
    int seconds;
    int microseconds;
};

TimeDifference* calculateTimeDifference(const timeval* start, const timeval* end) {
    TimeDifference* diff = new TimeDifference();

    if (start->tv_sec == end->tv_sec) {
        diff->seconds = 0;
        diff->microseconds = end->tv_usec - start->tv_usec;
    } else {
        diff->microseconds = 1000000 - start->tv_usec;
        diff->seconds = end->tv_sec - start->tv_sec - 1;
        diff->microseconds += end->tv_usec;
        if (diff->microseconds >= 1000000) {
            diff->microseconds -= 1000000;
            diff->seconds += 1;
        }
    }
    return diff;
}

inline int isMarkedReference(long i) {
    return static_cast<int>(i & 0x1L);
}

inline long unsetMark(long i) {
    return i & ~0x1L;
}

inline long setMark(long i) {
    return i | 0x1L;
}

inline long getUnmarkedReference(long w) {
    return w & ~0x1L;
}

inline long getMarkedReference(long w) {
    return w | 0x1L;
}

int globalVariable;


class List {
public:
    ListNode *head, *tail;

    // Constructor to initialize the list
    init() {
        head = new ListNode();
        head->value = INT_MIN;
        head->nextVertex.store(nullptr, memory_order_seq_cst);
        head->nextEdge.store(createEmptyEdgeList(), memory_order_seq_cst);

        tail = new ListNode();
        tail->value = INT_MAX;
        tail->nextVertex.store(nullptr, memory_order_seq_cst);
        tail->nextEdge.store(nullptr, memory_order_seq_cst);

        head->nextVertex.store(tail, memory_order_seq_cst);
    }

    // Create a new vertex with a specified key
    ListNode* createVertex(int key) {
        ListNode* edgeHead = new ListNode();
        edgeHead->value = INT_MIN;
        edgeHead->nextVertex.store(nullptr, memory_order_seq_cst);
        edgeHead->nextEdge.store(nullptr, memory_order_seq_cst);

        ListNode* edgeTail = new ListNode();
        edgeTail->value = INT_MAX;
        edgeTail->nextVertex.store(nullptr, memory_order_seq_cst);
        edgeTail->nextEdge.store(nullptr, memory_order_seq_cst);

        edgeHead->nextEdge.store(edgeTail, memory_order_seq_cst);

        ListNode* vertex = new ListNode();
        vertex->value = key;
        vertex->nextVertex.store(nullptr, memory_order_seq_cst);
        vertex->nextEdge.store(edgeHead, memory_order_seq_cst);

        return vertex;
    }

    // Create a new edge with a specified key
    ListNode* createEdge(int key) {
        ListNode* newEdge = new ListNode();
        newEdge->value = key;
        newEdge->nextVertex.store(nullptr, memory_order_seq_cst);
        newEdge->nextEdge.store(nullptr, memory_order_seq_cst);

        return newEdge;
    }

    // Create an empty edge list
    ListNode* createEmptyEdgeList() {
        ListNode* edgeHead = new ListNode();
        edgeHead->value = INT_MIN;
        edgeHead->nextVertex.store(nullptr, memory_order_seq_cst);
        edgeHead->nextEdge.store(nullptr, memory_order_seq_cst);

        ListNode* edgeTail = new ListNode();
        edgeTail->value = INT_MAX;
        edgeTail->nextVertex.store(nullptr, memory_order_seq_cst);
        edgeTail->nextEdge.store(nullptr, memory_order_seq_cst);

        edgeHead->nextEdge.store(edgeTail, memory_order_seq_cst);
        return edgeHead;
    }

    // Locate the position in the list based on a key
    void locate(ListNode* head, ListNode* tail, ListNode** n1, ListNode** n2, int key) {
        ListNode* pred = nullptr;
        ListNode* curr = nullptr;
        ListNode* succ = nullptr;

        if (head == this->head) { // Locate in the vertex list
            retry_vertex:
            do {
                pred = head;
                curr = pred->nextVertex.load(memory_order_seq_cst);
                while (curr != tail) {
                    succ = curr->nextVertex.load(memory_order_seq_cst);
                    if (!isMarkedReference(reinterpret_cast<long>(succ))) {
                        if (curr->value >= key) {
                            break;
                        }
                        pred = curr;
                    }
                    curr = reinterpret_cast<ListNode*>(getUnmarkedReference(reinterpret_cast<long>(succ)));
                }

                if (pred->nextVertex.compare_exchange_strong(curr, succ, memory_order_seq_cst)) {
                    if (curr->value >= key) {
                        *n1 = pred;
                        *n2 = curr;
                        return;
                    }
                }
            } while (true);
        } else { // Locate in the edge list of the vertex
            retry_edge:
            do {
                pred = head;
                curr = pred->nextEdge.load(memory_order_seq_cst);
                while (curr != tail) {
                    succ = curr->nextEdge.load(memory_order_seq_cst);
                    if (!isMarkedReference(reinterpret_cast<long>(succ))) {
                        if (curr->value >= key) {
                            break;
                        }
                        pred = curr;
                    }
                    curr = reinterpret_cast<ListNode*>(getUnmarkedReference(reinterpret_cast<long>(succ)));
                }

                if (pred->nextEdge.compare_exchange_strong(curr, succ, memory_order_seq_cst)) {
                    if (curr->value >= key) {
                        *n1 = pred;
                        *n2 = curr;
                        return;
                    }
                }
            } while (true);
        }
    }

    bool contains(ListNode* startNode, ListNode* endNode, ListNode** foundNode, int key) {
        ListNode *currentNode, *previousNode;
        if (startNode == head) {  // Search in the vertex list
            previousNode = head;
            locateNode(startNode, endNode, &previousNode, &currentNode, key);
            if ((!currentNode->nextVertex.load(memory_order_seq_cst)) || currentNode->value != key)
                return false;
            else {
                *foundNode = currentNode;
                return true;
            }
        } else {  // Search in the edge list
            previousNode = startNode;
            locateNode(startNode, endNode, &previousNode, &currentNode, key);
            if ((!currentNode->nextEdge.load(memory_order_seq_cst)) || currentNode->value != key)
                return false;
            else {
                *foundNode = currentNode;
                return true;
            }
        }
    }

    bool addNode(ListNode* startNode, ListNode* endNode, int key) {
        ListNode *previousNode, *currentNode;
        if (startNode == head) {  // Vertex add
            ListNode *newVertex = createVertex(key);
            while (true) {
                locateNode(startNode, endNode, &previousNode, &currentNode, key);
                if (currentNode->value == key) {
                    return true;  // Vertex already exists
                } else {
                    newVertex->nextVertex.store(currentNode, memory_order_seq_cst);
                    if (previousNode->nextVertex.compare_exchange_strong(currentNode, newVertex, memory_order_seq_cst))
                        return true;
                }
            }
        } else {  // Edge add
            ListNode *newEdge = createEdge(key);
            while (true) {
                locateNode(startNode, endNode, &previousNode, &currentNode, key);
                if (currentNode->value == key) {
                    return false;  // Edge already exists
                } else {
                    newEdge->nextEdge.store(currentNode, memory_order_seq_cst);
                    if (previousNode->nextEdge.compare_exchange_strong(currentNode, newEdge, memory_order_seq_cst))
                        return true;
                }
            }
        }
    }

    bool removeNode(ListNode* startNode, ListNode* endNode, int key) {
        ListNode *previousNode, *currentNode, *successorNode;
        if (startNode == head) {  // Vertex remove
            do {
                locateNode(startNode, endNode, &previousNode, &currentNode, key);
                if (currentNode->value != key)
                    return false;  // Vertex not found
                successorNode = currentNode->nextVertex.load(memory_order_seq_cst);
                if (!isMarkedReference(reinterpret_cast<long>(successorNode))) {
                    if (atomic_compare_exchange_strong_explicit(&currentNode->nextVertex, &successorNode, reinterpret_cast<ListNode*>(markReference(reinterpret_cast<long>(successorNode))), memory_order_seq_cst, memory_order_seq_cst))
                        break;
                }
            } while (true);
            if (!atomic_compare_exchange_strong_explicit(&previousNode->nextVertex, &currentNode, successorNode, memory_order_seq_cst, memory_order_seq_cst))
                locateNode(startNode, endNode, &previousNode, &currentNode, key);
            return true;  // Vertex removed
        } else {  // Edge remove
            do {
                locateNode(startNode, endNode, &previousNode, &currentNode, key);
                if (currentNode->value != key)
                    return false;  // Edge not found
                successorNode = currentNode->nextEdge.load(memory_order_seq_cst);
                if (!isMarkedReference(reinterpret_cast<long>(successorNode))) {
                    if (atomic_compare_exchange_strong_explicit(&currentNode->nextEdge, &successorNode, reinterpret_cast<ListNode*>(markReference(reinterpret_cast<long>(successorNode))), memory_order_seq_cst, memory_order_seq_cst))
                        break;
                }
            } while (true);
            if (!atomic_compare_exchange_strong_explicit(&previousNode->nextEdge, &currentNode, successorNode, memory_order_seq_cst, memory_order_seq_cst))
                locateNode(startNode, endNode, &previousNode, &currentNode, key);
            return true;  // Edge removed
        }
    }

    void displayList() {
        ListNode* currentNode = head->nextVertex;
        cout << "Head -> ";
        while (currentNode != tail) {
            cout << currentNode->value << " -> ";
            currentNode = currentNode->nextVertex;
        }
        cout << "Tail" << endl;
    }

};




class Graph {
    List vertexList;

public:      
    Graph() {
        vertexList.init();
    }

    bool addVertex(int key) {
        return vertexList.addNode(vertexList.head, vertexList.tail, key);
    }

    bool removeVertex(int key) {
        return vertexList.removeNode(vertexList.head, vertexList.tail, key);
    }

    bool containsVertex(int key) {
        ListNode *foundNode = nullptr;
        return vertexList.contains(vertexList.head, vertexList.tail, &foundNode, key);
    }

    bool addEdge(int fromKey, int toKey) {
        ListNode *fromVertex, *toVertex;
        bool fromExists = vertexList.contains(vertexList.head, vertexList.tail, &fromVertex, fromKey);
        bool toExists = vertexList.contains(vertexList.head, vertexList.tail, &toVertex, toKey);

        if (!fromExists || !toExists) {
            return false;  // Either of the vertices does not exist
        }

        return vertexList.addNode(fromVertex->nextEdge, nullptr, toKey);  // Add edge to the edge list of 'fromVertex'
    }

    bool removeEdge(int fromKey, int toKey) {
        ListNode *fromVertex;
        bool fromExists = vertexList.contains(vertexList.head, vertexList.tail, &fromVertex, fromKey);

        if (!fromExists) {
            return false;  // Vertex from which the edge starts does not exist
        }

        return vertexList.removeNode(fromVertex->nextEdge, nullptr, toKey);  // Remove edge from the edge list of 'fromVertex'
    }

    bool containsEdge(int fromKey, int toKey) {
        ListNode *fromVertex, *toVertex;
        bool fromExists = vertexList.contains(vertexList.head, vertexList.tail, &fromVertex, fromKey);

        if (!fromExists) {
            return false;  // Vertex from which the edge starts does not exist
        }

        return vertexList.contains(fromVertex->nextEdge, nullptr, &toVertex, toKey);  // Check if edge exists in the edge list of 'fromVertex'
    }

    void printGraph() {
        ListNode *vertex = vertexList.head->nextVertex;
        while (vertex != vertexList.tail) {
            cout << vertex->value << " -> ";
            ListNode *edge = vertex->nextEdge->nextEdge;
            while (edge != nullptr && edge->value != INT_MAX) {
                cout << edge->value << " ";
                edge = edge->nextEdge;
            }
            cout << endl;
            vertex = vertex->nextVertex;
        }
    }

    void initializeGraph(int numberOfVertices) {
        for (int i = 0; i < numberOfVertices; i++) {
            addVertex(i);
        }
    }
};

   
