#include <iostream>
#include <float.h>
#include <stdint.h>
#include <cstdlib>
#include <pthread.h>
#include <cassert>
#include <limits.h>
#include <sys/time.h>
#include <vector>
#include <ctime>        // std::time
#include <random>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <atomic>

using namespace std;

// Node structure for the linked list
struct ListNode {
    int value;                   // Data
    atomic<ListNode*> nextVertex;  // Pointer to the next vertex
    atomic<ListNode*> nextEdge;    // Pointer to the next adjacency list
};

// Structure for calculating time differences
struct TimeDifference {
    int seconds;
    int microseconds;
};

// Function to calculate the time difference
TimeDifference* calculateTimeDifference(const timeval* start, const timeval* end) {
    auto* diff = new TimeDifference();

    if (start->tv_sec == end->tv_sec) {
        diff->seconds = 0;
        diff->microseconds = end->tv_usec - start->tv_usec;
    } else {
        diff->microseconds = 1000000 - start->tv_usec + end->tv_usec;
        diff->seconds = end->tv_sec - start->tv_sec - 1;
        if (diff->microseconds >= 1000000) {
            diff->microseconds -= 1000000;
            diff->seconds += 1;
        }
    }
    return diff;
}

class List {
public:
    ListNode *head, *tail;

    // Constructor for the List class.
    List() {
        init();
    }

    // Initializes the list with head and tail nodes.
    void init() {
        head = new ListNode(INT_MIN);
        tail = new ListNode(INT_MAX);
        head->nextVertex = tail;
    }

    // Creates a new vertex node with associated edge head and tail.
    ListNode* createVertex(int key) {
        ListNode* edgeHead = new ListNode(INT_MIN);
        ListNode* edgeTail = new ListNode(INT_MAX);
        edgeHead->nextEdge = edgeTail;

        ListNode* vertex = new ListNode(key);
        vertex->nextEdge = edgeHead;
        return vertex;
    }

    // Creates a new edge node.
    ListNode* createEdge(int key) {
        return new ListNode(key);
    }

    void locate(ListNode* head, ListNode* tail, ListNode** pred, ListNode** curr, int key) {
        *pred = head; 
        *curr = (*pred)->nextVertex;

        if (head->nextVertex == tail) {
            *n1 = head;
            *n2 = tail;
            return;
        }

        if (head == this->head) {
            while (*curr != tail && (*curr)->value < key) {
                *pred = *curr;
                *curr = (*curr)->nextVertex;
            }
        } else {
            *curr = (*pred)->nextEdge; 
            while (*curr != tail && (*curr)->value < key) {
                *pred = *curr;
                *curr = (*curr)->nextEdge;
            }
        }
    }

    bool add(ListNode* h, ListNode* t, int key) {
        ListNode* pred, *curr;
        locate(h, t, &pred, &curr, key);

        if (curr->value == key) {
            return false; // Node already exists
        }

        // Create a new vertex or edge node
        ListNode* newNode;
        if (h == head) {
            newNode = createVertex(key); // Create a new vertex
            newNode->nextVertex = curr;  // Link the new vertex
            pred->nextVertex = newNode;
        } else {
            newNode = createEdge(key); // Create a new edge
            newNode->nextEdge = curr;  // Link the new edge
            pred->nextEdge = newNode;
        }
        
        return true;
    }

    bool Remove(ListNode* h, ListNode* t, int key) {
        ListNode* pred, *curr;
        locate(h, t, &pred, &curr, key);

        if (curr == t || curr->value != key) {
            return false; // Node not found or end of list reached
        }

        // Adjust pointers to remove the node
        if (h == head) {
            pred->nextVertex = curr->nextVertex;
        } else {
            pred->nextEdge = curr->nextEdge;
        }

        delete curr; // Free the memory allocated for the node
        return true;
    }

    void print() {
        ListNode* curr = head->nextVertex; 
        cout << "Head -> ";      
        while (curr != tail) {
            cout << curr->value << " -> ";
            curr = curr->nextVertex;
        }
        cout << "Tail" << endl;
    }

};

class Graph {
    List list;

public:
    Graph() {
        list.init();
    }

    bool addVertex(int key) {
        return list.add(list.head, list.tail, key);
    }

    bool removeVertex(int key) {
        return list.remove(list.head, list.tail, key);
    }

    bool containsVertex(int key) {
        return list.contains(list.head, list.tail, key);
    }

    bool addEdge(int key1, int key2) {
        ListNode* u;
        if (!list.contains(list.head, list.tail, key1)) return false;
        if (!list.contains(list.head, list.tail, key2)) return false;

        list.locate(list.head, list.tail, &u, key1);
        return list.add(u->nextEdge, nullptr, key2);
    }

    bool removeEdge(int key1, int key2) {
        ListNode* u;
        if (!list.contains(list.head, list.tail, key1)) return false;

        list.locate(list.head, list.tail, &u, key1);
        return list.remove(u->nextEdge, nullptr, key2);
    }

    bool containsEdge(int key1, int key2) {
        ListNode* u;
        if (!list.contains(list.head, list.tail, key1)) return false;

        list.locate(list.head, list.tail, &u, key1);
        return list.contains(u->nextEdge, nullptr, key2);
    }

    void printGraph() {
        ListNode* vertex = list.head->nextVertex;
        while (vertex != list.tail) {
            cout << vertex->value << " -> ";
            ListNode* edge = vertex->nextEdge->nextEdge;
            while (edge != vertex->nextEdge) {
                cout << edge->value << " ";
                edge = edge->nextEdge;
            }
            cout << endl;
            vertex = vertex->nextVertex;
        }
    }

    void initGraph(int n) {
        for (int i = 0; i < n; ++i) {
            addVertex(i);
        }
    }
};


