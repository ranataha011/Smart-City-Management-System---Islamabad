#pragma once
#include <iostream>
#include <string>
#include <cctype>
#include "MainCityGraph.h"
#include "Data_Structures.h"

using namespace std;

// =====================================================
// HIERARCHY ENTRY (Sector → Module → Node)
// =====================================================
struct HierarchyEntry {
    int level;                 // 1=Sector, 2=Module, 3=Node
    string sector;
    ModuleType module;
    string nodeID;
    string nodeName;
    Coordinates coords;
    string key;

    HierarchyEntry() {
        level = 0;
        sector = "";
        module = MODULE_TRANSPORT;
        nodeID = "";
        nodeName = "";
        key = "";
    }

    // Level 1: Sector
    HierarchyEntry(const string& s) {
        level = 1;
        sector = s;
        key = s;
    }

    // Level 2: Module
    HierarchyEntry(const string& s, ModuleType m) {
        level = 2;
        sector = s;
        module = m;
        key = s + "::" + to_string((int)m);
    }

    // Level 3: Node
    HierarchyEntry(const string& s, ModuleType m,
        const string& nid, const string& nm,
        const Coordinates& c) {
        level = 3;
        sector = s;
        module = m;
        nodeID = nid;
        nodeName = nm;
        coords = c;
        key = s + "::" + to_string((int)m) + "::" + nid;
    }
};

// Compares two strings lexicographically
// Returns: -1 if a < b, 0 if a == b, 1 if a > b
int cmpStrSimple(const string& a, const string& b) {
    int len1 = (int)a.length();
    int len2 = (int)b.length();
    int minLen = (len1 < len2 ? len1 : len2);
    
    for (int i = 0; i < minLen; i++) {
        if (a[i] < b[i]) {
            return -1;
        }
        if (a[i] > b[i]) {
            return 1;
        }
    }
    
    if (len1 == len2) {
        return 0;
    }
    return (len1 < len2 ? -1 : 1);
}

// Compares two hierarchy entries by their keys
int cmpEntry(const HierarchyEntry& entryA, const HierarchyEntry& entryB) {
    return cmpStrSimple(entryA.key, entryB.key);
}

// B-Tree node structure for hierarchical data storage
// Data Structure: B-Tree - O(log n) search, insert, delete operations
template<typename T>
class HTNode {
public:
    bool leaf;
    int minDegree;
    int keyCount;
    T* keys;
    HTNode<T>** children;

    // Constructor - creates a B-tree node
    HTNode(int degree, bool isLeaf) {
        minDegree = degree;
        leaf = isLeaf;
        keyCount = 0;
        keys = new T[2 * minDegree - 1];
        children = new HTNode<T>*[2 * minDegree];
        for (int i = 0; i < 2 * minDegree; i++) {
            children[i] = nullptr;
        }
    }

    ~HTNode() {
        delete[] keys;
        delete[] children;
    }

    // Splits a full child node during insertion
    // Data Structure: B-Tree split operation
    void splitChild(int childIndex, HTNode<T>* fullChild) {
        HTNode<T>* newChild = new HTNode<T>(fullChild->minDegree, fullChild->leaf);
        newChild->keyCount = minDegree - 1;

        // Copy the last (minDegree-1) keys from fullChild to newChild
        for (int j = 0; j < minDegree - 1; j++) {
            newChild->keys[j] = fullChild->keys[j + minDegree];
        }

        // Copy the last minDegree children from fullChild to newChild
        if (!fullChild->leaf) {
            for (int j = 0; j < minDegree; j++) {
                newChild->children[j] = fullChild->children[j + minDegree];
            }
        }

        // Reduce the number of keys in fullChild
        fullChild->keyCount = minDegree - 1;

        // Shift children to make space for newChild
        for (int j = keyCount; j >= childIndex + 1; j--) {
            children[j + 1] = children[j];
        }

        // Link newChild as a child of this node
        children[childIndex + 1] = newChild;

        // Shift keys to make space for the middle key
        for (int j = keyCount - 1; j >= childIndex; j--) {
            keys[j + 1] = keys[j];
        }

        // Copy the middle key of fullChild to this node
        keys[childIndex] = fullChild->keys[minDegree - 1];

        keyCount++;
    }

    // Inserts a key into a non-full node
    // Data Structure: B-Tree insertion
    void insertNonFull(const T& newKey) {
        int index = keyCount - 1;

        if (leaf) {
            // Find the location to insert the new key
            while (index >= 0 && cmpEntry(keys[index], newKey) == 1) {
                keys[index + 1] = keys[index];
                index--;
            }
            keys[index + 1] = newKey;
            keyCount++;
        }
        else {
            // Find the child which is going to have the new key
            while (index >= 0 && cmpEntry(keys[index], newKey) == 1) {
                index--;
            }

            index++;

            // Validate child index
            if (index < 0 || index > keyCount || !children[index]) {
                return;
            }

            // If the found child is full, split it
            if (children[index]->keyCount == 2 * minDegree - 1) {
                splitChild(index, children[index]);
                // After split, the middle key goes up and the child is split
                // Decide which of the two children is going to have the new key
                if (cmpEntry(keys[index], newKey) == -1) {
                    index++;
                }
            }

            // Validate child index again after potential split
            if (index >= 0 && index <= keyCount && children[index]) {
                children[index]->insertNonFull(newKey);
            }
        }
    }
};

// B-Tree wrapper class for hierarchical data organization
// Data Structure: B-Tree - balanced tree structure for efficient hierarchical storage
template<typename T>
class HTree {
public:
    HTNode<T>* root;
    int minDegree;

    // Constructor - creates an empty B-tree with specified minimum degree
    HTree(int degree = 3) {
        root = nullptr;
        minDegree = degree;
    }

    // Destructor - recursively deletes all nodes
    ~HTree() {
        clear(root);
    }

    // Recursively clears all nodes in the tree
    // Data Structure: B-Tree - recursive deletion
    void clear(HTNode<T>* node) {
        if (!node) {
            return;
        }
        if (!node->leaf) {
            for (int i = 0; i <= node->keyCount; i++) {
                clear(node->children[i]);
            }
        }
        delete node;
    }

    // Inserts a key into the B-tree
    // Data Structure: B-Tree - maintains balance during insertion
    void insert(const T& newKey) {
        if (minDegree < 2) {
            cout << "[ERROR] B-tree minimum degree must be at least 2" << endl;
            return;
        }

        if (!root) {
            root = new HTNode<T>(minDegree, true);
            root->keys[0] = newKey;
            root->keyCount = 1;
            return;
        }

        // If root is full, tree grows in height
        if (root->keyCount == 2 * minDegree - 1) {
            HTNode<T>* newRoot = new HTNode<T>(minDegree, false);
            newRoot->children[0] = root;
            newRoot->splitChild(0, root);
            
            // Determine which child will have the new key
            int childIndex = (cmpEntry(newRoot->keys[0], newKey) == -1 ? 1 : 0);
            if (childIndex >= 0 && childIndex <= newRoot->keyCount && newRoot->children[childIndex]) {
                newRoot->children[childIndex]->insertNonFull(newKey);
            }
            root = newRoot;
        }
        else {
            root->insertNonFull(newKey);
        }
    }

    // Prints the B-tree in level order with improved visualization
    // Data Structure: B-Tree - level-order traversal using queue
    void levelOrder() {
        if (!root) {
            cout << endl << "[B-TREE EMPTY]" << endl;
            return;
        }

        const string RESET = "\033[0m";
        const string YELLOW = "\033[33m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string CYAN = "\033[36m";
        const string GREEN = "\033[32m";
        const string WHITE = "\033[37m";

        cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
        cout << BRIGHT_YELLOW << "         B-TREE STRUCTURE" << RESET << endl;
        cout << BRIGHT_YELLOW << "========================================" << RESET << endl << endl;

        LinkedList<HTNode<T>*> currentLevel;
        LinkedList<HTNode<T>*> nextLevel;
        currentLevel.insertBack(root);

        int level = 0;
        int totalNodes = 0;
        int totalKeys = 0;

        while (currentLevel.getHead()) {
            level++;
            cout << YELLOW << "Level " << level << ":" << RESET << endl;
            cout << "----------------------------------------" << endl;

            LListNode<HTNode<T>*>* nodePtr = currentLevel.getHead();
            int nodesInLevel = 0;

            while (nodePtr) {
                HTNode<T>* currentNode = nodePtr->data;
                nodesInLevel++;
                totalNodes++;

                // Display node type
                cout << "  Node " << nodesInLevel << " (" 
                    << (currentNode->leaf ? GREEN + "LEAF" : CYAN + "INTERNAL") 
                    << RESET << ", " << currentNode->keyCount << " keys):" << endl;

                // Display keys with their level information
                cout << "    Keys: [ ";
                for (int i = 0; i < currentNode->keyCount; i++) {
                    totalKeys++;
                    HierarchyEntry entry = currentNode->keys[i];
                    
                    // Color code by level
                    string levelColor;
                    string levelLabel;
                    switch (entry.level) {
                    case 1:
                        levelColor = BRIGHT_YELLOW;
                        levelLabel = "SECTOR";
                        break;
                    case 2:
                        levelColor = CYAN;
                        levelLabel = "MODULE";
                        break;
                    case 3:
                        levelColor = GREEN;
                        levelLabel = "NODE";
                        break;
                    default:
                        levelColor = WHITE;
                        levelLabel = "UNKNOWN";
                    }

                    cout << levelColor << entry.key << RESET;
                    if (entry.level == 1) {
                        cout << " (" << levelLabel << ": " << entry.sector << ")";
                    }
                    else if (entry.level == 2) {
                        cout << " (" << levelLabel << ": " << moduleName(entry.module) << ")";
                    }
                    else if (entry.level == 3) {
                        cout << " (" << levelLabel << ": " << entry.nodeID << ")";
                    }

                    if (i < currentNode->keyCount - 1) {
                        cout << " | ";
                    }
                }
                cout << " ]" << endl;

                // Add children to next level
                if (!currentNode->leaf) {
                    for (int i = 0; i <= currentNode->keyCount; i++) {
                        if (currentNode->children[i]) {
                            nextLevel.insertBack(currentNode->children[i]);
                        }
                    }
                }

                nodePtr = nodePtr->next;
                if (nodePtr) {
                    cout << endl;
                }
            }

            cout << endl;

            // Move to next level
            currentLevel = nextLevel;
            nextLevel = LinkedList<HTNode<T>*>();
        }

        cout << "========================================" << endl;
        cout << "B-TREE STATISTICS:" << endl;
        cout << "  Total Levels: " << level << endl;
        cout << "  Total Nodes: " << totalNodes << endl;
        cout << "  Total Keys: " << totalKeys << endl;
        cout << "  Minimum Degree: " << root->minDegree << endl;
        cout << "  Max Keys per Node: " << (2 * root->minDegree - 1) << endl;
        cout << "========================================" << endl << endl;
    }
};

// Returns the human-readable name for a module type
string moduleName(ModuleType moduleType) {
    switch (moduleType) {
    case MODULE_TRANSPORT:
        return "TRANSPORT";
    case MODULE_EDUCATION:
        return "EDUCATION";
    case MODULE_MEDICAL:
        return "MEDICAL";
    case MODULE_COMMERCIAL:
        return "COMMERCIAL";
    case MODULE_PUBLIC_FACILITY:
        return "PUBLIC FACILITY";
    case MODULE_HOUSING:
        return "HOUSING";
    default:
        return "UNKNOWN";
    }
}

// Builds the hierarchical structure from city graph nodes
// Data Structure: B-Tree (hierarchical storage) + HashTable (duplicate tracking)
void buildHierarchy(Graph<string>& graph,
    HashTable<CityNode>& nodes,
    SectorCoordinateSystem& sectors,
    HTree<HierarchyEntry>& tree)
{
    // Track duplicates to avoid inserting same sector/module multiple times
    struct Flag {
        string key;
    };
    
    class FlagSet {
        LinkedList<Flag> buckets[101];
        
        int hash(const string& key) {
            unsigned long hashValue = 5381;
            for (char c : key) {
                hashValue = ((hashValue << 5) + hashValue) + (unsigned char)c;
            }
            return (int)(hashValue % 101);
        }
        
    public:
        bool has(const string& key) {
            int index = hash(key);
            LListNode<Flag>* node = buckets[index].getHead();
            while (node) {
                if (node->data.key == key) {
                    return true;
                }
                node = node->next;
            }
            return false;
        }
        
        void add(const string& key) {
            if (has(key)) {
                return;
            }
            buckets[hash(key)].insertBack({ key });
        }
    } usedFlags;

    int nodeCount = 0;
    int sectorCount = 0;
    int moduleCount = 0;

    // STEP 1: Insert ALL sectors from SectorCoordinateSystem first
    // This ensures hierarchy is synced with all defined sectors
    LinkedList<string> allSectors = sectors.getAllSectorNames();
    LListNode<string>* sectorNode = allSectors.getHead();
    while (sectorNode) {
        string sectorName = sectorNode->data;
        string sectorKey = "SEC::" + sectorName;
        if (!usedFlags.has(sectorKey)) {
            tree.insert(HierarchyEntry(sectorName));
            usedFlags.add(sectorKey);
            sectorCount++;
        }
        sectorNode = sectorNode->next;
    }

    // STEP 2: Process nodes from graph and add modules/nodes
    VertexNode<string>* vertex = graph.getHeadVertex();
    if (vertex) {
        while (vertex) {
            CityNode* cityNode = nodes.search(vertex->id);
            if (cityNode) {
                // Get real sector from coordinates
                string sector = sectors.getSectorFromCoordinates(
                    cityNode->coords.latitude,
                    cityNode->coords.longitude
                );
                if (sector.empty() || sector == "Unknown") {
                    sector = "UNKNOWN";
                    // Add UNKNOWN sector if not already added
                    string sectorKey = "SEC::" + sector;
                    if (!usedFlags.has(sectorKey)) {
                        tree.insert(HierarchyEntry(sector));
                        usedFlags.add(sectorKey);
                        sectorCount++;
                    }
                }

                // Insert Module (Level 2) if not already inserted
                string moduleKey = "MOD::" + sector + "::" + to_string((int)cityNode->moduleType);
                if (!usedFlags.has(moduleKey)) {
                    tree.insert(HierarchyEntry(sector, cityNode->moduleType));
                    usedFlags.add(moduleKey);
                    moduleCount++;
                }

                // Insert Node (Level 3)
                tree.insert(HierarchyEntry(
                    sector,
                    cityNode->moduleType,
                    cityNode->nodeID,
                    cityNode->name,
                    cityNode->coords
                ));
                nodeCount++;
            }

            vertex = vertex->next;
        }
    }

    cout << "[SUCCESS] Hierarchy built: " << sectorCount << " sectors, "
        << moduleCount << " modules, " << nodeCount << " nodes" << endl;
    cout << "[INFO] All sectors from SectorCoordinateSystem are included in hierarchy" << endl;
}

// Prints the hierarchy in a structured format with sector and module bundling
// Data Structure: B-Tree (level-order traversal) + LinkedList (flattening)
void printHierarchy(HTree<HierarchyEntry>& tree) {
    if (!tree.root) {
        cout << endl << "[HIERARCHY EMPTY]" << endl;
        return;
    }

    // Flatten the B-tree into a list
    LinkedList<HTNode<HierarchyEntry>*> queue;
    LinkedList<HierarchyEntry> flatList;

    queue.insertBack(tree.root);

    while (queue.getHead()) {
        HTNode<HierarchyEntry>* currentNode = queue.getHead()->data;
        queue.removeByValue(currentNode);

        for (int i = 0; i < currentNode->keyCount; i++) {
            flatList.insertBack(currentNode->keys[i]);
        }

        if (!currentNode->leaf) {
            for (int j = 0; j <= currentNode->keyCount; j++) {
                if (currentNode->children[j]) {
                    queue.insertBack(currentNode->children[j]);
                }
            }
        }
    }

    // Group entries by sector and module for better display
    struct ModuleGroup {
        ModuleType moduleType;
        LinkedList<HierarchyEntry> nodes;
    };
    
    struct SectorGroup {
        string sectorName;
        LinkedList<ModuleGroup> modules;
    };
    
    LinkedList<SectorGroup> sectors;
    
    // First pass: collect all entries and organize by sector and module
    LListNode<HierarchyEntry>* entryNode = flatList.getHead();
    while (entryNode) {
        HierarchyEntry entry = entryNode->data;
        
        if (entry.level == 1) {
            // Sector entry - create new sector group
            SectorGroup newSector;
            newSector.sectorName = entry.sector;
            sectors.insertBack(newSector);
        }
        else if (entry.level == 2) {
            // Module entry - find the sector and add module
            LListNode<SectorGroup>* sectorNode = sectors.getHead();
            while (sectorNode) {
                if (sectorNode->data.sectorName == entry.sector) {
                    ModuleGroup newModule;
                    newModule.moduleType = entry.module;
                    sectorNode->data.modules.insertBack(newModule);
                    break;
                }
                sectorNode = sectorNode->next;
            }
        }
        else if (entry.level == 3) {
            // Node entry - find sector and module, add node
            LListNode<SectorGroup>* sectorNode = sectors.getHead();
            while (sectorNode) {
                if (sectorNode->data.sectorName == entry.sector) {
                    LListNode<ModuleGroup>* moduleNode = sectorNode->data.modules.getHead();
                    while (moduleNode) {
                        if (moduleNode->data.moduleType == entry.module) {
                            moduleNode->data.nodes.insertBack(entry);
                            break;
                        }
                        moduleNode = moduleNode->next;
                    }
                    break;
                }
                sectorNode = sectorNode->next;
            }
        }
        
        entryNode = entryNode->next;
    }

    // Display the bundled hierarchy
    cout << endl << "=========== ISLAMABAD HIERARCHY ===========" << endl;
    
    LListNode<SectorGroup>* sectorNode = sectors.getHead();
    int totalSectors = 0;
    int totalModules = 0;
    int totalNodes = 0;
    
    while (sectorNode) {
        totalSectors++;
        cout << endl << "SECTOR: " << sectorNode->data.sectorName << endl;
        cout << "----------------------------------------" << endl;
        
        LListNode<ModuleGroup>* moduleNode = sectorNode->data.modules.getHead();
        while (moduleNode) {
            totalModules++;
            cout << "  MODULE: " << moduleName(moduleNode->data.moduleType) << endl;
            
            int nodeCount = 0;
            LListNode<HierarchyEntry>* nodeEntry = moduleNode->data.nodes.getHead();
            while (nodeEntry) {
                nodeCount++;
                totalNodes++;
                HierarchyEntry node = nodeEntry->data;
                cout << "    [" << nodeCount << "] " << node.nodeID 
                    << " - " << node.nodeName << endl;
                cout << "        Coordinates: (" << node.coords.latitude 
                    << ", " << node.coords.longitude << ")" << endl;
                nodeEntry = nodeEntry->next;
            }
            
            if (nodeCount == 0) {
                cout << "    (No nodes in this module)" << endl;
            }
            else {
                cout << "    Total nodes: " << nodeCount << endl;
            }
            
            moduleNode = moduleNode->next;
            if (moduleNode) {
                cout << endl;
            }
        }
        
        sectorNode = sectorNode->next;
        if (sectorNode) {
            cout << endl;
        }
    }
    
    cout << endl << "===========================================" << endl;
    cout << "SUMMARY:" << endl;
    cout << "  Total Sectors: " << totalSectors << endl;
    cout << "  Total Modules: " << totalModules << endl;
    cout << "  Total Nodes: " << totalNodes << endl;
    cout << "===========================================" << endl;
}

// Main menu for Islamabad Hierarchy and B-Tree operations
void hierarchyMenu(Graph<string>& graph,
    HashTable<CityNode>& nodes,
    SectorCoordinateSystem& sectors)
{
    // ANSI color codes
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string YELLOW = "\033[33m";
    const string BRIGHT_YELLOW = "\033[93m";
    const string WHITE = "\033[37m";
    const string BRIGHT_WHITE = "\033[97m";
    const string CYAN = "\033[36m";
    const string BRIGHT_RED = "\033[91m";
    const string GREEN = "\033[32m";

    HTree<HierarchyEntry> tree(3);

    while (true) {
        cout << endl << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "    ISLAMABAD HIERARCHY & B-TREE" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl << endl;

        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "  HIERARCHY OPERATIONS" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_WHITE << "  1. Build Hierarchy (from City Graph)" << RESET << endl;
        cout << BRIGHT_WHITE << "  2. Display Hierarchy (Sector -> Module -> Node)" << RESET << endl << endl;

        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "  B-TREE OPERATIONS" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_WHITE << "  3. Display Raw B-Tree Structure" << RESET << endl << endl;

        cout << BRIGHT_WHITE << BOLD << "  0. Back to Main Menu" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "Choose: " << RESET;

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) {
            break;
        }

        switch (choice) {
        case 1:
            cout << endl << CYAN << "[INFO] Building hierarchy from city graph..." << RESET << endl;
            buildHierarchy(graph, nodes, sectors, tree);
            cout << GREEN << "[SUCCESS] Hierarchy built successfully!" << RESET << endl;
            break;

        case 2:
            printHierarchy(tree);
            break;

        case 3:
            cout << endl << CYAN << "[B-TREE STRUCTURE]" << RESET << endl;
            cout << "Level-order traversal:" << endl;
            tree.levelOrder();
            break;

        default:
            cout << BRIGHT_RED << "[ERROR] Invalid option. Please try again." << RESET << endl;
        }

        if (choice != 0) {
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
        }
    }
}
