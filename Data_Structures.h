#pragma once
#include <iostream>
#include <string>

using namespace std;

// ==========================================
// 0. HELPER FUNCTIONS
// ==========================================
template <typename T>
T myAbs(T value) {
    if (value < 0) {
        return -value;
    }
    return value;
}

// ==========================================
// 1. LINKED LIST (Generic)
// ==========================================
// Data Structure: Singly Linked List - O(1) insertion at front/back, O(n) search/removal

// Node structure for linked list
template<typename T>
struct LListNode {
    T data;
    LListNode<T>* next;
    LListNode(const T& d) : data(d), next(nullptr) {}
};

// Singly-linked list with head and tail pointers for efficient insertion
template<typename T>
class LinkedList {
private:
    LListNode<T>* head;
    LListNode<T>* tail;

public:
    LinkedList() : head(nullptr), tail(nullptr) {}
    ~LinkedList() { clear(); }

    // ---------- COPY CONSTRUCTOR ----------
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr) {
        LListNode<T>* cur = other.head;
        while (cur) {
            insertBack(cur->data);     // deep copy each node
            cur = cur->next;
        }
    }

    // ---------- ASSIGNMENT OPERATOR ----------
    LinkedList& operator=(const LinkedList& other) {
        if (this == &other) return *this;
        clear();
        LListNode<T>* cur = other.head;
        while (cur) {
            insertBack(cur->data);
            cur = cur->next;
        }
        return *this;
    }

    // Get head pointer for iteration
    // Time Complexity: O(1)
    LListNode<T>* getHead() { return head; }

    // Set head pointer (used for removing first node)
    // Time Complexity: O(n) - recomputes tail if needed
    void setHead(LListNode<T>* h) {
        head = h;
        if (!head) {
            tail = nullptr;
        }
        else {
            // Recompute tail safely in case someone passed a mid-list pointer
            LListNode<T>* t = head;
            while (t->next) t = t->next;
            tail = t;
        }
    }

    // Insert element at the end of the list
    // Time Complexity: O(1) - uses tail pointer
    void insertBack(const T& val) {
        LListNode<T>* node = new LListNode<T>(val);
        if (!head) {
            head = tail = node;
        }
        else {
            tail->next = node;
            tail = node;
        }
    }

    // Insert element at the beginning of the list
    // Time Complexity: O(1)
    void insertFront(const T& val) {
        LListNode<T>* node = new LListNode<T>(val);
        node->next = head;
        head = node;
        if (!tail) tail = node;
    }

    // Remove the first occurrence of value from the list
    // Time Complexity: O(n) - linear search
    bool removeByValue(const T& val) {
        LListNode<T>* curr = head;
        LListNode<T>* prev = nullptr;
        while (curr) {
            if (curr->data == val) {
                if (prev) prev->next = curr->next;
                else head = curr->next;
                if (curr == tail) tail = prev;
                delete curr;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    // Clear entire list and deallocate all nodes
    // Time Complexity: O(n)
    void clear() {
        LListNode<T>* cur = head;
        while (cur) {
            LListNode<T>* nx = cur->next;
            delete cur;
            cur = nx;
        }
        head = tail = nullptr;
    }
};


// ==========================================
// 2. STACK (Generic)
// ==========================================
// Data Structure: Linked List-based Stack - O(1) push/pop operations

template <typename T>
struct StackNode {
    T data;
    StackNode<T>* next;
    StackNode(T d) : data(d), next(nullptr) {}
};

template <typename T>
class Stack {
    StackNode<T>* top;

public:
    Stack() : top(nullptr) {}

    // Push element onto stack
    // Time Complexity: O(1)
    void push(T val) {
        StackNode<T>* newNode = new StackNode<T>(val);
        newNode->next = top;
        top = newNode;
    }

    T pop() {
        if (!top) {
            cout << "Stack Underflow" << endl;
            return T(); // Return default
        }
        T val = top->data;
        StackNode<T>* temp = top;
        top = top->next;
        delete temp;
        return val;
    }

    T peek() {
        if (!top) return T();
        return top->data;
    }

    // Check if stack is empty
    // Time Complexity: O(1)
    bool isEmpty() { return top == nullptr; }
};

// ==========================================
// 3. CIRCULAR QUEUE (Generic)
// ==========================================
// Data Structure: Circular Linked List-based Queue - O(1) enqueue/dequeue operations

template <typename T>
struct QueueNode {
    T data;
    QueueNode<T>* next;
    QueueNode(T d) : data(d), next(nullptr) {}
};

template <typename T>
class CircularQueue {
    QueueNode<T>* rear;
    int size;
    int capacity;

public:
    CircularQueue(int cap = 50) : rear(nullptr), size(0), capacity(cap) {}

    // Add element to rear of queue
    // Time Complexity: O(1)
    bool enqueue(T val) {
        if (size >= capacity) {
            cout << "[Queue Full] Cannot add item." << endl;
            return false;
        }

        QueueNode<T>* newNode = new QueueNode<T>(val);

        if (isEmpty()) {
            rear = newNode;
            rear->next = rear; // Points to itself (Circular)
        }
        else {
            newNode->next = rear->next; // New node points to Front
            rear->next = newNode;       // Old Rear points to New Node
            rear = newNode;             // New Node becomes Rear
        }
        size++;
        return true;
    }

    // Remove element from front of queue
    // Time Complexity: O(1)
    T dequeue() {
        if (isEmpty()) return T();

        QueueNode<T>* front = rear->next;
        T val = front->data;

        if (rear == front) {
            delete front;
            rear = nullptr;
        }
        else {
            rear->next = front->next;
            delete front;
        }
        size--;
        return val;
    }

    // Peek at front element without removing
    // Time Complexity: O(1)
    T peek() {
        if (isEmpty()) return T();
        return rear->next->data;
    }

    // Check if queue is empty
    // Time Complexity: O(1)
    bool isEmpty() { return rear == nullptr; }

    // Check if queue is full
    // Time Complexity: O(1)
    bool isFull() { return size == capacity; }
};

// ==========================================
// 4. TEMPLATIZED GRAPH (Adjacency List + Dijkstra)
// ==========================================
// Data Structure: Adjacency List Graph - O(V+E) space, O(E log V) shortest path

// Forward declaration
template <typename T> struct VertexNode;

// Edge structure for adjacency list representation
template <typename T>
struct EdgeNode {
    T destinationID;
    int weight;
    EdgeNode<T>* next;
    bool isEmergencyRoute;

    EdgeNode(T dest, int w, bool emergency = false)
        : destinationID(dest), weight(w), isEmergencyRoute(emergency), next(nullptr) {
    }
};

// Vertex structure with Dijkstra algorithm support
template <typename T>
struct VertexNode {
    T id;
    double x, y;

    EdgeNode<T>* headEdge;
    VertexNode<T>* next;

    // Dijkstra Helpers
    int minDistance;
    bool visited;
    T parentID;
    bool hasParent;

    VertexNode(T key, double xCoord, double yCoord)
        : id(key), x(xCoord), y(yCoord), headEdge(nullptr), next(nullptr),
        minDistance(2147483647), visited(false), hasParent(false) {
    }
};

// Internal MinHeap for Dijkstra's algorithm priority queue
// Time Complexity: O(log n) insert/extract
template <typename T>
class GraphMinHeap {
private:
    struct HeapNode {
        T vertexID;
        int distance;

        HeapNode() : distance(0) {}
    };

    HeapNode* array;
    int capacity;
    int size;

public:
    GraphMinHeap(int cap) {
        capacity = cap;
        size = 0;
        array = new HeapNode[capacity];
    }

    ~GraphMinHeap() { delete[] array; }

    bool isEmpty() { return size == 0; }

    void push(T id, int dist) {
        if (size == capacity) return;
        int i = size;
        array[size].vertexID = id;
        array[size].distance = dist;
        size++;

        while (i != 0 && array[(i - 1) / 2].distance > array[i].distance) {
            HeapNode temp = array[i];
            array[i] = array[(i - 1) / 2];
            array[(i - 1) / 2] = temp;
            i = (i - 1) / 2;
        }
    }

    struct NodeResult { T id; int dist; };
    NodeResult pop() {
        if (size == 0) return { T(), -1 };
        NodeResult root = { array[0].vertexID, array[0].distance };
        array[0] = array[size - 1];
        size--;

        int i = 0;
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;

            if (left < size && array[left].distance < array[smallest].distance) smallest = left;
            if (right < size && array[right].distance < array[smallest].distance) smallest = right;

            if (smallest != i) {
                HeapNode temp = array[i];
                array[i] = array[smallest];
                array[smallest] = temp;
                i = smallest;
            }
            else break;
        }
        return root;
    }
};

template <typename T>
class Graph {
private:
    VertexNode<T>* headVertex;
    int vertexCount;

    VertexNode<T>* findVertex(T id) {
        VertexNode<T>* current = headVertex;
        while (current != nullptr) {
            if (current->id == id) return current;
            current = current->next;
        }
        return nullptr;
    }

    void printPathRecursive(T currentID) {
        VertexNode<T>* node = findVertex(currentID);
        if (node && node->hasParent) {
            printPathRecursive(node->parentID);
            cout << " -> ";
        }
        cout << currentID;
    }

public:
    Graph() : headVertex(nullptr), vertexCount(0) {}

    // Add a vertex to the graph
    // Time Complexity: O(1) - checks for duplicates first
    void addVertex(T id, double x = 0.0, double y = 0.0) {
        if (findVertex(id) != nullptr) return;
        VertexNode<T>* newNode = new VertexNode<T>(id, x, y);
        newNode->next = headVertex;
        headVertex = newNode;
        vertexCount++;
    }

    // Add an edge between two vertices
    // Time Complexity: O(1) - creates vertices if they don't exist
    void addEdge(T src, T dest, int weight, bool bidirectional = true, bool isEmergency = false) {
        VertexNode<T>* sourceNode = findVertex(src);
        VertexNode<T>* destNode = findVertex(dest);

        if (!sourceNode) { addVertex(src); sourceNode = findVertex(src); }
        if (!destNode) { addVertex(dest); destNode = findVertex(dest); }

        EdgeNode<T>* newEdge = new EdgeNode<T>(dest, weight, isEmergency);
        newEdge->next = sourceNode->headEdge;
        sourceNode->headEdge = newEdge;

        if (bidirectional) {
            EdgeNode<T>* backEdge = new EdgeNode<T>(src, weight, isEmergency);
            backEdge->next = destNode->headEdge;
            destNode->headEdge = backEdge;
        }
    }

    // Print adjacency list representation of graph
    // Time Complexity: O(V+E)
    void printGraph() {
        VertexNode<T>* current = headVertex;
        cout << "\n--- Adjacency List ---\n";
        while (current != nullptr) {
            cout << current->id << " -> ";
            EdgeNode<T>* edge = current->headEdge;
            while (edge != nullptr) {
                cout << "[" << edge->destinationID << "|" << edge->weight << "km";
                if (edge->isEmergencyRoute) {
                    cout << "|EMERGENCY";
                }
                cout << "] ";
                edge = edge->next;
            }
            cout << endl;
            current = current->next;
        }
        cout << "----------------------\n";
    }

    // Find shortest path using Dijkstra's algorithm
    // Time Complexity: O(E log V) with min-heap priority queue
    void findShortestPath(T startID, T endID, bool allowEmergencyRoutes = false) {
        VertexNode<T>* startNode = findVertex(startID);
        VertexNode<T>* endNode = findVertex(endID);

        if (!startNode || !endNode) {
            cout << "Invalid Locations." << endl;
            return;
        }

        // Reset
        VertexNode<T>* v = headVertex;
        while (v != nullptr) {
            v->minDistance = 2147483647;
            v->visited = false;
            v->hasParent = false;
            v = v->next;
        }

        GraphMinHeap<T> pq(vertexCount * vertexCount + 10);
        startNode->minDistance = 0;
        pq.push(startID, 0);

        while (!pq.isEmpty()) {
            auto current = pq.pop();
            T uID = current.id;
            VertexNode<T>* uNode = findVertex(uID);

            if (uNode->visited) continue;
            uNode->visited = true;
            if (uID == endID) break;

            EdgeNode<T>* edge = uNode->headEdge;
            while (edge != nullptr) {
                // Check if we should skip emergency routes
                if (edge->isEmergencyRoute && !allowEmergencyRoutes) {
                    edge = edge->next;
                    continue;  // Skip this edge
                }

                VertexNode<T>* vNode = findVertex(edge->destinationID);
                int weight = edge->weight;

                if (!vNode->visited && uNode->minDistance != 2147483647) {
                    if (uNode->minDistance + weight < vNode->minDistance) {
                        vNode->minDistance = uNode->minDistance + weight;
                        vNode->parentID = uID;
                        vNode->hasParent = true;
                        pq.push(vNode->id, vNode->minDistance);
                    }
                }
                edge = edge->next;
            }
        }

        if (endNode->minDistance == 2147483647) {
            cout << "No route exists." << endl;
        }
        else {
            cout << "Shortest Distance: " << endNode->minDistance << " km" << endl;
            cout << "Route: ";
            printPathRecursive(endID);
            cout << endl;
        }
    }

    // ==========================================
    // VERTEX AND EDGE ACCESS METHODS
    // ==========================================

    // Get all edges connected to a vertex
    // Time Complexity: O(V) - finds vertex first
    EdgeNode<T>* getEdges(T vertexID) {
        VertexNode<T>* vertex = findVertex(vertexID);
        if (!vertex) {
            return nullptr;
        }
        return vertex->headEdge;
    }

    // Check if vertex exists in graph
    // Time Complexity: O(V)
    bool hasVertex(T vertexID) {
        return findVertex(vertexID) != nullptr;
    }

    // Print all edges connected to a vertex
    // Time Complexity: O(V+E) - finds vertex then iterates edges
    void printVertexEdges(T vertexID) {
        VertexNode<T>* vertex = findVertex(vertexID);
        if (!vertex) {
            cout << "NULL";
            return;
        }

        EdgeNode<T>* edge = vertex->headEdge;

        if (!edge) {
            cout << "NULL";
            return;
        }

        bool first = true;
        while (edge != nullptr) {
            if (!first) cout << " ";
            cout << "[" << edge->destinationID << "|" << edge->weight << "km]";
            edge = edge->next;
            first = false;
        }
    }

    // Get number of edges connected to a vertex (degree)
    // Time Complexity: O(V+E) - finds vertex then counts edges
    int getVertexDegree(T vertexID) {
        VertexNode<T>* vertex = findVertex(vertexID);
        if (!vertex) return 0;

        int degree = 0;
        EdgeNode<T>* edge = vertex->headEdge;

        while (edge != nullptr) {
            degree++;
            edge = edge->next;
        }

        return degree;
    }

    // Get coordinates of a vertex
    // Time Complexity: O(V)
    bool getVertexCoordinates(T vertexID, double& x, double& y) {
        VertexNode<T>* vertex = findVertex(vertexID);
        if (!vertex) return false;

        x = vertex->x;
        y = vertex->y;
        return true;
    }

    // Get total number of vertices in graph
    // Time Complexity: O(1)
    int getVertexCount() {
        return vertexCount;
    }

    // Get head vertex pointer for external iteration
    // Time Complexity: O(1)
    VertexNode<T>* getHeadVertex() {
        return headVertex;
    }
    // Remove vertex and all its edges from graph
    // Time Complexity: O(V+E) - finds vertex, removes edges, updates other vertices
    void removeVertex(T vertexID) {
        VertexNode<T>* current = headVertex;
        VertexNode<T>* prev = nullptr;

        while (current != nullptr) {
            if (current->id == vertexID) {
                // Remove all edges from this vertex first
                EdgeNode<T>* edge = current->headEdge;
                while (edge != nullptr) {
                    EdgeNode<T>* temp = edge;
                    edge = edge->next;
                    delete temp;
                }

                // Remove vertex from linked list
                if (prev == nullptr) {
                    // Removing head vertex
                    headVertex = current->next;
                }
                else {
                    prev->next = current->next;
                }

                delete current;
                vertexCount--;
                cout << "[GRAPH] Removed vertex " << vertexID << " from graph" << endl;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

    // Remove edge between two vertices
    // Time Complexity: O(V+E) - finds source vertex then searches edge list
    void removeEdge(T fromID, T toID) {
        VertexNode<T>* sourceNode = findVertex(fromID);
        if (!sourceNode) return;

        // Remove edge from source to destination
        EdgeNode<T>* current = sourceNode->headEdge;
        EdgeNode<T>* prev = nullptr;

        while (current != nullptr) {
            if (current->destinationID == toID) {
                if (prev == nullptr) {
                    // Removing first edge
                    sourceNode->headEdge = current->next;
                }
                else {
                    prev->next = current->next;
                }
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }
};

// ==========================================
// 5. HASH TABLE (Generic)
// ==========================================
// Data Structure: Chained Hash Table - O(1) average insert/search, O(n) worst case

template <typename T>
struct HashNode {
    string key;
    T value;
    HashNode<T>* next;

    HashNode(string k, T v) : key(k), value(v), next(nullptr) {}
};

template <typename T>
class HashTable {
private:
    HashNode<T>** table;
    int capacity;

    int hashFunction(string key) {
        long long hash = 0;
        long long p = 31;
        long long m = 1000000007LL; // avoid floating modulus
        long long p_pow = 1;
        for (char c : key) {
            hash = (hash + (long long)(unsigned char)c * p_pow) % m;
            p_pow = (p_pow * p) % m;
        }
        return myAbs((int)(hash % capacity));
    }

public:
    HashTable(int size = 100) {
        capacity = size;
        table = new HashNode<T>*[capacity];
        for (int i = 0; i < capacity; i++) table[i] = nullptr;
    }

    ~HashTable() {
        for (int i = 0; i < capacity; ++i) {
            HashNode<T>* cur = table[i];
            while (cur) {
                HashNode<T>* nx = cur->next;
                delete cur;
                cur = nx;
            }
        }
        delete[] table;
    }

    // Insert or update key-value pair
    // Time Complexity: O(1) average, O(n) worst case (all collisions)
    void insert(string key, T item) {
        int index = hashFunction(key);
        HashNode<T>* newNode = new HashNode<T>(key, item);
        newNode->next = table[index];
        table[index] = newNode;
    }

    // Search for value by key, returns pointer or nullptr
    // Time Complexity: O(1) average, O(n) worst case
    T* search(string key) {
        int index = hashFunction(key);
        HashNode<T>* temp = table[index];
        while (temp != nullptr) {
            if (temp->key == key) return &temp->value;
            temp = temp->next;
        }
        return nullptr;
    }

    // Remove key-value pair from hash table
    // Time Complexity: O(1) average, O(n) worst case
    void remove(string key) {
        int index = hashFunction(key);
        HashNode<T>* temp = table[index];
        HashNode<T>* prev = nullptr;

        while (temp != nullptr) {
            if (temp->key == key) {
                if (prev == nullptr) table[index] = temp->next;
                else prev->next = temp->next;
                delete temp;
                return;
            }
            prev = temp;
            temp = temp->next;
        }
    }
};

// ==========================================
// 6. PRIORITY QUEUE (Generic - Max or Min)
// ==========================================
// Data Structure: Binary Heap - O(log n) insert/extract, O(1) peek

template <typename T>
struct PQNode {
    T data;
    double priority; // Lower or Higher depending on mode
};

template <typename T>
class PriorityQueue {
private:
    PQNode<T>* array;
    int capacity;
    int size;
    bool isMinHeap; // true for MinHeap (1 before 10), false for MaxHeap (10 before 1)

    void heapify(int idx) {
        int target = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;

        if (isMinHeap) {
            if (left < size && array[left].priority < array[target].priority) target = left;
            if (right < size && array[right].priority < array[target].priority) target = right;
        }
        else {
            if (left < size && array[left].priority > array[target].priority) target = left;
            if (right < size && array[right].priority > array[target].priority) target = right;
        }

        if (target != idx) {
            PQNode<T> temp = array[idx];
            array[idx] = array[target];
            array[target] = temp;
            heapify(target);
        }
    }

public:
    PriorityQueue(int cap, bool minMode = true) {
        capacity = cap;
        size = 0;
        isMinHeap = minMode;
        array = new PQNode<T>[cap];
    }

    // Insert element with priority
    // Time Complexity: O(log n)
    void push(T item, double prio) {
        if (size == capacity) return;
        size++;
        int i = size - 1;
        array[i] = { item, prio };

        while (i != 0) {
            int parent = (i - 1) / 2;
            bool swapNeeded = false;
            if (isMinHeap && array[parent].priority > array[i].priority) swapNeeded = true;
            else if (!isMinHeap && array[parent].priority < array[i].priority) swapNeeded = true;

            if (swapNeeded) {
                PQNode<T> temp = array[i];
                array[i] = array[parent];
                array[parent] = temp;
                i = parent;
            }
            else break;
        }
    }

    T pop() {
        if (size <= 0) return T();
        T result = array[0].data;
        array[0] = array[size - 1];
        size--;
        heapify(0);
        return result;
    }

    bool isEmpty() { return size == 0; }
};

// ==========================================
// 7. N-ARY TREE (Generic)
// ==========================================
// Data Structure: N-ary Tree - O(n) search/traversal, O(1) child insertion

template <typename T>
struct TreeNode {
    T data;
    TreeNode* parent;
    TreeNode* firstChild;
    TreeNode* nextSibling;

    TreeNode(const T& d)
        : data(d), parent(nullptr), firstChild(nullptr), nextSibling(nullptr) {
    }
};

template <typename T>
class NaryTree {
public:
    TreeNode<T>* root;

    NaryTree(const T& d) {
        root = new TreeNode<T>(d);
    }

    ~NaryTree() {
        destroy(root);
    }

    void destroy(TreeNode<T>* node) {
        if (!node) return;
        TreeNode<T>* c = node->firstChild;
        while (c) {
            TreeNode<T>* next = c->nextSibling;
            destroy(c);
            c = next;
        }
        delete node;
    }

    // Add child node to parent
    // Time Complexity: O(k) where k is number of siblings (to find end)
    void addChild(TreeNode<T>* parent, const T& data) {
        TreeNode<T>* newNode = new TreeNode<T>(data);
        newNode->parent = parent;

        if (!parent->firstChild) {
            parent->firstChild = newNode;
        }
        else {
            TreeNode<T>* s = parent->firstChild;
            while (s->nextSibling) s = s->nextSibling;
            s->nextSibling = newNode;
        }
    }

    // Find node with given value in tree
    // Time Complexity: O(n) - traverses entire tree in worst case
    TreeNode<T>* findNode(TreeNode<T>* node, const T& val) {
        if (!node) return nullptr;
        if (node->data == val) return node;

        TreeNode<T>* c = node->firstChild;
        while (c) {
            TreeNode<T>* r = findNode(c, val);
            if (r) return r;
            c = c->nextSibling;
        }
        return nullptr;
    }

    // Print tree structure with indentation
    // Time Complexity: O(n) - visits all nodes
    void printTree(TreeNode<T>* node, int depth = 0) {
        if (!node) return;

        for (int i = 0; i < depth; i++)
            cout << "  ";

        cout << node->data << "\n";

        TreeNode<T>* child = node->firstChild;
        while (child) {
            printTree(child, depth + 1);
            child = child->nextSibling;
        }
    }
};

// Entry structure for chained hash map
template <typename T>
struct ChainedEntry {
    string key;
    T value;

    ChainedEntry() {}
    ChainedEntry(const string& k, const T& v) : key(k), value(v) {}
};

// Chained hash map implementation using linked lists for collision resolution
// Data Structure: Chained Hash Map - O(1) average insert/search, O(n) worst case
template <typename T>
class ChainedMap {
private:
    int capacity;
    LinkedList< ChainedEntry<T> >* buckets;   // each bucket is a linked list

    // djb2 hash function for string keys
    // Time Complexity: O(k) where k is key length
    int hash(const string& key) const {
        unsigned long h = 5381;
        for (char c : key)
            h = ((h << 5) + h) + (unsigned char)c; // hash * 33 + c
        return (int)(h % capacity);
    }

public:
    ChainedMap(int cap = 101) : capacity(cap) {
        buckets = new LinkedList< ChainedEntry<T> >[capacity];
    }

    ~ChainedMap() {
        delete[] buckets;
    }

    // Insert or update key-value pair
    // Time Complexity: O(1) average, O(n) worst case
    void insert(const string& key, const T& value) {
        int idx = hash(key);
        LinkedList<ChainedEntry<T>>& bucket = buckets[idx];

        LListNode<ChainedEntry<T>>* node = bucket.getHead();

        while (node) {
            if (node->data.key == key) {
                node->data.value = value;   // update
                return;
            }
            node = node->next;
        }

        bucket.insertBack(ChainedEntry<T>(key, value));  // new entry
    }

    // Search for value by key, returns pointer or nullptr
    // Time Complexity: O(1) average, O(n) worst case
    T* search(const string& key) {
        int idx = hash(key);
        LinkedList<ChainedEntry<T>>& bucket = buckets[idx];

        LListNode<ChainedEntry<T>>* node = bucket.getHead();

        while (node) {
            if (node->data.key == key)
                return &node->data.value;
            node = node->next;
        }
        return nullptr;
    }

    // Check if key exists in map
    // Time Complexity: O(1) average, O(n) worst case
    bool contains(const string& key) {
        return search(key) != nullptr;
    }

    // Remove entry by key
    // Time Complexity: O(1) average, O(n) worst case
    bool remove(const string& key) {
        int idx = hash(key);
        LinkedList<ChainedEntry<T>>& bucket = buckets[idx];

        LListNode<ChainedEntry<T>>* node = bucket.getHead();
        LListNode<ChainedEntry<T>>* prev = nullptr;

        while (node) {
            if (node->data.key == key) {
                if (prev) prev->next = node->next;
                else bucket.setHead(node->next);

                node->next = nullptr;
                delete node;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }

    // Clear all entries from map
    // Time Complexity: O(n) - clears all buckets
    void clear() {
        for (int i = 0; i < capacity; i++)
            buckets[i].clear();
    }

    // Get capacity of hash map
    // Time Complexity: O(1)
    int getCapacity() const {
        return capacity;
    }
};

