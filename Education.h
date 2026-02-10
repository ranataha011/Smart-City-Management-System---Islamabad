#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

#include "Data_Structures.h"
#include "MainCityGraph.h"

using namespace std;

// =======================================================================
// TYPES
// =======================================================================
struct SchoolInfo {
    string schoolID;
    string schoolName;
    string sector;
    double rating;
    
    SchoolInfo() : rating(0.0) {}
};

struct Faculty {
    string facultyID;
    string name;
    string qualification;
    string schoolID;
    string department;
};

struct Student {
    string studentID;
    string name;
    int age;
    string schoolID;
    string department;
    string className;
    
    Student() : age(0) {}
};

// =======================================================================
// Simple max-heap for ranking (array-based, no vector)
// =======================================================================
struct HeapItem {
    double key;
    string schoolID;
    
    HeapItem() : key(0.0) {}
    HeapItem(double k, const string& id) : key(k), schoolID(id) {}
};

// Max-heap data structure for ranking schools by rating
// Data Structure: Array-based Max-Heap - O(log n) insert/extract operations
class MaxHeap {
private:
    HeapItem* items;
    int capacity;
    int size;

    // Expands the heap array when full
    void ensureCapacity() {
        if (size < capacity) {
            return;
        }
        int newCap = (capacity == 0) ? 8 : capacity * 2;
        HeapItem* newItems = new HeapItem[newCap];
        for (int i = 0; i < size; ++i) {
            newItems[i] = items[i];
        }
        delete[] items;
        items = newItems;
        capacity = newCap;
    }

    // Swaps two heap items at given indices
    void swapItems(int idx1, int idx2) {
        HeapItem temp = items[idx1];
        items[idx1] = items[idx2];
        items[idx2] = temp;
    }

    // Moves an item up the heap to maintain max-heap property
    void siftUp(int idx) {
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (items[parent].key >= items[idx].key) {
                break;
            }
            swapItems(parent, idx);
            idx = parent;
        }
    }

    // Moves an item down the heap to maintain max-heap property
    void siftDown(int idx) {
        while (true) {
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;
            int largest = idx;
            
            if (left < size && items[left].key > items[largest].key) {
                largest = left;
            }
            if (right < size && items[right].key > items[largest].key) {
                largest = right;
            }
            if (largest == idx) {
                break;
            }
            swapItems(idx, largest);
            idx = largest;
        }
    }

public:
    MaxHeap() : items(nullptr), capacity(0), size(0) {}
    ~MaxHeap() { 
        delete[] items; 
    }

    // Adds a new item to the heap
    void push(double key, const string& id) {
        ensureCapacity();
        items[size].key = key;
        items[size].schoolID = id;
        size++;
        siftUp(size - 1);
    }

    // Checks if the heap is empty
    bool empty() const { 
        return size == 0; 
    }

    // Returns the top (maximum) item without removing it
    HeapItem top() const { 
        return (size > 0 ? items[0] : HeapItem{ 0.0, "" }); 
    }

    // Removes the top (maximum) item from the heap
    void pop() { 
        if (size == 0) {
            return;
        }
        items[0] = items[size - 1];
        size--;
        siftDown(0);
    }

    // Returns the number of items in the heap
    int getSize() const { 
        return size; 
    }
};

// =======================================================================
// EducationModule
// =======================================================================
class EducationModule {
private:
    MainCityGraph* mainGraph;

    // Core storage
    HashTable<SchoolInfo> schoolHash;           // schoolID -> SchoolInfo
    LinkedList<NaryTree<string>*> schoolForest; // each tree root = "schoolID | schoolName"

    // Faculty / Student
    HashTable<Faculty> facultyHash;             // facultyID -> Faculty
    HashTable<Student> studentHash;             // studentID -> Student

    // Mappings: deptKey -> LinkedList<facultyID>, classKey -> LinkedList<studentID>
    HashTable< LinkedList<string> > deptToFaculty;
    HashTable< LinkedList<string> > classToStudents;

    // Extracts school ID from tree root string format "ID | Name"
    // Data Structure: String manipulation - O(n) parsing
    static string extractSchoolIDFromRoot(const string& root) {
        size_t separator = root.find(" | ");
        if (separator == string::npos) {
            separator = root.find('|');
            if (separator == string::npos) {
                return trimStatic(root);
            }
            return trimStatic(root.substr(0, separator));
        }
        return trimStatic(root.substr(0, separator));
    }

    // Removes leading and trailing whitespace from a string
    static string trimStatic(string str) {
        int start = 0;
        int end = (int)str.length() - 1;
        
        while (start <= end && isspace((unsigned char)str[start])) {
            ++start;
        }
        while (end >= start && isspace((unsigned char)str[end])) {
            --end;
        }
        
        if (start > end) {
            return string("");
        }
        return str.substr(start, end - start + 1);
    }

public:
    EducationModule(MainCityGraph* g)
        : mainGraph(g),
        schoolHash(401),
        facultyHash(1009),
        studentHash(2003),
        deptToFaculty(401),
        classToStudents(2003)
    {
    }

    // Removes leading and trailing whitespace from a string
    // Data Structure: String manipulation
    static string trimCopy(string str) {
        int start = 0;
        int end = (int)str.length() - 1;
        
        while (start <= end && isspace((unsigned char)str[start])) {
            ++start;
        }
        while (end >= start && isspace((unsigned char)str[end])) {
            --end;
        }
        
        if (start > end) {
            return string("");
        }
        return str.substr(start, end - start + 1);
    }

    // Trims a string in place
    static void trimInPlace(string& str) { 
        str = trimCopy(str); 
    }

    // Parses a CSV line into fields, handling quoted values
    // Data Structure: String parsing - O(n) where n is line length
    static void parseCSVLine(const string& line, string fields[], int expected) {
        int fieldIdx = 0;
        bool inQuotes = false;
        string current = "";
        
        for (int i = 0; i < (int)line.length(); ++i) {
            char c = line[i];
            if (c == '"') {
                inQuotes = !inQuotes;
                continue;
            }
            if (c == ',' && !inQuotes) {
                if (fieldIdx < expected) {
                    fields[fieldIdx++] = current;
                }
                current = "";
            }
            else {
                current += c;
            }
        }
        if (fieldIdx < expected) {
            fields[fieldIdx] = current;
        }
    }

    // Splits a string by delimiter into a linked list
    // Data Structure: LinkedList - O(n) where n is string length
    LinkedList<string> splitToList(const string& text, char delimiter) {
        LinkedList<string> result;
        string temp = "";
        
        for (int i = 0; i < (int)text.length(); ++i) {
            char c = text[i];
            if (c == delimiter) {
                string trimmed = trimCopy(temp);
                if (trimmed != "") {
                    result.insertBack(trimmed);
                }
                temp = "";
            }
            else {
                temp += c;
            }
        }
        string trimmed = trimCopy(temp);
        if (trimmed != "") {
            result.insertBack(trimmed);
        }
        return result;
    }

    // Checks if a value exists in a linked list
    // Data Structure: LinkedList - O(n) linear search
    bool containsInList(LinkedList<string>& list, const string& value) {
        LListNode<string>* node = list.getHead();
        while (node) {
            if (node->data == value) {
                return true;
            }
            node = node->next;
        }
        return false;
    }

    // Creates a composite key for department lookup
    string makeDeptKey(const string& schoolID, const string& dept) const {
        return schoolID + "|" + dept;
    }

    // Creates a composite key for class lookup
    string makeClassKey(const string& schoolID, const string& dept, const string& className) const {
        return schoolID + "|" + dept + "|" + className;
    }

    // Finds the N-ary tree corresponding to a school ID
    // Data Structure: LinkedList (traversal) + N-ary Tree (lookup)
    NaryTree<string>* findSchoolTree(const string& schoolID) {
        LListNode<NaryTree<string>*>* node = schoolForest.getHead();
        while (node) {
            string root = node->data->root->data;
            string id = extractSchoolIDFromRoot(root);
            if (id == schoolID) {
                return node->data;
            }
            node = node->next;
        }
        return nullptr;
    }

    // Adds a new school to the system
    // Data Structure: HashTable (O(1) lookup) + LinkedList (stores trees) + N-ary Tree (hierarchy)
    void addSchool(const string& schoolID, const string& schoolName, const string& sector, double rating) {
        if (schoolID.empty() || schoolName.empty() || sector.empty()) {
            cout << "[ERROR] School ID, Name, and Sector cannot be empty" << endl;
            return;
        }
        
        if (rating < 0.0 || rating > 10.0) {
            cout << "[ERROR] Rating must be between 0.0 and 10.0" << endl;
            return;
        }
        
        if (schoolHash.search(schoolID)) {
            cout << "[INFO] School already exists: " << schoolID << endl;
            return;
        }
        
        SchoolInfo school;
        school.schoolID = schoolID;
        school.schoolName = schoolName;
        school.sector = sector;
        school.rating = rating;
        schoolHash.insert(schoolID, school);

        NaryTree<string>* tree = new NaryTree<string>(schoolID + " | " + schoolName);
        schoolForest.insertBack(tree);

        // Add to MainCityGraph - try sector coordinates, fallback to addNodeInSector
        double lat = 0.0;
        double lon = 0.0;
        if (mainGraph && mainGraph->getSectorSystem().getCoordinatesInSector(sector, lat, lon)) {
            mainGraph->addNodeWithAutoConnect("E_" + schoolID, MODULE_EDUCATION, schoolName, lat, lon, sector, nullptr);
        }
        else if (mainGraph) {
            // If sector not found, still create node in-sector attempt will show error there
            mainGraph->addNodeInSector("E_" + schoolID, MODULE_EDUCATION, schoolName, sector, nullptr);
        }

        cout << "[SUCCESS] School added: " << schoolID << " - " << schoolName << endl;
    }

    // Adds a new department to a school
    // Data Structure: N-ary Tree (adds child node to school tree)
    void addDepartment(const string& schoolID, const string& deptName) {
        if (schoolID.empty() || deptName.empty()) {
            cout << "[ERROR] School ID and Department name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        if (!tree) {
            cout << "[ERROR] School not found: " << schoolID << endl;
            return;
        }
        
        // Ensure department not already present under root
        TreeNode<string>* existing = tree->findNode(tree->root, deptName);
        if (existing && existing->parent == tree->root) {
            cout << "[INFO] Department already exists: " << deptName << endl;
            return;
        }
        
        tree->addChild(tree->root, deptName);
        cout << "[SUCCESS] Department added: " << deptName << " in " << schoolID << endl;
    }

    // Adds a new class to a department
    // Data Structure: N-ary Tree (adds child node to department tree)
    void addClass(const string& schoolID, const string& deptName, const string& className) {
        if (schoolID.empty() || deptName.empty() || className.empty()) {
            cout << "[ERROR] School ID, Department name, and Class name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        if (!tree) {
            cout << "[ERROR] School not found" << endl;
            return;
        }
        
        TreeNode<string>* dept = tree->findNode(tree->root, deptName);
        if (!dept) {
            cout << "[ERROR] Department not found" << endl;
            return;
        }
        
        TreeNode<string>* existing = tree->findNode(dept, className);
        if (existing && existing->parent == dept) {
            cout << "[INFO] Class already exists" << endl;
            return;
        }
        
        tree->addChild(dept, className);
        cout << "[SUCCESS] Class " << className << " added under " << deptName << endl;
    }

    // Registers a new faculty member in a department
    // Data Structure: HashTable (O(1) faculty lookup) + HashTable (department mapping) + LinkedList (faculty lists)
    void registerFaculty(const string& facultyID, const string& name, const string& qualification,
        const string& schoolID, const string& deptName) {
        if (facultyID.empty() || name.empty() || schoolID.empty() || deptName.empty()) {
            cout << "[ERROR] Faculty ID, Name, School ID, and Department cannot be empty" << endl;
            return;
        }
        
        if (facultyHash.search(facultyID)) {
            cout << "[INFO] Faculty exists: " << facultyID << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        if (!tree) {
            cout << "[ERROR] School not found" << endl;
            return;
        }
        
        TreeNode<string>* dept = tree->findNode(tree->root, deptName);
        if (!dept) {
            cout << "[ERROR] Department not found" << endl;
            return;
        }

        Faculty faculty;
        faculty.facultyID = facultyID;
        faculty.name = name;
        faculty.qualification = qualification;
        faculty.schoolID = schoolID;
        faculty.department = deptName;
        facultyHash.insert(facultyID, faculty);

        string deptKey = makeDeptKey(schoolID, deptName);
        LinkedList<string>* facultyList = deptToFaculty.search(deptKey);
        if (!facultyList) {
            LinkedList<string> newList;
            newList.insertBack(facultyID);
            deptToFaculty.insert(deptKey, newList);
        }
        else {
            if (!containsInList(*facultyList, facultyID)) {
                facultyList->insertBack(facultyID);
            }
        }

        cout << "[SUCCESS] Faculty registered: " << facultyID << " (" << name << ")" << endl;
    }

    // Lists all faculty members in a department
    // Data Structure: HashTable (department lookup) + LinkedList (faculty list traversal) + HashTable (faculty info)
    void listFacultyOfDepartment(const string& schoolID, const string& deptName) {
        if (schoolID.empty() || deptName.empty()) {
            cout << "[ERROR] School ID and Department name cannot be empty" << endl;
            return;
        }
        
        string deptKey = makeDeptKey(schoolID, deptName);
        LinkedList<string>* facultyList = deptToFaculty.search(deptKey);
        if (!facultyList) {
            cout << "[INFO] No faculty for this department" << endl;
            return;
        }
        
        cout << endl << "Faculty in " << deptName << " of " << schoolID << ":" << endl;
        LListNode<string>* node = facultyList->getHead();
        while (node) {
            Faculty* faculty = facultyHash.search(node->data);
            if (faculty) {
                cout << faculty->facultyID << " | " << faculty->name << " | " << faculty->qualification << endl;
            }
            node = node->next;
        }
    }

    // Deletes a faculty member from the system
    // Data Structure: HashTable (removes from all mappings) + LinkedList (removes from department list)
    void deleteFaculty(const string& facultyID) {
        if (facultyID.empty()) {
            cout << "[ERROR] Faculty ID cannot be empty" << endl;
            return;
        }
        
        Faculty* faculty = facultyHash.search(facultyID);
        if (!faculty) {
            cout << "[ERROR] Faculty not found" << endl;
            return;
        }
        
        string deptKey = makeDeptKey(faculty->schoolID, faculty->department);
        LinkedList<string>* facultyList = deptToFaculty.search(deptKey);
        if (facultyList) {
            facultyList->removeByValue(facultyID);
            // If list becomes empty, remove mapping
            if (!facultyList->getHead()) {
                deptToFaculty.remove(deptKey);
            }
        }
        facultyHash.remove(facultyID);
        cout << "[SUCCESS] Faculty removed: " << facultyID << endl;
    }

    // Registers a new student in a class
    // Data Structure: HashTable (O(1) student lookup) + HashTable (class mapping) + LinkedList (student lists)
    void registerStudent(const string& studentID, const string& name, int age,
        const string& schoolID, const string& deptName, const string& className) {
        if (studentID.empty() || name.empty() || schoolID.empty() || deptName.empty() || className.empty()) {
            cout << "[ERROR] Student ID, Name, School ID, Department, and Class cannot be empty" << endl;
            return;
        }
        
        if (age < 0 || age > 150) {
            cout << "[ERROR] Invalid age: " << age << ". Age must be between 0 and 150" << endl;
            return;
        }
        
        if (studentHash.search(studentID)) {
            cout << "[INFO] Student exists" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        if (!tree) {
            cout << "[ERROR] School not found" << endl;
            return;
        }
        
        TreeNode<string>* dept = tree->findNode(tree->root, deptName);
        if (!dept) {
            cout << "[ERROR] Department not found" << endl;
            return;
        }
        
        TreeNode<string>* classNode = tree->findNode(dept, className);
        if (!classNode) {
            cout << "[ERROR] Class not found" << endl;
            return;
        }

        Student student;
        student.studentID = studentID;
        student.name = name;
        student.age = age;
        student.schoolID = schoolID;
        student.department = deptName;
        student.className = className;
        studentHash.insert(studentID, student);

        string classKey = makeClassKey(schoolID, deptName, className);
        LinkedList<string>* studentList = classToStudents.search(classKey);
        if (!studentList) {
            LinkedList<string> newList;
            newList.insertBack(studentID);
            classToStudents.insert(classKey, newList);
        }
        else {
            if (!containsInList(*studentList, studentID)) {
                studentList->insertBack(studentID);
            }
        }

        cout << "[SUCCESS] Student registered: " << studentID << " (" << name << ")" << endl;
    }

    // Lists all students in a class
    // Data Structure: HashTable (class lookup) + LinkedList (student list traversal) + HashTable (student info)
    void listStudentsInClass(const string& schoolID, const string& deptName, const string& className) {
        if (schoolID.empty() || deptName.empty() || className.empty()) {
            cout << "[ERROR] School ID, Department name, and Class name cannot be empty" << endl;
            return;
        }
        
        string classKey = makeClassKey(schoolID, deptName, className);
        LinkedList<string>* studentList = classToStudents.search(classKey);
        if (!studentList) {
            cout << "[INFO] No students in this class" << endl;
            return;
        }
        
        cout << endl << "Students in " << className << " (" << deptName << " - " << schoolID << "):" << endl;
        LListNode<string>* node = studentList->getHead();
        while (node) {
            Student* student = studentHash.search(node->data);
            if (student) {
                cout << student->studentID << " | " << student->name << " | Age: " << student->age << endl;
            }
            node = node->next;
        }
    }

    // Deletes a student from the system
    // Data Structure: HashTable (removes from all mappings) + LinkedList (removes from class list)
    void deleteStudent(const string& studentID) {
        if (studentID.empty()) {
            cout << "[ERROR] Student ID cannot be empty" << endl;
            return;
        }
        
        Student* student = studentHash.search(studentID);
        if (!student) {
            cout << "[ERROR] Student not found" << endl;
            return;
        }
        
        string classKey = makeClassKey(student->schoolID, student->department, student->className);
        LinkedList<string>* studentList = classToStudents.search(classKey);
        if (studentList) {
            studentList->removeByValue(studentID);
            if (!studentList->getHead()) {
                classToStudents.remove(classKey);
            }
        }
        studentHash.remove(studentID);
        cout << "[SUCCESS] Student removed: " << studentID << endl;
    }

    // Searches for schools that offer a specific class or subject
    // Data Structure: LinkedList (school traversal) + N-ary Tree (class search) + HashTable (school info)
    void findSchoolsOfferingClass(const string& className) {
        if (className.empty()) {
            cout << "[ERROR] Class name cannot be empty" << endl;
            return;
        }
        
        cout << endl << "=== Schools offering subject/class: " << className << " ===" << endl;
        bool found = false;
        LListNode<NaryTree<string>*>* node = schoolForest.getHead();
        
        while (node) {
            string root = node->data->root->data;
            string schoolID = extractSchoolIDFromRoot(root);
            TreeNode<string>* classNode = node->data->findNode(node->data->root, className);
            
            if (classNode) {
                found = true;
                SchoolInfo* school = schoolHash.search(schoolID);
                if (school) {
                    cout << " - " << school->schoolName << " (" << schoolID << ") Sector: " 
                         << school->sector << " Rating: " << school->rating << endl;
                }
            }
            node = node->next;
        }
        
        if (!found) {
            cout << "[INFO] No schools offer this class/subject" << endl;
        }
    }

    // Ranks schools by rating and displays the top K schools
    // Data Structure: MaxHeap (O(n log n) heap operations) + HashTable (school lookup)
    void rankSchoolsAndShowTop(int K) {
        if (K <= 0) {
            cout << "[ERROR] K must be greater than 0" << endl;
            return;
        }
        
        MaxHeap heap;
        LListNode<NaryTree<string>*>* node = schoolForest.getHead();
        
        while (node) {
            string root = node->data->root->data;
            string schoolID = extractSchoolIDFromRoot(root);
            SchoolInfo* school = schoolHash.search(schoolID);
            if (school) {
                heap.push(school->rating, school->schoolID);
            }
            node = node->next;
        }

        cout << endl << "=== Top " << K << " Schools by rating ===" << endl;
        int shown = 0;
        while (!heap.empty() && shown < K) {
            HeapItem item = heap.top();
            heap.pop();
            SchoolInfo* school = schoolHash.search(item.schoolID);
            if (school) {
                cout << ++shown << ". " << school->schoolName << " (" << school->schoolID 
                     << ") Rating: " << school->rating << " Sector: " << school->sector << endl;
            }
        }
        
        if (shown == 0) {
            cout << "[INFO] No schools to rank" << endl;
        }
    }

    // Locates the nearest school to a given node using the city graph
    // Data Structure: Graph (shortest path algorithms)
    void locateNearestSchool(const string& currentNodeID) {
        if (currentNodeID.empty()) {
            cout << "[ERROR] Current node ID cannot be empty" << endl;
            return;
        }
        
        if (!mainGraph) {
            cout << "[ERROR] Main city graph not available" << endl;
            return;
        }
        
        cout << "[INFO] Searching nearest school to " << currentNodeID << " using city graph..." << endl;
        mainGraph->findNearestNodeByType(currentNodeID, MODULE_EDUCATION);
    }

    // Recursively deletes a subtree starting from the given node
    // Data Structure: N-ary Tree - recursive deletion of all descendants
    void deleteSubtree(TreeNode<string>* node) {
        if (!node) {
            return;
        }
        
        TreeNode<string>* child = node->firstChild;
        while (child) {
            TreeNode<string>* next = child->nextSibling;
            deleteSubtree(child);
            child = next;
        }
        delete node;
    }

    // Removes a child node from its parent's children list
    // Data Structure: N-ary Tree - maintains sibling links for efficient removal
    TreeNode<string>* removeChildNode(TreeNode<string>* parent, const string& childData) {
        if (!parent) {
            return nullptr;
        }
        
        TreeNode<string>* current = parent->firstChild;
        TreeNode<string>* previous = nullptr;
        
        while (current) {
            if (current->data == childData) {
                if (previous) {
                    previous->nextSibling = current->nextSibling;
                }
                else {
                    parent->firstChild = current->nextSibling;
                }
                current->nextSibling = nullptr;
                return current;
            }
            previous = current;
            current = current->nextSibling;
        }
        return nullptr;
    }

    // Deletes a class and all its students
    // Data Structure: N-ary Tree (removes from hierarchy) + HashTable (removes student mappings)
    void deleteClass(const string& schoolID, const string& deptName, const string& className) {
        if (schoolID.empty() || deptName.empty() || className.empty()) {
            cout << "[ERROR] School ID, Department name, and Class name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        if (!tree) {
            cout << "[ERROR] School not found" << endl;
            return;
        }
        
        TreeNode<string>* dept = tree->findNode(tree->root, deptName);
        if (!dept) {
            cout << "[ERROR] Department not found" << endl;
            return;
        }
        
        TreeNode<string>* removed = removeChildNode(dept, className);
        if (!removed) {
            cout << "[ERROR] Class not found" << endl;
            return;
        }

        string classKey = makeClassKey(schoolID, deptName, className);
        LinkedList<string>* students = classToStudents.search(classKey);
        if (students) {
            LListNode<string>* node = students->getHead();
            while (node) {
                studentHash.remove(node->data);
                node = node->next;
            }
            classToStudents.remove(classKey);
        }

        deleteSubtree(removed);
        cout << "[SUCCESS] Class deleted" << endl;
    }

    // Deletes a department and all its classes and associated faculty/students
    // Data Structure: N-ary Tree (hierarchical deletion) + HashTable (batch removal)
    void deleteDepartment(const string& schoolID, const string& deptName) {
        if (schoolID.empty() || deptName.empty()) {
            cout << "[ERROR] School ID and Department name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        if (!tree) {
            cout << "[ERROR] School not found" << endl;
            return;
        }
        
        TreeNode<string>* removed = removeChildNode(tree->root, deptName);
        if (!removed) {
            cout << "[ERROR] Department not found" << endl;
            return;
        }

        string deptKey = makeDeptKey(schoolID, deptName);
        LinkedList<string>* faculty = deptToFaculty.search(deptKey);
        if (faculty) {
            LListNode<string>* node = faculty->getHead();
            while (node) {
                facultyHash.remove(node->data);
                node = node->next;
            }
            deptToFaculty.remove(deptKey);
        }

        TreeNode<string>* classNode = removed->firstChild;
        while (classNode) {
            string classKey = makeClassKey(schoolID, deptName, classNode->data);
            LinkedList<string>* students = classToStudents.search(classKey);
            if (students) {
                LListNode<string>* node = students->getHead();
                while (node) {
                    studentHash.remove(node->data);
                    node = node->next;
                }
                classToStudents.remove(classKey);
            }
            classNode = classNode->nextSibling;
        }

        deleteSubtree(removed);
        cout << "[SUCCESS] Department deleted" << endl;
    }

    // Deletes a school and all its departments, classes, faculty, and students
    // Data Structure: LinkedList (removes school tree) + N-ary Tree (recursive deletion) + HashTable (cleanup)
    void deleteSchool(const string& schoolID) {
        if (schoolID.empty()) {
            cout << "[ERROR] School ID cannot be empty" << endl;
            return;
        }
        
        // Find the corresponding tree node
        LListNode<NaryTree<string>*>* node = schoolForest.getHead();
        NaryTree<string>* targetTree = nullptr;
        
        while (node) {
            string root = node->data->root->data;
            string id = extractSchoolIDFromRoot(root);
            if (id == schoolID) {
                targetTree = node->data;
                break;
            }
            node = node->next;
        }

        if (!targetTree) {
            cout << "[ERROR] School not found" << endl;
            return;
        }

        // Cleanup faculties and students under this school's departments and classes
        TreeNode<string>* dept = targetTree->root->firstChild;
        while (dept) {
            string deptName = dept->data;
            string deptKey = makeDeptKey(schoolID, deptName);
            LinkedList<string>* faculty = deptToFaculty.search(deptKey);
            
            if (faculty) {
                LListNode<string>* node = faculty->getHead();
                while (node) {
                    facultyHash.remove(node->data);
                    node = node->next;
                }
                deptToFaculty.remove(deptKey);
            }
            
            TreeNode<string>* classNode = dept->firstChild;
            while (classNode) {
                string classKey = makeClassKey(schoolID, deptName, classNode->data);
                LinkedList<string>* students = classToStudents.search(classKey);
                
                if (students) {
                    LListNode<string>* node = students->getHead();
                    while (node) {
                        studentHash.remove(node->data);
                        node = node->next;
                    }
                    classToStudents.remove(classKey);
                }
                classNode = classNode->nextSibling;
            }
            dept = dept->nextSibling;
        }

        // Delete the tree (NaryTree destructor will handle subtree)
        delete targetTree;

        // Remove the tree pointer from the schoolForest linked list
        schoolForest.removeByValue(targetTree);

        // Remove school info from hash
        schoolHash.remove(schoolID);

        cout << "[SUCCESS] School deleted completely" << endl;
    }

    // Loads schools from a CSV file (format: SchoolID,Name,Sector,Rating,Subjects)
    // Data Structure: File I/O + String parsing + LinkedList (subject splitting)
    void loadFromCSV(const string& filename) {
        if (filename.empty()) {
            cout << "[ERROR] Filename cannot be empty" << endl;
            return;
        }
        
        ifstream file(filename.c_str());
        if (!file.is_open()) {
            cout << "[ERROR] Could not open " << filename << endl;
            return;
        }

        string line;
        bool firstLine = true;
        int successCount = 0;
        int errorCount = 0;
        
        while (getline(file, line)) {
            if (firstLine) {
                firstLine = false;
                continue; // skip header
            }
            
            if (line.empty() || line.size() < 2) {
                continue;
            }

            string parts[5];
            parseCSVLine(line, parts, 5);

            for (int i = 0; i < 5; ++i) {
                parts[i] = trimCopy(parts[i]);
            }

            string schoolID = parts[0];
            string schoolName = parts[1];
            string sector = parts[2];
            string ratingStr = parts[3];
            string subjectsStr = parts[4];

            if (schoolID.empty() || schoolName.empty()) {
                errorCount++;
                continue;
            }

            double rating = 0.0;
            try {
                rating = stod(ratingStr);
                if (rating < 0.0 || rating > 10.0) {
                    rating = 0.0;
                }
            }
            catch (...) {
                rating = 0.0;
            }

            // Add school first
            addSchool(schoolID, schoolName, sector, rating);

            // Subjects -> departments & classes
            LinkedList<string> subjects = splitToList(subjectsStr, ',');
            LListNode<string>* subjectNode = subjects.getHead();
            
            while (subjectNode) {
                string subject = trimCopy(subjectNode->data);
                if (!subject.empty()) {
                    addDepartment(schoolID, subject); // subject as department
                    addClass(schoolID, subject, subject); // subject as class
                }
                subjectNode = subjectNode->next;
            }
            successCount++;
        }

        file.close();
        cout << "[INFO] CSV loading complete: " << successCount << " records loaded, " << errorCount << " errors" << endl;
    }

    // Displays the complete hierarchy of a specific school
    // Data Structure: HashTable (school lookup) + N-ary Tree (hierarchy display)
    void displaySchoolHierarchy(const string& schoolID) {
        if (schoolID.empty()) {
            cout << "[ERROR] School ID cannot be empty" << endl;
            return;
        }
        
        SchoolInfo* school = schoolHash.search(schoolID);
        if (!school) {
            cout << "[ERROR] School not found" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSchoolTree(schoolID);
        cout << endl << "=== SCHOOL INFO ===" << endl;
        cout << "ID: " << school->schoolID << endl;
        cout << "Name: " << school->schoolName << endl;
        cout << "Sector: " << school->sector << endl;
        cout << "Rating: " << school->rating << endl;
        cout << endl << "Hierarchy:" << endl;
        
        if (tree) {
            tree->printTree(tree->root);
        }
        else {
            cout << "[INFO] No hierarchy available" << endl;
        }
    }

    // Displays all school hierarchies in a numbered format
    // Data Structure: LinkedList (school traversal) + N-ary Tree (hierarchical display)
    void displayAllHierarchiesNumbered() {
        cout << endl << "===== COMPLETE HIERARCHY (Numbered) =====" << endl;
        int schoolIdx = 0;
        LListNode<NaryTree<string>*>* schoolNode = schoolForest.getHead();
        
        while (schoolNode) {
            ++schoolIdx;
            string root = schoolNode->data->root->data;
            string schoolID = extractSchoolIDFromRoot(root);
            SchoolInfo* school = schoolHash.search(schoolID);
            cout << schoolIdx << ". " << (school ? school->schoolName : root) << " (" << schoolID << ")" << endl;

            int deptIdx = 0;
            TreeNode<string>* dept = schoolNode->data->root->firstChild;
            while (dept) {
                ++deptIdx;
                cout << "   " << schoolIdx << "." << deptIdx << " " << dept->data << endl;
                
                int classIdx = 0;
                TreeNode<string>* classNode = dept->firstChild;
                while (classNode) {
                    ++classIdx;
                    cout << "      - " << schoolIdx << "." << deptIdx << "." << classIdx << " " << classNode->data << endl;
                    classNode = classNode->nextSibling;
                }
                dept = dept->nextSibling;
            }
            schoolNode = schoolNode->next;
        }
    }

    // Lists all schools with their departments and classes
    // Data Structure: LinkedList (school traversal) + N-ary Tree (hierarchical listing)
    void listAllSchools() {
        cout << endl << "===== ALL SCHOOLS =====" << endl;
        LListNode<NaryTree<string>*>* node = schoolForest.getHead();
        
        while (node) {
            string root = node->data->root->data;
            string schoolID = extractSchoolIDFromRoot(root);
            SchoolInfo* school = schoolHash.search(schoolID);
            
            if (school) {
                cout << "School: " << school->schoolName << " (" << school->schoolID 
                     << ") Sector: " << school->sector << " Rating: " << school->rating << endl;
                cout << " Departments & Classes:" << endl;
                
                TreeNode<string>* dept = node->data->root->firstChild;
                while (dept) {
                    cout << "  - " << dept->data << endl;
                    TreeNode<string>* classNode = dept->firstChild;
                    while (classNode) {
                        cout << "     * " << classNode->data << endl;
                        classNode = classNode->nextSibling;
                    }
                    dept = dept->nextSibling;
                }
            }
            node = node->next;
        }
    }

    // Main menu interface for the Education module
    void menu() {
        // ANSI color codes
        const string RESET = "\033[0m";
        const string BOLD = "\033[1m";
        const string YELLOW = "\033[33m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string WHITE = "\033[37m";
        const string BRIGHT_WHITE = "\033[97m";
        const string CYAN = "\033[36m";
        const string BRIGHT_RED = "\033[91m";
        
        while (true) {
            cout << "\n" << BRIGHT_YELLOW << BOLD << "============================================================" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "              EDUCATION MANAGEMENT SYSTEM" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "============================================================" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  STRUCTURE MANAGEMENT" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  1. Add School         2. Add Department       3. Add Class" << RESET << endl;
            cout << BRIGHT_WHITE << "  4. Display School    5. List All Schools" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  FACULTY MANAGEMENT" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  6. Register Faculty   7. List Faculty (Dept)   8. Delete Faculty" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  STUDENT MANAGEMENT" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  9. Register Student  10. List Students (Class) 11. Delete Student" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  SEARCH & RANKING" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 12. Search Schools offering Class" << RESET << endl;
            cout << BRIGHT_WHITE << " 13. Rank Schools (Top K)        14. Locate Nearest School" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  DELETION OPERATIONS" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 16. Delete School     17. Delete Department    18. Delete Class" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  DATA IMPORT & DISPLAY" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 15. Load Schools CSV  19. Display ALL Hierarchies (Numbered)" << RESET << endl << endl;
            
            cout << BRIGHT_WHITE << BOLD << "  0. Back to Main Menu" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "Choose: " << RESET;
            int ch;
            cin >> ch;
            cin.ignore();

            if (ch == 0) return;

            string schoolID, schoolName, sector, dept, className, facultyID, facultyName, qualification;
            string studentID, studentName;
            double rating;
            int age;
            int K;

            switch (ch) {
            case 1:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add School]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "School Name: " << RESET;
                getline(cin, schoolName);
                cout << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Rating: " << RESET;
                cin >> rating;
                cin.ignore();
                addSchool(trimCopy(schoolID), trimCopy(schoolName), trimCopy(sector), rating);
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 2:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add Department]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department name: " << RESET;
                getline(cin, dept);
                addDepartment(trimCopy(schoolID), trimCopy(dept));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 3:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add Class]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                cout << CYAN << "Class name: " << RESET;
                getline(cin, className);
                addClass(trimCopy(schoolID), trimCopy(dept), trimCopy(className));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 4:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Display School]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                displaySchoolHierarchy(trimCopy(schoolID));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 5:
                listAllSchools();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 6:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Register Faculty]" << RESET << endl;
                cout << CYAN << "Faculty ID: " << RESET;
                getline(cin, facultyID);
                cout << CYAN << "Name: " << RESET;
                getline(cin, facultyName);
                cout << CYAN << "Qualification: " << RESET;
                getline(cin, qualification);
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                registerFaculty(trimCopy(facultyID), trimCopy(facultyName), trimCopy(qualification), trimCopy(schoolID), trimCopy(dept));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 7:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[List Faculty]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                listFacultyOfDepartment(trimCopy(schoolID), trimCopy(dept));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 8:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Faculty]" << RESET << endl;
                cout << CYAN << "Faculty ID: " << RESET;
                getline(cin, facultyID);
                deleteFaculty(trimCopy(facultyID));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 9:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Register Student]" << RESET << endl;
                cout << CYAN << "Student ID: " << RESET;
                getline(cin, studentID);
                cout << CYAN << "Name: " << RESET;
                getline(cin, studentName);
                cout << CYAN << "Age: " << RESET;
                cin >> age;
                cin.ignore();
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                cout << CYAN << "Class: " << RESET;
                getline(cin, className);
                registerStudent(trimCopy(studentID), trimCopy(studentName), age, trimCopy(schoolID), trimCopy(dept), trimCopy(className));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 10:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[List Students]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                cout << CYAN << "Class: " << RESET;
                getline(cin, className);
                listStudentsInClass(trimCopy(schoolID), trimCopy(dept), trimCopy(className));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 11:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Student]" << RESET << endl;
                cout << CYAN << "Student ID: " << RESET;
                getline(cin, studentID);
                deleteStudent(trimCopy(studentID));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 12:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Search Schools]" << RESET << endl;
                cout << CYAN << "Class/Subject: " << RESET;
                getline(cin, className);
                findSchoolsOfferingClass(trimCopy(className));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 13:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Rank Schools]" << RESET << endl;
                cout << CYAN << "Top K: " << RESET;
                cin >> K;
                cin.ignore();
                rankSchoolsAndShowTop(K);
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 14:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Locate Nearest School]" << RESET << endl;
                cout << CYAN << "Current Node ID (e.g. H_... or T_stop...): " << RESET;
                getline(cin, schoolID);
                locateNearestSchool(trimCopy(schoolID));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 15:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Load Schools CSV]" << RESET << endl;
                cout << CYAN << "CSV filename: " << RESET;
                getline(cin, schoolID);
                loadFromCSV(trimCopy(schoolID));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 16:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete School]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                deleteSchool(trimCopy(schoolID));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 17:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Department]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                deleteDepartment(trimCopy(schoolID), trimCopy(dept));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 18:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Class]" << RESET << endl;
                cout << CYAN << "School ID: " << RESET;
                getline(cin, schoolID);
                cout << CYAN << "Department: " << RESET;
                getline(cin, dept);
                cout << CYAN << "Class: " << RESET;
                getline(cin, className);
                deleteClass(trimCopy(schoolID), trimCopy(dept), trimCopy(className));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 19:
                displayAllHierarchiesNumbered();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            default:
                cout << "\n" << BRIGHT_RED << BOLD << "[ERROR]" << RESET << BRIGHT_WHITE << " Invalid choice. Please select a number from the menu." << RESET << endl;
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
            }
        }
    }
};

