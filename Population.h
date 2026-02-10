#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include "Data_Structures.h"
#include "MainCityGraph.h"

using namespace std;

struct Person {
    string cnic;
    string name;
    int age;
    string gender;
    string sector;
    string street;
    string houseNo;
    string occupation;
};

class PopulationModule {
private:
    MainCityGraph* mainGraph;

    LinkedList< NaryTree<string>* > housingForest;
    ChainedMap< LinkedList<string> > houseToResidents;
    ChainedMap<Person> citizenHash;
    ChainedMap< NaryTree<string>* > familyTrees;

    // Helper function to trim whitespace from a string
    static string trimCopyStatic(string inputString) {
        int startIndex = 0;
        int endIndex = (int)inputString.length() - 1;
        
        while (startIndex <= endIndex && isspace((unsigned char)inputString[startIndex])) {
            ++startIndex;
        }
        while (endIndex >= startIndex && isspace((unsigned char)inputString[endIndex])) {
            --endIndex;
        }
        
        if (startIndex > endIndex) {
            return string("");
        }
        return inputString.substr(startIndex, endIndex - startIndex + 1);
    }

    // Wrapper function for trimming strings
    static string trimCopy(const string& inputString) {
        return trimCopyStatic(string(inputString));
    }

    // Parses a CSV line into fields, handling quoted values
    static void parseCSVLine(const string& csvLine, string outputFields[], int expectedFieldCount) {
        int currentFieldIndex = 0;
        bool insideQuotes = false;
        string currentField = "";
        
        for (int i = 0; i < (int)csvLine.length(); ++i) {
            char currentChar = csvLine[i];
            
            if (currentChar == '"') {
                insideQuotes = !insideQuotes;
                continue;
            }
            
            if (currentChar == ',' && !insideQuotes) {
                if (currentFieldIndex < expectedFieldCount) {
                    outputFields[currentFieldIndex++] = currentField;
                }
                currentField = "";
            }
            else {
                currentField += currentChar;
            }
        }
        
        if (currentFieldIndex < expectedFieldCount) {
            outputFields[currentFieldIndex] = currentField;
        }
    }

    // Extracts sector ID from a root node's data string
    static string extractSectorIDFromRoot(const string& rootData) {
        size_t separatorPosition = rootData.find(" | ");
        
        if (separatorPosition == string::npos) {
            separatorPosition = rootData.find('|');
            if (separatorPosition == string::npos) {
                return trimCopyStatic(rootData);
            }
            return trimCopyStatic(rootData.substr(0, separatorPosition));
        }
        
        return trimCopyStatic(rootData.substr(0, separatorPosition));
    }

    // Creates a unique key for a house by combining sector, street, and house number
    string makeHouseKey(const string& sector, const string& street, const string& houseNumber) const {
        return sector + "|" + street + "|" + houseNumber;
    }

    // Finds the sector tree in the housing forest
    // Data Structure: LinkedList - used to store multiple sector trees, allows sequential traversal
    NaryTree<string>* findSectorTree(const string& sectorID) {
        LListNode<NaryTree<string>*>* currentNode = housingForest.getHead();
        
        while (currentNode) {
            string rootData = currentNode->data->root->data;
            string extractedID = extractSectorIDFromRoot(rootData);
            
            if (extractedID == sectorID) {
                return currentNode->data;
            }
            currentNode = currentNode->next;
        }
        
        return nullptr;
    }

    // Checks if a value exists in a linked list
    // Data Structure: LinkedList - simple sequential search for membership check
    bool containsInList(LinkedList<string>& listToSearch, const string& valueToFind) {
        LListNode<string>* currentNode = listToSearch.getHead();
        
        while (currentNode) {
            if (currentNode->data == valueToFind) {
                return true;
            }
            currentNode = currentNode->next;
        }
        
        return false;
    }

    // Finds a child node with matching data in a tree node's children
    // Data Structure: N-ary Tree - traverses sibling-linked children
    TreeNode<string>* findChild(TreeNode<string>* parentNode, const string& targetData) {
        if (!parentNode) {
            return nullptr;
        }
        
        TreeNode<string>* currentChild = parentNode->firstChild;
        
        while (currentChild) {
            if (currentChild->data == targetData) {
                return currentChild;
            }
            currentChild = currentChild->nextSibling;
        }
        
        return nullptr;
    }

    // Finds a nested child under a parent (e.g., house under street)
    // Data Structure: N-ary Tree - two-level traversal for hierarchical lookup
    TreeNode<string>* findChildUnderParent(TreeNode<string>* parentNode, const string& intermediateChildName, const string& targetChildName) {
        if (!parentNode) {
            return nullptr;
        }
        
        TreeNode<string>* intermediateNode = findChild(parentNode, intermediateChildName);
        if (!intermediateNode) {
            return nullptr;
        }
        
        return findChild(intermediateNode, targetChildName);
    }

    // Calculates coordinates for a street within a sector
    void getStreetCoordinates(const string& sectorID, int streetNumber, double& outputLatitude, double& outputLongitude) {
        double baseLatitude = 33.7;
        double baseLongitude = 73.1;
        
        if (mainGraph) {
            mainGraph->getSectorSystem().getCoordinatesInSector(sectorID, baseLatitude, baseLongitude);
        }
        
        double baseShiftAmount = 0.0008;
        double calculatedShift = streetNumber * baseShiftAmount;
        
        if (streetNumber % 2 == 1) {
            outputLatitude = baseLatitude + calculatedShift;
            outputLongitude = baseLongitude;
        }
        else {
            outputLatitude = baseLatitude;
            outputLongitude = baseLongitude + calculatedShift;
        }
    }

    // Calculates coordinates for a house based on street coordinates
    void getHouseCoordinates(double streetLatitude, double streetLongitude, int houseNumber, double& outputLatitude, double& outputLongitude) {
        int moduloResult = (houseNumber % 10);
        double jitterAmount = moduloResult * 0.00007;
        
        outputLatitude = streetLatitude + jitterAmount;
        outputLongitude = streetLongitude - jitterAmount * 0.6;
    }

    // Removes a child node from its parent's children list
    // Data Structure: N-ary Tree - maintains sibling links for efficient removal
    TreeNode<string>* removeChildNode(TreeNode<string>* parentNode, const string& childDataToRemove) {
        if (!parentNode) {
            return nullptr;
        }
        
        TreeNode<string>* currentChild = parentNode->firstChild;
        TreeNode<string>* previousChild = nullptr;
        
        while (currentChild) {
            if (currentChild->data == childDataToRemove) {
                if (previousChild) {
                    previousChild->nextSibling = currentChild->nextSibling;
                }
                else {
                    parentNode->firstChild = currentChild->nextSibling;
                }
                currentChild->nextSibling = nullptr;
                return currentChild;
            }
            previousChild = currentChild;
            currentChild = currentChild->nextSibling;
        }
        
        return nullptr;
    }

    // Recursively deletes a subtree starting from the given node
    // Data Structure: N-ary Tree - recursive deletion of all descendants
    void deleteSubtree(TreeNode<string>* rootNode) {
        if (!rootNode) {
            return;
        }
        
        TreeNode<string>* currentChild = rootNode->firstChild;
        
        while (currentChild) {
            TreeNode<string>* nextSibling = currentChild->nextSibling;
            deleteSubtree(currentChild);
            currentChild = nextSibling;
        }
        
        delete rootNode;
    }

    // Ensures a family tree exists for a house, creates one if it doesn't
    // Data Structure: ChainedMap - O(1) average lookup for family trees by house key
    NaryTree<string>* ensureFamilyTree(const string& sectorID, const string& streetName, const string& houseNumber) {
        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        NaryTree<string>** existingTreePtr = familyTrees.search(houseKey);
        
        if (existingTreePtr && *existingTreePtr) {
            return *existingTreePtr;
        }
        
        NaryTree<string>* newFamilyTree = new NaryTree<string>(houseKey);
        familyTrees.insert(houseKey, newFamilyTree);
        return newFamilyTree;
    }

    // Attaches an existing node as a child of a new parent
    // Data Structure: N-ary Tree - maintains parent-child and sibling relationships
    void attachExistingNode(TreeNode<string>* newParentNode, TreeNode<string>* nodeToAttach) {
        if (!newParentNode || !nodeToAttach) {
            return;
        }
        
        nodeToAttach->nextSibling = newParentNode->firstChild;
        newParentNode->firstChild = nodeToAttach;
        nodeToAttach->parent = newParentNode;
    }

    // Finds a person node in a family tree by CNIC
    // Data Structure: N-ary Tree - tree search for person lookup
    TreeNode<string>* findPersonNodeInFamilyTree(NaryTree<string>* familyTree, const string& personCNIC) {
        if (!familyTree) {
            return nullptr;
        }
        return familyTree->findNode(familyTree->root, personCNIC);
    }

public:

    PopulationModule(MainCityGraph* g)
        : mainGraph(g),
        houseToResidents(2003),
        citizenHash(10007),
        familyTrees(2003)
    {
    }

    // Lists all residents in a specific street
    // Data Structure: N-ary Tree (hierarchy) + ChainedMap (resident lookup) + LinkedList (resident lists)
    void listResidentsInStreet(const string& sectorID, const string& streetName) {
        if (sectorID.empty() || streetName.empty()) {
            cout << "[ERROR] Sector ID and Street name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* sectorTree = findSectorTree(sectorID);
        if (!sectorTree) {
            cout << "[ERROR] Sector not found" << endl;
            return;
        }
        
        TreeNode<string>* streetNode = sectorTree->findNode(sectorTree->root, streetName);
        if (!streetNode) {
            cout << "[ERROR] Street not found" << endl;
            return;
        }
        
        cout << endl << "Residents in Street " << streetName << " (" << sectorID << "):" << endl;
        TreeNode<string>* house = streetNode->firstChild;

        while (house) {
            string houseKey = makeHouseKey(sectorID, streetName, house->data);
            LinkedList<string>* residents = houseToResidents.search(houseKey);
            
            if (residents) {
                LListNode<string>* resident = residents->getHead();
                
                while (resident) {
                    Person* person = citizenHash.search(resident->data);
                    if (person) {
                        cout << person->cnic << " | " << person->name << " | "
                            << person->occupation << " | Age: " << person->age
                            << " | Gender: " << person->gender << endl;
                    }
                    resident = resident->next;
                }
            }
            house = house->nextSibling;
        }
    }

    // Lists all residents in a specific house
    // Data Structure: ChainedMap (O(1) house lookup) + LinkedList (resident list) + ChainedMap (citizen lookup)
    void listResidentsInHouse(const string& sectorID, const string& streetName, const string& houseNumber) {
        if (sectorID.empty() || streetName.empty() || houseNumber.empty()) {
            cout << "[ERROR] Sector ID, Street name, and House number cannot be empty" << endl;
            return;
        }
        
        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        LinkedList<string>* residents = houseToResidents.search(houseKey);
        
        if (!residents) {
            cout << "[INFO] No residents in this house or house not found" << endl;
            return;
        }

        cout << endl << "Residents in House " << houseNumber << " (" << streetName << " - " << sectorID << "):" << endl;
        LListNode<string>* resident = residents->getHead();
        
        while (resident) {
            Person* person = citizenHash.search(resident->data);
            if (person) {
                cout << person->cnic << " | " << person->name
                    << " | Age: " << person->age << " | "
                    << person->occupation << " | Gender: " << person->gender << endl;
            }
            resident = resident->next;
        }
    }

    // Lists all residents in a specific sector
    // Data Structure: N-ary Tree (hierarchical traversal) - recursively lists all streets
    void listResidentsInSector(const string& sectorID) {
        if (sectorID.empty()) {
            cout << "[ERROR] Sector ID cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* sectorTree = findSectorTree(sectorID);
        if (!sectorTree) {
            cout << "[ERROR] Sector not found" << endl;
            return;
        }

        cout << endl << "Residents in Sector " << sectorID << ":" << endl;
        TreeNode<string>* street = sectorTree->root->firstChild;

        while (street) {
            listResidentsInStreet(sectorID, street->data);
            street = street->nextSibling;
        }
    }

    // Adds a new sector to the housing hierarchy
    // Data Structure: LinkedList (stores sector trees) + N-ary Tree (sector hierarchy)
    void addSector(const string& sectorID) {
        if (sectorID.empty()) {
            cout << "[ERROR] Sector ID cannot be empty" << endl;
            return;
        }
        
        double lat = 0.0;
        double lon = 0.0;
        
        if (mainGraph) {
            if (!mainGraph->getSectorSystem().getCoordinatesInSector(sectorID, lat, lon)) {
                cout << "[ERROR] Sector not found in MainCityGraph: " << sectorID << endl;
                return;
            }
        }
        
        if (findSectorTree(sectorID)) {
            cout << "[INFO] Sector already exists: " << sectorID << endl;
            return;
        }
        
        NaryTree<string>* tree = new NaryTree<string>(sectorID + " | " + sectorID);
        housingForest.insertBack(tree);
        cout << "[SUCCESS] Sector added: " << sectorID << endl;
    }

    // Adds a new street to a sector
    // Data Structure: N-ary Tree (adds child node to sector tree)
    void addStreet(const string& sectorID, const string& streetName) {
        if (sectorID.empty() || streetName.empty()) {
            cout << "[ERROR] Sector ID and Street name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSectorTree(sectorID);
        if (!tree) {
            cout << "[ERROR] Sector not found: " << sectorID << endl;
            return;
        }
        
        TreeNode<string>* existing = findChild(tree->root, streetName);
        if (existing && existing->parent == tree->root) {
            cout << "[INFO] Street already exists: " << streetName << endl;
            return;
        }
        
        tree->addChild(tree->root, streetName);
        cout << "[SUCCESS] Street added: " << streetName << " in " << sectorID << endl;
    }

    // Adds a new house to a street
    // Data Structure: N-ary Tree (hierarchy) + ChainedMap (resident mapping) + ChainedMap (family trees)
    void addHouse(const string& sectorID, const string& streetName, const string& houseNumber) {
        if (sectorID.empty() || streetName.empty() || houseNumber.empty()) {
            cout << "[ERROR] Sector ID, Street name, and House number cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSectorTree(sectorID);
        if (!tree) {
            cout << "[ERROR] Sector not found: " << sectorID << endl;
            return;
        }
        
        TreeNode<string>* street = findChild(tree->root, streetName);
        if (!street) {
            cout << "[ERROR] Street not found: " << streetName << endl;
            return;
        }
        
        TreeNode<string>* existing = findChild(street, houseNumber);
        if (existing && existing->parent == street) {
            cout << "[INFO] House already exists" << endl;
            return;
        }
        
        tree->addChild(street, houseNumber);
        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        
        LinkedList<string> emptyList;
        houseToResidents.insert(houseKey, emptyList);
        
        NaryTree<string>* familyTree = new NaryTree<string>(houseKey);
        familyTrees.insert(houseKey, familyTree);
        
        int streetNum = 1;
        int houseNum = 1;
        try {
            streetNum = stoi(streetName);
        }
        catch (...) {
            streetNum = 1;
        }
        try {
            houseNum = stoi(houseNumber);
        }
        catch (...) {
            houseNum = 1;
        }

        double sLat, sLon, hLat, hLon;
        getStreetCoordinates(sectorID, streetNum, sLat, sLon);
        getHouseCoordinates(sLat, sLon, houseNum, hLat, hLon);
        
        string safeID = sectorID;
        for (char& c : safeID) {
            if (isspace((unsigned char)c)) {
                c = '_';
            }
        }
        
        string nodeID = "H_" + safeID + "_" + streetName + "_" + houseNumber;
        string label = sectorID + " / St:" + streetName + " / H:" + houseNumber;

        if (mainGraph) {
            mainGraph->addNodeWithAutoConnect(nodeID, MODULE_HOUSING, label, hLat, hLon, sectorID, nullptr);
        }

        cout << "[SUCCESS] House " << houseNumber << " added under Street " << streetName << " (" << sectorID << ")" << endl;
    }

    // Registers a new citizen in the system
    // Data Structure: ChainedMap (O(1) citizen lookup) + ChainedMap (house-resident mapping) + N-ary Tree (family tree)
    void registerCitizen(const string& cnic, const string& name, int age, const string& gender,
        const string& sectorID, const string& streetName, const string& houseNumber, const string& occupation) {
        if (cnic.empty() || name.empty()) {
            cout << "[ERROR] CNIC and Name cannot be empty" << endl;
            return;
        }
        
        if (sectorID.empty() || streetName.empty() || houseNumber.empty()) {
            cout << "[ERROR] Sector ID, Street name, and House number cannot be empty" << endl;
            return;
        }
        
        if (age < 0 || age > 150) {
            cout << "[ERROR] Invalid age: " << age << ". Age must be between 0 and 150" << endl;
            return;
        }
        
        if (citizenHash.search(cnic)) {
            cout << "[INFO] Citizen already registered: " << cnic << endl;
            return;
        }

        NaryTree<string>* sectorTree = findSectorTree(sectorID);
        if (!sectorTree) {
            cout << "[ERROR] Sector not found: " << sectorID << endl;
            return;
        }
        
        TreeNode<string>* streetNode = findChild(sectorTree->root, streetName);
        if (!streetNode) {
            cout << "[ERROR] Street not found: " << streetName << endl;
            return;
        }
        
        TreeNode<string>* houseNode = findChild(streetNode, houseNumber);
        if (!houseNode) {
            cout << "[ERROR] House not found: " << houseNumber << " under street " << streetName << endl;
            return;
        }

        Person person;
        person.cnic = cnic;
        person.name = name;
        person.age = age;
        person.gender = gender;
        person.sector = sectorID;
        person.street = streetName;
        person.houseNo = houseNumber;
        person.occupation = occupation;

        citizenHash.insert(cnic, person);

        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        LinkedList<string>* residents = houseToResidents.search(houseKey);
        
        if (!residents) {
            LinkedList<string> newList;
            newList.insertBack(cnic);
            houseToResidents.insert(houseKey, newList);
        }
        else {
            if (!containsInList(*residents, cnic)) {
                residents->insertBack(cnic);
            }
        }
        
        NaryTree<string>* familyTree = ensureFamilyTree(sectorID, streetName, houseNumber);
        if (familyTree) {
            TreeNode<string>* existing = familyTree->findNode(familyTree->root, cnic);
            if (!existing) {
                familyTree->addChild(familyTree->root, cnic);
            }
        }

        cout << "[SUCCESS] Citizen registered: " << cnic << " (" << name << ")" << endl;
    }

    // Finds a citizen by their CNIC
    // Data Structure: ChainedMap - O(1) average lookup time
    Person* findCitizenByCNIC(const string& cnic) {
        return citizenHash.search(cnic);
    }

    // Prints information about a resident by CNIC
    // Data Structure: ChainedMap - fast citizen lookup
    void printResidentByCNIC(const string& cnic) {
        if (cnic.empty()) {
            cout << "[ERROR] CNIC cannot be empty" << endl;
            return;
        }
        
        Person* person = findCitizenByCNIC(cnic);
        if (!person) {
            cout << "[INFO] Resident not found: " << cnic << endl;
            return;
        }
        
        cout << person->cnic << " | " << person->name << " | Age: " << person->age 
            << " | Gender: " << person->gender
            << " | " << person->sector << " | Street: " << person->street 
            << " | House: " << person->houseNo
            << " | Occupation: " << person->occupation << endl;
    }

    // Prints all residents in the system
    // Data Structure: LinkedList (sector traversal) + N-ary Tree (hierarchy) + ChainedMap (lookups)
    void printAllResidents() {
        cout << endl << "=== ALL RESIDENTS ===" << endl;
        LListNode<NaryTree<string>*>* sector = housingForest.getHead();
        
        if (!sector) {
            cout << "[INFO] No sectors loaded" << endl;
            return;
        }
        
        while (sector) {
            string sectorID = extractSectorIDFromRoot(sector->data->root->data);
            TreeNode<string>* street = sector->data->root->firstChild;
            
            while (street) {
                TreeNode<string>* house = street->firstChild;
                
                while (house) {
                    string houseKey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(houseKey);
                    
                    if (residents) {
                        LListNode<string>* resident = residents->getHead();
                        
                        while (resident) {
                            Person* person = citizenHash.search(resident->data);
                            if (person) {
                                cout << person->cnic << " | " << person->name << " | Age: " << person->age
                                    << " | Gender: " << person->gender << " | Sector: " << person->sector
                                    << " | Street: " << person->street << " | House: " << person->houseNo
                                    << " | Occupation: " << person->occupation << endl;
                            }
                            resident = resident->next;
                        }
                    }
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            sector = sector->next;
        }
    }

    // Updates resident information, handling address changes
    // Data Structure: ChainedMap (citizen lookup) + ChainedMap (house-resident mapping) + N-ary Tree (family tree updates)
    void updateResident(const string& cnic,
        const string& newName,
        int newAge,
        const string& newGender,
        const string& newSector,
        const string& newStreet,
        const string& newHouse,
        const string& newOccupation) {
        if (cnic.empty()) {
            cout << "[ERROR] CNIC cannot be empty" << endl;
            return;
        }
        
        if (newSector.empty() || newStreet.empty() || newHouse.empty()) {
            cout << "[ERROR] New Sector ID, Street name, and House number cannot be empty" << endl;
            return;
        }
        
        if (newAge < 0 || newAge > 150) {
            cout << "[ERROR] Invalid age: " << newAge << ". Age must be between 0 and 150" << endl;
            return;
        }
        
        Person* person = citizenHash.search(cnic);
        if (!person) {
            cout << "[ERROR] Resident not found: " << cnic << endl;
            return;
        }

        string oldSector = person->sector;
        string oldStreet = person->street;
        string oldHouse = person->houseNo;
        
        person->name = newName;
        person->age = newAge;
        person->gender = newGender;
        person->occupation = newOccupation;
        
        bool addressChanged = (oldSector != newSector) || (oldStreet != newStreet) || (oldHouse != newHouse);
        
        if (addressChanged) {
            if (!findSectorTree(newSector)) {
                double dummyLat, dummyLon;
                if (mainGraph && mainGraph->getSectorSystem().getCoordinatesInSector(newSector, dummyLat, dummyLon)) {
                    addSector(newSector);
                }
                else {
                    cout << "[ERROR] New sector not found in MainCityGraph: " << newSector << ". Aborting address update." << endl;
                    return;
                }
            }
            
            NaryTree<string>* newTree = findSectorTree(newSector);
            if (!findChild(newTree->root, newStreet)) {
                addStreet(newSector, newStreet);
            }
            if (!findChildUnderParent(newTree->root, newStreet, newHouse)) {
                addHouse(newSector, newStreet, newHouse);
            }

            string oldKey = makeHouseKey(oldSector, oldStreet, oldHouse);
            LinkedList<string>* oldList = houseToResidents.search(oldKey);
            if (oldList) {
                oldList->removeByValue(cnic);
                if (!oldList->getHead()) {
                    houseToResidents.remove(oldKey);
                }
            }

            NaryTree<string>** oldTreePtr = familyTrees.search(oldKey);
            if (oldTreePtr && *oldTreePtr) {
                TreeNode<string>* node = (*oldTreePtr)->findNode((*oldTreePtr)->root, cnic);
                if (node && node->parent) {
                    TreeNode<string>* detached = removeChildNode(node->parent, node->data);
                    if (detached) {
                        // Node detached, will be reattached to new family tree
                    }
                }
            }

            string newKey = makeHouseKey(newSector, newStreet, newHouse);
            LinkedList<string>* newList = houseToResidents.search(newKey);
            if (!newList) {
                LinkedList<string> list;
                list.insertBack(cnic);
                houseToResidents.insert(newKey, list);
            }
            else {
                if (!containsInList(*newList, cnic)) {
                    newList->insertBack(cnic);
                }
            }
            
            person->sector = newSector;
            person->street = newStreet;
            person->houseNo = newHouse;
            
            NaryTree<string>* familyTree = ensureFamilyTree(newSector, newStreet, newHouse);
            if (familyTree) {
                TreeNode<string>* existing = familyTree->findNode(familyTree->root, cnic);
                if (!existing) {
                    familyTree->addChild(familyTree->root, cnic);
                }
            }

            cout << "[SUCCESS] Resident " << cnic << " address updated to " << newSector << " / " << newStreet << " / " << newHouse << endl;
        }
        else {
            cout << "[SUCCESS] Resident " << cnic << " info updated (no address change)." << endl;
        }
    }

    // Deletes a citizen from the system
    // Data Structure: ChainedMap (removes from all mappings) + N-ary Tree (removes from family tree)
    void deleteCitizen(const string& cnic) {
        if (cnic.empty()) {
            cout << "[ERROR] CNIC cannot be empty" << endl;
            return;
        }
        
        Person* person = citizenHash.search(cnic);
        if (!person) {
            cout << "[ERROR] Citizen not found" << endl;
            return;
        }
        
        string houseKey = makeHouseKey(person->sector, person->street, person->houseNo);
        LinkedList<string>* residents = houseToResidents.search(houseKey);
        
        if (residents) {
            residents->removeByValue(cnic);
            if (!residents->getHead()) {
                houseToResidents.remove(houseKey);
            }
        }
        
        NaryTree<string>** treePtr = familyTrees.search(houseKey);
        if (treePtr && *treePtr) {
            TreeNode<string>* node = (*treePtr)->findNode((*treePtr)->root, cnic);
            if (node && node->parent) {
                TreeNode<string>* detached = removeChildNode(node->parent, node->data);
                if (detached) {
                    deleteSubtree(detached);
                }
            }
        }

        citizenHash.remove(cnic);
        cout << "[SUCCESS] Citizen removed: " << cnic << endl;
    }

    // Deletes a house and all its residents
    // Data Structure: N-ary Tree (removes from hierarchy) + ChainedMap (removes resident mappings) + ChainedMap (removes family tree)
    void deleteHouse(const string& sectorID, const string& streetName, const string& houseNumber) {
        if (sectorID.empty() || streetName.empty() || houseNumber.empty()) {
            cout << "[ERROR] Sector ID, Street name, and House number cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSectorTree(sectorID);
        if (!tree) {
            cout << "[ERROR] Sector not found" << endl;
            return;
        }
        
        TreeNode<string>* street = findChild(tree->root, streetName);
        if (!street) {
            cout << "[ERROR] Street not found" << endl;
            return;
        }
        
        TreeNode<string>* removed = removeChildNode(street, houseNumber);
        if (!removed) {
            cout << "[ERROR] House not found" << endl;
            return;
        }

        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        LinkedList<string>* residents = houseToResidents.search(houseKey);
        
        if (residents) {
            LListNode<string>* resident = residents->getHead();
            while (resident) {
                citizenHash.remove(resident->data);
                resident = resident->next;
            }
            houseToResidents.remove(houseKey);
        }

        NaryTree<string>** treePtr = familyTrees.search(houseKey);
        if (treePtr && *treePtr) {
            delete *treePtr;
            familyTrees.remove(houseKey);
        }
        
        deleteSubtree(removed);
        cout << "[SUCCESS] House deleted" << endl;
    }

    // Deletes a street and all its houses and residents
    // Data Structure: N-ary Tree (hierarchical deletion) + ChainedMap (batch removal of residents and family trees)
    void deleteStreet(const string& sectorID, const string& streetName) {
        if (sectorID.empty() || streetName.empty()) {
            cout << "[ERROR] Sector ID and Street name cannot be empty" << endl;
            return;
        }
        
        NaryTree<string>* tree = findSectorTree(sectorID);
        if (!tree) {
            cout << "[ERROR] Sector not found" << endl;
            return;
        }
        
        TreeNode<string>* removed = removeChildNode(tree->root, streetName);
        if (!removed) {
            cout << "[ERROR] Street not found" << endl;
            return;
        }

        TreeNode<string>* house = removed->firstChild;
        while (house) {
            string houseKey = makeHouseKey(sectorID, streetName, house->data);
            LinkedList<string>* residents = houseToResidents.search(houseKey);
            
            if (residents) {
                LListNode<string>* resident = residents->getHead();
                while (resident) {
                    citizenHash.remove(resident->data);
                    resident = resident->next;
                }
                houseToResidents.remove(houseKey);
            }
            
            NaryTree<string>** treePtr = familyTrees.search(houseKey);
            if (treePtr && *treePtr) {
                delete *treePtr;
                familyTrees.remove(houseKey);
            }
            house = house->nextSibling;
        }

        deleteSubtree(removed);
        cout << "[SUCCESS] Street deleted" << endl;
    }

    // Deletes a sector and all its streets, houses, and residents
    // Data Structure: LinkedList (removes sector tree) + N-ary Tree (recursive deletion) + ChainedMap (cleanup)
    void deleteSector(const string& sectorID) {
        if (sectorID.empty()) {
            cout << "[ERROR] Sector ID cannot be empty" << endl;
            return;
        }
        
        LListNode<NaryTree<string>*>* prev = nullptr;
        LListNode<NaryTree<string>*>* curr = housingForest.getHead();
        
        while (curr) {
            string rootData = curr->data->root->data;
            string id = extractSectorIDFromRoot(rootData);
            
            if (id == sectorID) {
                TreeNode<string>* street = curr->data->root->firstChild;
                
                while (street) {
                    string streetName = street->data;
                    TreeNode<string>* house = street->firstChild;
                    
                    while (house) {
                        string houseKey = makeHouseKey(sectorID, streetName, house->data);
                        LinkedList<string>* residents = houseToResidents.search(houseKey);
                        
                        if (residents) {
                            LListNode<string>* resident = residents->getHead();
                            while (resident) {
                                citizenHash.remove(resident->data);
                                resident = resident->next;
                            }
                            houseToResidents.remove(houseKey);
                        }
                        
                        NaryTree<string>** treePtr = familyTrees.search(houseKey);
                        if (treePtr && *treePtr) {
                            delete *treePtr;
                            familyTrees.remove(houseKey);
                        }
                        house = house->nextSibling;
                    }
                    street = street->nextSibling;
                }

                delete curr->data;
                if (prev) {
                    prev->next = curr->next;
                }
                else {
                    housingForest.setHead(curr->next);
                }
                delete curr;
                cout << "[SUCCESS] Sector deleted completely" << endl;
                return;
            }
            prev = curr;
            curr = curr->next;
        }
        
        cout << "[ERROR] Sector not found" << endl;
    }

    // Adds a family member to a house's family tree
    // Data Structure: N-ary Tree (family tree structure) + ChainedMap (family tree lookup)
    void addFamilyMemberToHouse(const string& sectorID, const string& streetName, const string& houseNumber, const string& personCNIC) {
        if (sectorID.empty() || streetName.empty() || houseNumber.empty() || personCNIC.empty()) {
            cout << "[ERROR] Sector ID, Street name, House number, and Person CNIC cannot be empty" << endl;
            return;
        }
        
        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        NaryTree<string>* tree = findSectorTree(sectorID);
        
        if (!tree) {
            cout << "[ERROR] Sector not found" << endl;
            return;
        }
        
        TreeNode<string>* street = findChild(tree->root, streetName);
        if (!street) {
            cout << "[ERROR] Street not found" << endl;
            return;
        }
        
        TreeNode<string>* house = findChild(street, houseNumber);
        if (!house) {
            cout << "[ERROR] House not found" << endl;
            return;
        }

        NaryTree<string>* familyTree = ensureFamilyTree(sectorID, streetName, houseNumber);
        if (!familyTree) {
            cout << "[ERROR] Could not create/find family tree" << endl;
            return;
        }

        TreeNode<string>* existing = familyTree->findNode(familyTree->root, personCNIC);
        if (existing) {
            cout << "[INFO] Member already exists in family tree" << endl;
            return;
        }
        
        familyTree->addChild(familyTree->root, personCNIC);
        cout << "[SUCCESS] Added family member node (" << personCNIC << ") under house " << houseNumber << endl;
    }

    // Establishes a parent-child relationship in a family tree
    // Data Structure: N-ary Tree (tree restructuring - detach and reattach nodes)
    void addParentChildRelation(const string& sectorID, const string& streetName, const string& houseNumber,
        const string& parentCNIC, const string& childCNIC) {
        if (sectorID.empty() || streetName.empty() || houseNumber.empty() || parentCNIC.empty() || childCNIC.empty()) {
            cout << "[ERROR] All parameters (Sector ID, Street name, House number, Parent CNIC, Child CNIC) cannot be empty" << endl;
            return;
        }
        
        if (parentCNIC == childCNIC) {
            cout << "[ERROR] Parent and Child CNIC cannot be the same" << endl;
            return;
        }
        
        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        NaryTree<string>** treePtr = familyTrees.search(houseKey);
        
        if (!treePtr || !*treePtr) {
            cout << "[ERROR] Family tree not found for house. Create house/family first." << endl;
            return;
        }

        NaryTree<string>* familyTree = *treePtr;
        TreeNode<string>* parent = familyTree->findNode(familyTree->root, parentCNIC);
        TreeNode<string>* child = familyTree->findNode(familyTree->root, childCNIC);

        if (!parent) {
            cout << "[ERROR] Parent CNIC not found in this house family tree: " << parentCNIC << endl;
            return;
        }
        
        if (!child) {
            cout << "[ERROR] Child CNIC not found in this house family tree: " << childCNIC << endl;
            return;
        }

        if (child->parent == parent) {
            cout << "[INFO] Parent-child relation already exists." << endl;
            return;
        }

        if (child->parent) {
            TreeNode<string>* detached = removeChildNode(child->parent, child->data);
            if (!detached) {
                cout << "[ERROR] Failed to detach child from its current parent" << endl;
                return;
            }
            attachExistingNode(parent, detached);
            cout << "[SUCCESS] Made " << parentCNIC << " a parent of " << childCNIC << " (subtree preserved)" << endl;
        }
        else {
            TreeNode<string>* detached = removeChildNode(familyTree->root, child->data);
            if (!detached) {
                cout << "[ERROR] Failed to detach child from root" << endl;
                return;
            }
            attachExistingNode(parent, detached);
            cout << "[SUCCESS] Made " << parentCNIC << " a parent of " << childCNIC << endl;
        }
    }

    // Recursively prints a family tree with indentation
    // Data Structure: N-ary Tree (recursive traversal)
    void printFamilyRecursive(TreeNode<string>* currentNode, int depth, NaryTree<string>* familyTree) {
        if (!currentNode) {
            return;
        }

        if (currentNode != familyTree->root) {
            for (int i = 0; i < depth; ++i) {
                cout << "  ";
            }
            cout << "- " << currentNode->data << "\n";
        }

        TreeNode<string>* currentChild = currentNode->firstChild;
        while (currentChild) {
            int nextDepth = depth + (currentNode == familyTree->root ? 0 : 1);
            printFamilyRecursive(currentChild, nextDepth, familyTree);
            currentChild = currentChild->nextSibling;
        }
    }

    // Displays the family tree for a specific house
    // Data Structure: ChainedMap (family tree lookup) + N-ary Tree (recursive display)
    void displayFamilyTreeForHouse(const string& sectorID, const string& streetName, const string& houseNumber) {
        if (sectorID.empty() || streetName.empty() || houseNumber.empty()) {
            cout << "[ERROR] Sector ID, Street name, and House number cannot be empty" << endl;
            return;
        }
        
        string houseKey = makeHouseKey(sectorID, streetName, houseNumber);
        NaryTree<string>** familyTreePtr = familyTrees.search(houseKey);
        
        if (!familyTreePtr || !*familyTreePtr) {
            cout << "[INFO] No family tree for this house (or house not found)" << endl;
            return;
        }

        NaryTree<string>* familyTree = *familyTreePtr;

        cout << endl << "Family Tree for house " << houseNumber
            << " (" << streetName << " - " << sectorID << "):" << endl;
        printFamilyRecursive(familyTree->root, 0, familyTree);
    }

    // Generates age distribution report
    // Data Structure: LinkedList (sector traversal) + N-ary Tree (hierarchy) + ChainedMap (resident lookup)
    void reportAgeDistribution() {
        int age0to12 = 0;
        int age13to18 = 0;
        int age19to30 = 0;
        int age31to50 = 0;
        int age51Plus = 0;
        
        LListNode<NaryTree<string>*>* sector = housingForest.getHead();
        
        while (sector) {
            string sectorID = extractSectorIDFromRoot(sector->data->root->data);
            TreeNode<string>* street = sector->data->root->firstChild;
            
            while (street) {
                TreeNode<string>* house = street->firstChild;
                
                while (house) {
                    string houseKey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(houseKey);
                    
                    if (residents) {
                        LListNode<string>* resident = residents->getHead();
                        
                        while (resident) {
                            Person* person = citizenHash.search(resident->data);
                            if (person) {
                                if (person->age <= 12) {
                                    ++age0to12;
                                }
                                else if (person->age <= 18) {
                                    ++age13to18;
                                }
                                else if (person->age <= 30) {
                                    ++age19to30;
                                }
                                else if (person->age <= 50) {
                                    ++age31to50;
                                }
                                else {
                                    ++age51Plus;
                                }
                            }
                            resident = resident->next;
                        }
                    }
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            sector = sector->next;
        }
        
        cout << endl << "--- Age Distribution ---" << endl;
        cout << "0-12: " << age0to12 << endl;
        cout << "13-18: " << age13to18 << endl;
        cout << "19-30: " << age19to30 << endl;
        cout << "31-50: " << age31to50 << endl;
        cout << "51+: " << age51Plus << endl;
    }

    // Generates occupation summary report
    // Data Structure: ChainedMap (O(1) occupation counting) + LinkedList (unique occupation tracking)
    void reportOccupationSummary() {
        ChainedMap<int> occCount(401);
        LinkedList<string> uniqueOccs;

        LListNode<NaryTree<string>*>* sector = housingForest.getHead();
        
        while (sector) {
            string sectorID = extractSectorIDFromRoot(sector->data->root->data);
            TreeNode<string>* street = sector->data->root->firstChild;
            
            while (street) {
                TreeNode<string>* house = street->firstChild;
                
                while (house) {
                    string houseKey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(houseKey);
                    
                    if (residents) {
                        LListNode<string>* resident = residents->getHead();
                        
                        while (resident) {
                            Person* person = citizenHash.search(resident->data);
                            if (person) {
                                int* count = occCount.search(person->occupation);
                                if (!count) {
                                    occCount.insert(person->occupation, 1);
                                }
                                else {
                                    (*count)++;
                                }

                                if (!containsInList(uniqueOccs, person->occupation)) {
                                    uniqueOccs.insertBack(person->occupation);
                                }
                            }
                            resident = resident->next;
                        }
                    }
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            sector = sector->next;
        }

        cout << endl << "--- Occupation Summary ---" << endl;
        LListNode<string>* occ = uniqueOccs.getHead();
        while (occ) {
            int* count = occCount.search(occ->data);
            int num = count ? *count : 0;
            cout << occ->data << ": " << num << endl;
            occ = occ->next;
        }
    }

    // Generates population density report by sector and street
    // Data Structure: LinkedList (sector traversal) + N-ary Tree (hierarchical counting)
    void reportPopulationDensity() {
        cout << endl << "--- Population Density by Sector & Street ---" << endl;
        LListNode<NaryTree<string>*>* sector = housingForest.getHead();
        
        while (sector) {
            string sectorID = extractSectorIDFromRoot(sector->data->root->data);
            int sectorTotal = 0;
            TreeNode<string>* street = sector->data->root->firstChild;
            
            while (street) {
                int streetCount = 0;
                TreeNode<string>* house = street->firstChild;
                
                while (house) {
                    string houseKey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(houseKey);
                    
                    if (residents) {
                        LListNode<string>* resident = residents->getHead();
                        while (resident) {
                            ++streetCount;
                            ++sectorTotal;
                            resident = resident->next;
                        }
                    }
                    house = house->nextSibling;
                }
                cout << "Street " << street->data << " (" << sectorID << "): " << streetCount << " residents" << endl;
                street = street->nextSibling;
            }
            cout << "Sector " << sectorID << ": " << sectorTotal << " residents" << endl << endl;
            sector = sector->next;
        }
    }

    // Generates gender ratio report
    // Data Structure: LinkedList (traversal) + ChainedMap (citizen lookup)
    void reportGenderRatio() {
        int maleCount = 0;
        int femaleCount = 0;
        int otherCount = 0;
        LListNode<NaryTree<string>*>* sector = housingForest.getHead();
        
        while (sector) {
            string sectorID = extractSectorIDFromRoot(sector->data->root->data);
            TreeNode<string>* street = sector->data->root->firstChild;
            
            while (street) {
                TreeNode<string>* house = street->firstChild;
                
                while (house) {
                    string houseKey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(houseKey);
                    
                    if (residents) {
                        LListNode<string>* resident = residents->getHead();
                        
                        while (resident) {
                            Person* person = citizenHash.search(resident->data);
                            if (person) {
                                string gender = person->gender;
                                if (gender == "M" || gender == "m" || gender == "Male" || gender == "male") {
                                    ++maleCount;
                                }
                                else if (gender == "F" || gender == "f" || gender == "Female" || gender == "female") {
                                    ++femaleCount;
                                }
                                else {
                                    ++otherCount;
                                }
                            }
                            resident = resident->next;
                        }
                    }
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            sector = sector->next;
        }

        cout << endl << "--- Gender Ratio ---" << endl;
        cout << "Male: " << maleCount << endl;
        cout << "Female: " << femaleCount << endl;
        cout << "Other/Unspecified: " << otherCount << endl;
    }

    void displayPopulationHeatmap(int gridSize = 40) {
        cout << "\n=== POPULATION HEATMAP (with Sector Boundaries) ===\n";
        if (!mainGraph) {
            cout << "[ERROR] MainCityGraph not available. Cannot determine sectors.\n";
            return;
        }
        LinkedList<double> allLats;
        LinkedList<double> allLons;
        
        LListNode<NaryTree<string>*>* s = housingForest.getHead();
        if (!s) {
            cout << "[INFO] No sectors loaded. Cannot generate heatmap.\n";
            return;
        }

        int totalHouses = 0;
        while (s) {
            string sectorID = extractSectorIDFromRoot(s->data->root->data);
            TreeNode<string>* street = s->data->root->firstChild;
            while (street) {
                TreeNode<string>* house = street->firstChild;
                while (house) {
                    string hkey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(hkey);
                    int residentCount = 0;
                    if (residents) {
                        LListNode<string>* it = residents->getHead();
                        while (it) {
                            residentCount++;
                            it = it->next;
                        }
                    }

                    if (residentCount > 0) {
                        int streetNo = 1;
                        int houseNo = 1;
                        try { streetNo = stoi(street->data); }
                        catch (...) { streetNo = 1; }
                        try { houseNo = stoi(house->data); }
                        catch (...) { houseNo = 1; }

                        double sLat, sLon, hLat, hLon;
                        getStreetCoordinates(sectorID, streetNo, sLat, sLon);
                        getHouseCoordinates(sLat, sLon, houseNo, hLat, hLon);

                        allLats.insertBack(hLat);
                        allLons.insertBack(hLon);
                        totalHouses++;
                    }
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            s = s->next;
        }

        if (totalHouses == 0) {
            cout << "[INFO] No houses with residents found. Cannot generate heatmap.\n";
            return;
        }

        double minLat = 999.0, maxLat = -999.0;
        double minLon = 999.0, maxLon = -999.0;
        LListNode<double>* latNode = allLats.getHead();
        LListNode<double>* lonNode = allLons.getHead();
        while (latNode && lonNode) {
            if (latNode->data < minLat) minLat = latNode->data;
            if (latNode->data > maxLat) maxLat = latNode->data;
            if (lonNode->data < minLon) minLon = lonNode->data;
            if (lonNode->data > maxLon) maxLon = lonNode->data;
            latNode = latNode->next;
            lonNode = lonNode->next;
        }

        double latRange = maxLat - minLat;
        double lonRange = maxLon - minLon;
        if (latRange < 0.001) latRange = 0.01;
        if (lonRange < 0.001) lonRange = 0.01;
        minLat -= latRange * 0.1;
        maxLat += latRange * 0.1;
        minLon -= lonRange * 0.1;
        maxLon += lonRange * 0.1;
        latRange = maxLat - minLat;
        lonRange = maxLon - minLon;
        ChainedMap<int> gridPopulation;
        ChainedMap<string> gridSector;
        ChainedMap<int> sectorCellCount;
        int maxPopInCell = 0;

        s = housingForest.getHead();
        while (s) {
            string sectorID = extractSectorIDFromRoot(s->data->root->data);
            TreeNode<string>* street = s->data->root->firstChild;
            while (street) {
                TreeNode<string>* house = street->firstChild;
                while (house) {
                    string hkey = makeHouseKey(sectorID, street->data, house->data);
                    LinkedList<string>* residents = houseToResidents.search(hkey);
                    int residentCount = 0;
                    if (residents) {
                        LListNode<string>* it = residents->getHead();
                        while (it) {
                            residentCount++;
                            it = it->next;
                        }
                    }

                    if (residentCount > 0) {
                        int streetNo = 1;
                        int houseNo = 1;
                        try { streetNo = stoi(street->data); }
                        catch (...) { streetNo = 1; }
                        try { houseNo = stoi(house->data); }
                        catch (...) { houseNo = 1; }

                        double sLat, sLon, hLat, hLon;
                        getStreetCoordinates(sectorID, streetNo, sLat, sLon);
                        getHouseCoordinates(sLat, sLon, houseNo, hLat, hLon);

                        // Map to grid cell
                        int row = (int)(((hLat - minLat) / latRange) * gridSize);
                        int col = (int)(((hLon - minLon) / lonRange) * gridSize);
                        if (row < 0) row = 0;
                        if (row >= gridSize) row = gridSize - 1;
                        if (col < 0) col = 0;
                        if (col >= gridSize) col = gridSize - 1;

                        string cellKey = to_string(row) + "_" + to_string(col);
                        int* cellPop = gridPopulation.search(cellKey);
                        if (cellPop) {
                            *cellPop += residentCount;
                            if (*cellPop > maxPopInCell) maxPopInCell = *cellPop;
                        }
                        else {
                            gridPopulation.insert(cellKey, residentCount);
                            if (residentCount > maxPopInCell) maxPopInCell = residentCount;
                        }
                        string sectorKey = cellKey + "_" + sectorID;
                        int* sectorCount = sectorCellCount.search(sectorKey);
                        if (sectorCount) {
                            *sectorCount += residentCount;
                        }
                        else {
                            sectorCellCount.insert(sectorKey, residentCount);
                        }
                    }
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            s = s->next;
        }

        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                string dominantSector = "";
                int maxSectorPop = 0;
                LListNode<NaryTree<string>*>* secIter = housingForest.getHead();
                while (secIter) {
                    string sectorID = extractSectorIDFromRoot(secIter->data->root->data);
                    string sectorKey = cellKey + "_" + sectorID;
                    int* sectorPop = sectorCellCount.search(sectorKey);
                    if (sectorPop && *sectorPop > maxSectorPop) {
                        maxSectorPop = *sectorPop;
                        dominantSector = sectorID;
                    }
                    secIter = secIter->next;
                }

                if (!dominantSector.empty()) {
                    gridSector.insert(cellKey, dominantSector);
                }
            }
        }

        ChainedMap<bool> isBoundary;
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                string* cellSector = gridSector.search(cellKey);
                string cellSec = cellSector ? *cellSector : "";
                bool boundary = false;
                if (r > 0) {
                    string upKey = to_string(r - 1) + "_" + to_string(c);
                    string* upSector = gridSector.search(upKey);
                    if (upSector && *upSector != cellSec) boundary = true;
                }
                if (r < gridSize - 1) {
                    string downKey = to_string(r + 1) + "_" + to_string(c);
                    string* downSector = gridSector.search(downKey);
                    if (downSector && *downSector != cellSec) boundary = true;
                }
                if (c > 0) {
                    string leftKey = to_string(r) + "_" + to_string(c - 1);
                    string* leftSector = gridSector.search(leftKey);
                    if (leftSector && *leftSector != cellSec) boundary = true;
                }
                if (c < gridSize - 1) {
                    string rightKey = to_string(r) + "_" + to_string(c + 1);
                    string* rightSector = gridSector.search(rightKey);
                    if (rightSector && *rightSector != cellSec) boundary = true;
                }

                if (boundary) {
                    isBoundary.insert(cellKey, true);
                }
            }
        }

        if (maxPopInCell == 0) {
            cout << "[INFO] No population data to display." << endl;
            return;
        }
        
        if (!allLats.getHead() || !allLons.getHead()) {
            cout << "[INFO] No coordinate data available for heatmap." << endl;
            return;
        }

        ChainedMap<string> sectorCenters;
        ChainedMap<int> sectorCellPositions;
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                string* sector = gridSector.search(cellKey);
                if (sector && !sector->empty()) {
                    int* count = sectorCellPositions.search(*sector);
                    if (count) {
                        (*count)++;
                    }
                    else {
                        sectorCellPositions.insert(*sector, 1);
                        sectorCenters.insert(*sector, cellKey);
                    }
                }
            }
        }

        cout << "\nPopulation Density Heatmap with Sector Boundaries (Grid: " << gridSize << "x" << gridSize << ")\n";
        cout << "Population Legend: . = 0, : = 1-2, * = 3-5, # = 6-10, @ = 11-20, % = 21+\n";
        cout << "Boundary Legend: | = vertical, - = horizontal, + = intersection/corner\n";
        cout << "Sector labels shown as first letter of sector name in center regions\n";
        cout << "Max population in a cell: " << maxPopInCell << " residents\n\n";
        cout << "     ";
        for (int c = 0; c < gridSize; c++) {
            if (c % 5 == 0) cout << (c % 10);
            else cout << " ";
        }
        cout << "\n";

        ChainedMap<string> sectorLabelPositions;
        LListNode<NaryTree<string>*>* secLabelIter = housingForest.getHead();
        while (secLabelIter) {
            string sectorID = extractSectorIDFromRoot(secLabelIter->data->root->data);
            int minRow = gridSize, maxRow = -1;
            int minCol = gridSize, maxCol = -1;
            int cellCount = 0;
            
            for (int r = 0; r < gridSize; r++) {
                for (int c = 0; c < gridSize; c++) {
                    string cellKey = to_string(r) + "_" + to_string(c);
                    string* sector = gridSector.search(cellKey);
                    if (sector && *sector == sectorID) {
                        if (r < minRow) minRow = r;
                        if (r > maxRow) maxRow = r;
                        if (c < minCol) minCol = c;
                        if (c > maxCol) maxCol = c;
                        cellCount++;
                    }
                }
            }
            if (cellCount > 3 && minRow >= 0 && maxRow >= 0) {
                int centerRow = (minRow + maxRow) / 2;
                int centerCol = (minCol + maxCol) / 2;
                string centerKey = to_string(centerRow) + "_" + to_string(centerCol);
                sectorLabelPositions.insert(centerKey, sectorID);
            }
            
            secLabelIter = secLabelIter->next;
        }
        for (int r = 0; r < gridSize; r++) {
            if (r % 5 == 0) {
                if (r < 10) cout << "  " << r << " ";
                else if (r < 100) cout << " " << r << " ";
                else cout << r << " ";
            }
            else {
                cout << "    ";
            }

            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                int* pop = gridPopulation.search(cellKey);
                int cellPop = pop ? *pop : 0;
                bool* boundary = isBoundary.search(cellKey);
                bool isBound = boundary ? *boundary : false;
                string* sector = gridSector.search(cellKey);
                string* labelSector = sectorLabelPositions.search(cellKey);
                if (labelSector && !labelSector->empty()) {
                    string label = *labelSector;
                    if (label.length() > 4) label = label.substr(0, 4);
                    char sectorChar = label[0];
                    if (sectorChar >= 'A' && sectorChar <= 'Z') {
                        cout << sectorChar;
                    }
                    else {
                        cout << (label.length() > 0 ? label[0] : '?');
                    }
                }
                else {
                    char symbol = ' ';
                    if (cellPop == 0) symbol = '.';
                    else if (cellPop <= 2) symbol = ':';
                    else if (cellPop <= 5) symbol = '*';
                    else if (cellPop <= 10) symbol = '#';
                    else if (cellPop <= 20) symbol = '@';
                    else symbol = '%';
                    if (isBound && cellPop > 0 && sector) {
                        bool verticalBoundary = false;
                        bool horizontalBoundary = false;
                        
                        if (r > 0) {
                            string upKey = to_string(r - 1) + "_" + to_string(c);
                            string* upSector = gridSector.search(upKey);
                            if (upSector && *upSector != *sector && !upSector->empty()) horizontalBoundary = true;
                        }
                        if (r < gridSize - 1) {
                            string downKey = to_string(r + 1) + "_" + to_string(c);
                            string* downSector = gridSector.search(downKey);
                            if (downSector && *downSector != *sector && !downSector->empty()) horizontalBoundary = true;
                        }
                        if (c > 0) {
                            string leftKey = to_string(r) + "_" + to_string(c - 1);
                            string* leftSector = gridSector.search(leftKey);
                            if (leftSector && *leftSector != *sector && !leftSector->empty()) verticalBoundary = true;
                        }
                        if (c < gridSize - 1) {
                            string rightKey = to_string(r) + "_" + to_string(c + 1);
                            string* rightSector = gridSector.search(rightKey);
                            if (rightSector && *rightSector != *sector && !rightSector->empty())                             verticalBoundary = true;
                        }
                        if (verticalBoundary || horizontalBoundary) {
                            if (verticalBoundary && horizontalBoundary) {
                                symbol = '+';
                            }
                            else if (verticalBoundary) {
                                symbol = '|';
                            }
                            else {
                                symbol = '-';
                            }
                        }
                    }
                    else if (isBound && cellPop == 0) {
                        symbol = '.';
                    }

                    cout << symbol;
                }
            }
            cout << "\n";
        }

        cout << "\n--- Sector Legend (Letters in heatmap) ---\n";
        ChainedMap<bool> sectorsShown; // Track which sectors we've shown
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                string* labelSector = sectorLabelPositions.search(cellKey);
                if (labelSector && !labelSector->empty()) {
                    bool* shown = sectorsShown.search(*labelSector);
                    if (!shown) {
                        sectorsShown.insert(*labelSector, true);
                        char sectorChar = labelSector->length() > 0 ? (*labelSector)[0] : '?';
                        cout << "  " << sectorChar << " = " << *labelSector << "\n";
                    }
                }
            }
        }

        ChainedMap<int> sectorPopCount;
        ChainedMap<int> sectorCellCountFinal;
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                string* sector = gridSector.search(cellKey);
                int* pop = gridPopulation.search(cellKey);
                if (sector && pop) {
                    int* secPop = sectorPopCount.search(*sector);
                    if (secPop) {
                        *secPop += *pop;
                    }
                    else {
                        sectorPopCount.insert(*sector, *pop);
                    }

                    int* secCells = sectorCellCountFinal.search(*sector);
                    if (secCells) {
                        (*secCells)++;
                    }
                    else {
                        sectorCellCountFinal.insert(*sector, 1);
                    }
                }
            }
        }

        int emptyCells = 0;
        int populatedCells = 0;
        int totalGridPop = 0;
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                string cellKey = to_string(r) + "_" + to_string(c);
                int* pop = gridPopulation.search(cellKey);
                if (pop && *pop > 0) {
                    populatedCells++;
                    totalGridPop += *pop;
                }
                else {
                    emptyCells++;
                }
            }
        }

        cout << "\n--- Heatmap Statistics ---\n";
        cout << "Total grid cells: " << (gridSize * gridSize) << "\n";
        cout << "Populated cells: " << populatedCells << "\n";
        cout << "Empty cells: " << emptyCells << "\n";
        cout << "Total population mapped: " << totalGridPop << " residents\n";
        cout << "Average per populated cell: " << (populatedCells > 0 ? (double)totalGridPop / populatedCells : 0.0) << "\n";
        cout << "Coordinate bounds: Lat[" << minLat << " to " << maxLat << "], Lon[" << minLon << " to " << maxLon << "]\n";

        cout << "\n--- Sector Breakdown ---\n";
        LListNode<NaryTree<string>*>* secIter2 = housingForest.getHead();
        while (secIter2) {
            string sectorID = extractSectorIDFromRoot(secIter2->data->root->data);
            int* secPop = sectorPopCount.search(sectorID);
            int* secCells = sectorCellCountFinal.search(sectorID);
            int pop = secPop ? *secPop : 0;
            int cells = secCells ? *secCells : 0;
            if (pop > 0 || cells > 0) {
                cout << "Sector " << sectorID << ": " << pop << " residents in " << cells << " cells";
                if (cells > 0) {
                    cout << " (avg: " << ((double)pop / cells) << " per cell)";
                }
                cout << "\n";
            }
            secIter2 = secIter2->next;
        }
    }

    void displayHousingHierarchy(const string& sectorID = "") {
        cout << "\n=== HOUSING HIERARCHY ===\n";
        if (sectorID != "") {
            NaryTree<string>* t = findSectorTree(sectorID);
            if (!t) { cout << "[ERROR] Sector not found\n"; return; }
            cout << t->root->data << "\n";
            TreeNode<string>* street = t->root->firstChild;
            while (street) {
                cout << "  - Street: " << street->data << "\n";
                TreeNode<string>* house = street->firstChild;
                while (house) {
                    cout << "     * House: " << house->data << "\n";
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            return;
        }

        LListNode<NaryTree<string>*>* s = housingForest.getHead();
        while (s) {
            cout << s->data->root->data << "\n";
            TreeNode<string>* street = s->data->root->firstChild;
            while (street) {
                cout << "  - Street: " << street->data << "\n";
                TreeNode<string>* house = street->firstChild;
                while (house) {
                    cout << "     * House: " << house->data << "\n";
                    house = house->nextSibling;
                }
                street = street->nextSibling;
            }
            s = s->next;
        }
    }

    void loadPopulationCSV(const string& filename) {
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
        bool first = true;
        int lineNum = 0;
        int successCount = 0;
        int errorCount = 0;
        
        while (getline(file, line)) {
            lineNum++;
            if (first) {
                first = false;
                continue;
            }
            
            if (line.empty() || line.size() < 3) {
                continue;
            }

            string parts[7];
            parseCSVLine(line, parts, 7);

            string cnic = trimCopy(parts[0]);
            string name = trimCopy(parts[1]);
            int age = 0;
            try { 
                string ageStr = trimCopy(parts[2]);
                if (!ageStr.empty()) {
                    age = stoi(ageStr);
                    if (age < 0 || age > 150) {
                        cout << "[WARN] Line " << lineNum << ": Invalid age " << age << " for " << cnic << ", skipping" << endl;
                        errorCount++;
                        continue;
                    }
                }
            }
            catch (...) { 
                cout << "[WARN] Line " << lineNum << ": Could not parse age for " << cnic << ", using 0" << endl;
                age = 0;
            }
            string sector = trimCopy(parts[3]);
            string streetPlain = trimCopy(parts[4]);
            string housePlain = trimCopy(parts[5]);
            string occupation = trimCopy(parts[6]);

            if (cnic.empty() || name.empty()) {
                cout << "[WARN] Line " << lineNum << ": Missing CNIC or Name, skipping" << endl;
                errorCount++;
                continue;
            }
            
            if (sector.empty() || streetPlain.empty() || housePlain.empty()) {
                cout << "[WARN] Line " << lineNum << ": Missing address information for " << cnic << ", skipping" << endl;
                errorCount++;
                continue;
            }

            // ensure sector exists in mainGraph and in our forest
            double tmpLat, tmpLon;
            if (mainGraph && !mainGraph->getSectorSystem().getCoordinatesInSector(sector, tmpLat, tmpLon)) {
                cout << "[WARN] CSV: sector '" << sector << "' not found in MainCityGraph � skipping line for " << cnic << endl;
                continue;
            }
            if (!findSectorTree(sector)) addSector(sector);
            NaryTree<string>* t = findSectorTree(sector);
            if (!t) { cout << "[ERROR] Could not create/find sector: " << sector << endl; continue; }

            TreeNode<string>* strNode = findChild(t->root, streetPlain);
            if (!strNode) addStreet(sector, streetPlain);
            TreeNode<string>* streetNode = findChild(t->root, streetPlain);
            TreeNode<string>* foundHouse = nullptr;
            if (streetNode) foundHouse = findChild(streetNode, housePlain);
            if (!foundHouse) addHouse(sector, streetPlain, housePlain);
            registerCitizen(cnic, name, age, "", sector, streetPlain, housePlain, occupation);
        }

        file.close();
        cout << "[SUCCESS] Population CSV loaded\n";
    }

    void removeHouseGraphNode(const string& sector, const string& streetPlain, const string& housePlain) {
    }

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
            cout << BRIGHT_YELLOW << BOLD << "         POPULATION & HOUSING MANAGEMENT SYSTEM" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "============================================================" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  STRUCTURE MANAGEMENT" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  1. Add Sector         2. Add Street         3. Add House" << RESET << endl;
            cout << BRIGHT_WHITE << "  4. Delete Sector      5. Delete Street       6. Delete House" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  CITIZEN MANAGEMENT" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  7. Register Citizen   8. Find Citizen       9. Update Resident" << RESET << endl;
            cout << BRIGHT_WHITE << " 10. Delete Citizen    11. Print All Residents" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  RESIDENT QUERIES" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 12. List by House     13. List by Street    14. List by Sector" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  FAMILY RELATIONSHIPS" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 15. Add Family Member 16. Set Parent-Child   17. View Family Tree" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  REPORTS & VISUALIZATION" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 18. Housing Hierarchy 19. Age Distribution  20. Occupation Summary" << RESET << endl;
            cout << BRIGHT_WHITE << " 21. Population Density 22. Gender Ratio      23. Population Heatmap" << RESET << endl << endl;
            
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  DATA IMPORT" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 24. Load Population CSV" << RESET << endl << endl;
            
            cout << BRIGHT_WHITE << BOLD << "  0. Back to Main Menu" << RESET << endl;
            cout << YELLOW << "------------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "Choose: " << RESET;
            int ch;
            cin >> ch;
            cin.ignore();

            if (ch == 0) return;

            string sector, street, houseNo;
            string cnic, name, gender, occ;
            int age;

            switch (ch) {
            case 1:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add Sector]" << RESET << endl << CYAN << "Sector ID: " << RESET;
                getline(cin, sector);
                addSector(trimCopy(sector));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 2:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add Street]" << RESET << endl << CYAN << "Sector ID: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                addStreet(trimCopy(sector), trimCopy(street));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 3:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add House]" << RESET << endl << CYAN << "Sector ID: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                addHouse(trimCopy(sector), trimCopy(street), trimCopy(houseNo));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 4:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Sector]" << RESET << endl << CYAN << "Sector ID: " << RESET;
                getline(cin, sector);
                deleteSector(trimCopy(sector));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 5:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Street]" << RESET << endl << CYAN << "Sector ID: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                deleteStreet(trimCopy(sector), trimCopy(street));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 6:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete House]" << RESET << endl << CYAN << "Sector ID: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                deleteHouse(trimCopy(sector), trimCopy(street), trimCopy(houseNo));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 7:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Register Citizen]" << RESET << endl << CYAN << "CNIC: " << RESET;
                getline(cin, cnic);
                cout << CYAN << "Name: " << RESET;
                getline(cin, name);
                cout << CYAN << "Age: " << RESET;
                cin >> age;
                cin.ignore();
                cout << CYAN << "Gender (M/F or leave blank): " << RESET;
                getline(cin, gender);
                cout << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                cout << CYAN << "Occupation: " << RESET;
                getline(cin, occ);
                registerCitizen(trimCopy(cnic), trimCopy(name), age, trimCopy(gender), trimCopy(sector), trimCopy(street), trimCopy(houseNo), trimCopy(occ));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 8:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Find Citizen]" << RESET << endl << CYAN << "CNIC: " << RESET;
                getline(cin, cnic);
                printResidentByCNIC(trimCopy(cnic));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 9:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Update Resident]" << RESET << endl << CYAN << "CNIC to update: " << RESET;
                getline(cin, cnic);
                cout << CYAN << "New name: " << RESET;
                getline(cin, name);
                cout << CYAN << "New age: " << RESET;
                cin >> age;
                cin.ignore();
                cout << CYAN << "New gender (M/F or blank): " << RESET;
                getline(cin, gender);
                cout << CYAN << "New sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "New street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "New house number: " << RESET;
                getline(cin, houseNo);
                cout << CYAN << "New occupation: " << RESET;
                getline(cin, occ);
                updateResident(trimCopy(cnic), trimCopy(name), age, trimCopy(gender), trimCopy(sector), trimCopy(street), trimCopy(houseNo), trimCopy(occ));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 10:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Delete Citizen]" << RESET << endl << CYAN << "CNIC: " << RESET;
                getline(cin, cnic);
                deleteCitizen(trimCopy(cnic));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 11:
                printAllResidents();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 12:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[List Residents by House]" << RESET << endl << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                listResidentsInHouse(trimCopy(sector), trimCopy(street), trimCopy(houseNo));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 13:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[List Residents by Street]" << RESET << endl << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                listResidentsInStreet(trimCopy(sector), trimCopy(street));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 14:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[List Residents by Sector]" << RESET << endl << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                listResidentsInSector(trimCopy(sector));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 15:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Add Family Member]" << RESET << endl << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                cout << CYAN << "CNIC to add: " << RESET;
                getline(cin, cnic);
                addFamilyMemberToHouse(trimCopy(sector), trimCopy(street), trimCopy(houseNo), trimCopy(cnic));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 16:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Set Parent-Child Relation]" << RESET << endl << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                cout << CYAN << "Parent CNIC: " << RESET;
                getline(cin, name);
                cout << CYAN << "Child CNIC: " << RESET;
                getline(cin, cnic);
                addParentChildRelation(trimCopy(sector), trimCopy(street), trimCopy(houseNo), trimCopy(name), trimCopy(cnic));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 17:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[View Family Tree]" << RESET << endl << CYAN << "Sector: " << RESET;
                getline(cin, sector);
                cout << CYAN << "Street number: " << RESET;
                getline(cin, street);
                cout << CYAN << "House number: " << RESET;
                getline(cin, houseNo);
                displayFamilyTreeForHouse(trimCopy(sector), trimCopy(street), trimCopy(houseNo));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 18:
                displayHousingHierarchy();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 19:
                reportAgeDistribution();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 20:
                reportOccupationSummary();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 21:
                reportPopulationDensity();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 22:
                reportGenderRatio();
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 23:
                {
                    cout << "\n" << BRIGHT_YELLOW << BOLD << "[Population Heatmap]" << RESET << endl << CYAN << "Grid size (default 40, recommended 30-50): " << RESET;
                    string gridInput;
                    getline(cin, gridInput);
                    int gridSize = 40;
                    if (!gridInput.empty()) {
                        try {
                            gridSize = stoi(trimCopy(gridInput));
                        }
                        catch (...) {
                            gridSize = 40;
                        }
                        if (gridSize < 10) gridSize = 10;
                        if (gridSize > 80) gridSize = 80;
                    }
                    displayPopulationHeatmap(gridSize);
                }
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 24:
                cout << "\n" << BRIGHT_YELLOW << BOLD << "[Load Population CSV]" << RESET << endl << CYAN << "CSV filename: " << RESET;
                getline(cin, sector);
                loadPopulationCSV(trimCopy(sector));
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 0:
                return;
            default:
                cout << "\n" << BRIGHT_RED << BOLD << "[ERROR]" << RESET << BRIGHT_WHITE << " Invalid choice. Please select a number from the menu." << RESET << endl;
                cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
            }
        }
    }
};



