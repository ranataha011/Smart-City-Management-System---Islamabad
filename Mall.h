#pragma once
#include "CityEntities.h"
#include "MainCityGraph.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

// ==========================================
// COMMERCIAL ENTITY STRUCTURES
// ==========================================

struct Product {
    string productID;
    string name;
    string category;
    string brand;
    double price;
    int quantity;
    string storeID;
    string mallID;

    Product() : price(0.0), quantity(0) {}

    Product(string id, string n, string cat, string br,
        double p, int q, string store, string mall)
        : productID(id), name(n), category(cat), brand(br),
        price(p), quantity(q), storeID(store), mallID(mall) {
    }
};

struct Store {
    string storeID;
    string name;
    string category;
    int floorNumber;
    string mallID;

    Store() : floorNumber(1) {}

    Store(string id, string n, string cat, int floor, string mall)
        : storeID(id), name(n), category(cat),
        floorNumber(floor), mallID(mall) {
    }
};

struct Mall {
    string mallID;
    string name;
    string sector;
    Coordinates coords;
    int totalFloors;
    int totalStores;
    string primaryCategory;

    LinkedList<string> storeIDs;

    Mall() : totalFloors(3), totalStores(0) {}

    Mall(string id, string n, string sec, double lat, double lon,
        int floors, string category)
        : mallID(id), name(n), sector(sec), coords(lat, lon),
        totalFloors(floors), totalStores(0), primaryCategory(category) {
    }
};

// ==========================================
// SHOPPING CART SYSTEM (Linked List)
// ==========================================

struct CartItem {
    Product* product;
    int quantity;
    CartItem* next;

    CartItem(Product* p, int q) : product(p), quantity(q), next(nullptr) {}
};

class ShoppingCart {
private:
    CartItem* head;
    CartItem* tail;
    int itemCount;
    double totalAmount;

public:
    ShoppingCart() : head(nullptr), tail(nullptr), itemCount(0), totalAmount(0.0) {}

    ~ShoppingCart() {
        clearCart();
    }

    void addToCart(Product* product, int quantity) {
        if (!product || quantity <= 0) return;
        if (quantity > product->quantity) return;

        CartItem* current = head;
        while (current != nullptr) {
            if (current->product->productID == product->productID) {
                current->quantity += quantity;
                totalAmount += product->price * quantity;
                return;
            }
            current = current->next;
        }

        CartItem* newItem = new CartItem(product, quantity);
        if (!head) head = tail = newItem;
        else {
            tail->next = newItem;
            tail = newItem;
        }
        itemCount++;
        totalAmount += product->price * quantity;
    }

    void removeFromCart(string productID) {
        CartItem* current = head;
        CartItem* prev = nullptr;

        while (current != nullptr) {
            if (current->product->productID == productID) {
                totalAmount -= current->product->price * current->quantity;

                if (prev == nullptr) head = current->next;
                else prev->next = current->next;

                if (current == tail) tail = prev;

                delete current;
                itemCount--;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

    void updateQuantity(string productID, int newQuantity) {
        CartItem* current = head;
        while (current != nullptr) {
            if (current->product->productID == productID) {
                totalAmount -= current->product->price * current->quantity;
                current->quantity = newQuantity;
                totalAmount += current->product->price * newQuantity;
                return;
            }
            current = current->next;
        }
    }

    void displayCart() {
        if (!head) {
            cout << "\nShopping cart is empty" << endl;
            return;
        }

        cout << "\n=== SHOPPING CART ===" << endl;
        cout << "Items: " << itemCount << " | Total: $" << totalAmount << endl;
        cout << "----------------------------------------" << endl;

        CartItem* current = head;
        int index = 1;
        while (current != nullptr) {
            double subtotal = current->product->price * current->quantity;
            cout << index++ << ". " << current->product->name << endl;
            cout << "   Brand: " << current->product->brand << endl;
            cout << "   Price: $" << current->product->price << " x " << current->quantity
                << " = $" << subtotal << endl;
            current = current->next;
        }
    }

    void clearCart() {
        CartItem* current = head;
        while (current != nullptr) {
            CartItem* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        itemCount = 0;
        totalAmount = 0.0;
    }

    double getTotalAmount() { return totalAmount; }
    int getItemCount() { return itemCount; }
    CartItem* getHead() { return head; }
    bool isEmpty() { return head == nullptr; }
};

// ==========================================
// COMMERCIAL SECTOR MAIN CLASS
// ==========================================

class CommercialSector {
private:
    MainCityGraph* cityGraph;

    HashTable<Mall> mallsHash;
    HashTable<Store> storesHash;
    HashTable<Product> productsHash;

    HashTable<LinkedList<string>> productsByCategory;
    HashTable<LinkedList<string>> productsByName;
    HashTable<LinkedList<string>> productsByBrand;
    HashTable<LinkedList<string>> storesByCategory;
    HashTable<LinkedList<string>> storesByMall;

    ShoppingCart shoppingCart;

    int mallCount;
    int storeCount;
    int productCount;

    string intToString(int num) {
        if (num == 0) return "0";
        string result = "";
        while (num > 0) {
            char digit = '0' + (num % 10);
            result = digit + result;
            num /= 10;
        }
        return result;
    }

    string toLowerCase(string str) {
        string result = str;
        for (size_t i = 0; i < result.length(); i++) {
            if (result[i] >= 'A' && result[i] <= 'Z') {
                result[i] = result[i] + 32;
            }
        }
        return result;
    }

    void generateRandomCoordinatesInSector(string sector, double& lat, double& lon) {
        static bool seeded = false;
        if (!seeded) {
            srand((unsigned int)time(0));
            seeded = true;
        }

        if (cityGraph->getSectorSystem().getCoordinatesInSector(sector, lat, lon)) {
            double latRange = 0.001;
            double lonRange = 0.001;
            lat += ((rand() % 1000) / 1000.0 - 0.5) * latRange;
            lon += ((rand() % 1000) / 1000.0 - 0.5) * lonRange;
        }
        else {
            if (sector.find("F-") != string::npos) {
                lat = 33.70 + ((rand() % 100) / 1000.0);
                lon = 73.03 + ((rand() % 100) / 1000.0);
            }
            else if (sector.find("G-") != string::npos) {
                lat = 33.69 + ((rand() % 100) / 1000.0);
                lon = 73.04 + ((rand() % 100) / 1000.0);
            }
            else if (sector == "Blue Area") {
                lat = 33.720 + ((rand() % 50) / 1000.0);
                lon = 73.065 + ((rand() % 50) / 1000.0);
            }
            else {
                lat = 33.684 + ((rand() % 200) / 1000.0);
                lon = 73.047 + ((rand() % 200) / 1000.0);
            }
        }
    }

    void connectTempNodeToNetwork(string tempNodeID, double lat, double lon) {
        int connectionsMade = 0;
        const int MAX_CONNECTIONS = 3;

        LListNode<string>* allNodes = cityGraph->getAllNodeIDs().getHead();

        while (allNodes != nullptr && connectionsMade < MAX_CONNECTIONS) {
            string nodeID = allNodes->data;

            if (nodeID == tempNodeID) {
                allNodes = allNodes->next;
                continue;
            }

            CityNode* otherNode = cityGraph->getNode(nodeID);
            if (otherNode) {
                int distanceKm = 1;
                cityGraph->connectNodes(tempNodeID, nodeID, distanceKm);
                connectionsMade++;
            }
            allNodes = allNodes->next;
        }
    }

public:
    CommercialSector(MainCityGraph* graph)
        : cityGraph(graph),
        mallsHash(100),
        storesHash(200),
        productsHash(500),
        productsByCategory(50),
        productsByName(100),
        productsByBrand(50),
        storesByCategory(50),
        storesByMall(50),
        mallCount(0),
        storeCount(0),
        productCount(0) {
    }

    string generateMallID() {
        mallCount++;
        string id = "MALL";
        if (mallCount < 10) id += "0";
        id += intToString(mallCount);
        return id;
    }

    string generateStoreID() {
        storeCount++;
        string id = "STORE";
        if (storeCount < 10) id += "0" + intToString(storeCount);
        else id += intToString(storeCount);
        return id;
    }

    string generateProductID() {
        productCount++;
        string id = "PROD";
        if (productCount < 10) id += "00" + intToString(productCount);
        else if (productCount < 100) id += "0" + intToString(productCount);
        else id += intToString(productCount);
        return id;
    }

    void registerMall(string name, string sector, int totalFloors, string primaryCategory = "General") {
        string mallID = generateMallID();
        double lat, lon;

        generateRandomCoordinatesInSector(sector, lat, lon);

        Mall* mall = new Mall(mallID, name, sector, lat, lon, totalFloors, primaryCategory);
        mallsHash.insert(mallID, *mall);

        // Use the existing auto-connect method
        cityGraph->addNodeWithAutoConnect(mallID, MODULE_COMMERCIAL, name,
            lat, lon, sector, mall);

        // Simple approach: Just connect to all other malls in same sector (within 5km)
        // The auto-connect should handle duplicates
        LListNode<string>* allNodes = cityGraph->getAllNodeIDs().getHead();
        int sameSectorConnections = 0;

        cout << "\n[SECTOR-CONNECT] Checking for other malls in " << sector << " sector..." << endl;

        while (allNodes != nullptr) {
            string otherID = allNodes->data;

            // Only connect to other malls
            if (otherID != mallID && otherID.find("MALL") == 0) {
                Mall* otherMall = mallsHash.search(otherID);

                if (otherMall && otherMall->sector == sector) {
                    // Simple distance check
                    double latDiff = lat - otherMall->coords.latitude;
                    double lonDiff = lon - otherMall->coords.longitude;
                    double distanceKm = 111.0 * sqrt((latDiff * latDiff) + (lonDiff * lonDiff));

                    if (distanceKm <= 5.0) {
                        int distIntKm = static_cast<int>(distanceKm + 0.5);
                        if (distIntKm < 1) distIntKm = 1;

                        // Just connect - duplicates won't hurt
                        cityGraph->connectNodes(mallID, otherID, distIntKm);
                        sameSectorConnections++;

                        cout << "[SECTOR-CONNECT] Connected to " << otherMall->name
                            << " (" << distIntKm << " km)" << endl;
                    }
                }
            }
            allNodes = allNodes->next;
        }

        cout << "[SECTOR-CONNECT] Made " << sameSectorConnections
            << " connection(s) to malls in same sector" << endl;
        cout << "[SUCCESS] Mall " << name << " registered with ID: " << mallID << endl;
    }

    void registerMallManual() {
        string name, sector, category;
        int totalFloors;

        cout << "\n=== REGISTER NEW MALL ===\n";

        cout << "Enter Mall Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Sector (e.g., F-8, G-10, Blue Area): ";
        getline(cin, sector);

        cout << "Enter Primary Category (Luxury, Family, Budget, etc.): ";
        getline(cin, category);

        cout << "Enter Total Floors: ";
        cin >> totalFloors;

        registerMall(name, sector, totalFloors, category);
        cout << "Mall registered: " << name << endl;
    }

    void registerStore(string name, string category, int floorNumber, string mallID) {
        Mall* mall = mallsHash.search(mallID);
        if (!mall) {
            cout << "Mall not found" << endl;
            return;
        }

        string storeID = generateStoreID();
        Store store(storeID, name, category, floorNumber, mallID);

        storesHash.insert(storeID, store);
        mall->storeIDs.insertBack(storeID);
        mall->totalStores++;
        mallsHash.insert(mallID, *mall);

        string lowerCategory = toLowerCase(category);
        LinkedList<string>* catList = storesByCategory.search(lowerCategory);
        if (catList) catList->insertBack(storeID);
        else {
            LinkedList<string> newList;
            newList.insertBack(storeID);
            storesByCategory.insert(lowerCategory, newList);
        }

        LinkedList<string>* mallList = storesByMall.search(mallID);
        if (mallList) mallList->insertBack(storeID);
        else {
            LinkedList<string> newList;
            newList.insertBack(storeID);
            storesByMall.insert(mallID, newList);
        }
    }

    void registerStoreManual() {
        string name, category, mallID;
        int floorNumber;

        cout << "\n=== REGISTER NEW STORE ===\n";

        displayAllMalls();

        cout << "\nEnter Store Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Category: ";
        getline(cin, category);

        cout << "Enter Mall ID: ";
        getline(cin, mallID);

        cout << "Enter Floor Number: ";
        cin >> floorNumber;

        registerStore(name, category, floorNumber, mallID);
        cout << "Store registered: " << name << endl;
    }

    void addProduct(string name, string category, string brand,
        double price, int quantity, string storeID) {
        Store* store = storesHash.search(storeID);
        if (!store) {
            cout << "Store not found" << endl;
            return;
        }

        string productID = generateProductID();
        Product product(productID, name, category, brand, price, quantity, storeID, store->mallID);

        productsHash.insert(productID, product);

        string lowerName = toLowerCase(name);
        LinkedList<string>* nameList = productsByName.search(lowerName);
        if (nameList) nameList->insertBack(productID);
        else {
            LinkedList<string> newList;
            newList.insertBack(productID);
            productsByName.insert(lowerName, newList);
        }

        string lowerCategory = toLowerCase(category);
        LinkedList<string>* catList = productsByCategory.search(lowerCategory);
        if (catList) catList->insertBack(productID);
        else {
            LinkedList<string> newList;
            newList.insertBack(productID);
            productsByCategory.insert(lowerCategory, newList);
        }

        string lowerBrand = toLowerCase(brand);
        LinkedList<string>* brandList = productsByBrand.search(lowerBrand);
        if (brandList) brandList->insertBack(productID);
        else {
            LinkedList<string> newList;
            newList.insertBack(productID);
            productsByBrand.insert(lowerBrand, newList);
        }
    }

    void addProductManual() {
        string name, category, brand, storeID;
        double price;
        int quantity;

        cout << "\n=== ADD NEW PRODUCT ===\n";

        displayAllStores();

        cout << "\nEnter Product Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Category: ";
        getline(cin, category);

        cout << "Enter Brand: ";
        getline(cin, brand);

        cout << "Enter Price: $";
        cin >> price;

        cout << "Enter Quantity: ";
        cin >> quantity;

        cout << "Enter Store ID: ";
        cin.ignore();
        getline(cin, storeID);

        addProduct(name, category, brand, price, quantity, storeID);
        cout << "Product added: " << name << endl;
    }

    void searchProductByName(string name) {
        cout << "\n=== SEARCH PRODUCT BY NAME ===" << endl;
        cout << "Searching for: " << name << endl;

        string lowerName = toLowerCase(name);
        LinkedList<string>* productList = productsByName.search(lowerName);

        if (!productList) {
            cout << "No products found" << endl;
            return;
        }

        LListNode<string>* current = productList->getHead();
        int count = 0;

        while (current != nullptr) {
            Product* product = productsHash.search(current->data);
            if (product) {
                count++;
                Store* store = storesHash.search(product->storeID);

                cout << count << ". " << product->name << endl;
                cout << "   Brand: " << product->brand << endl;
                cout << "   Price: $" << product->price << endl;
                cout << "   Stock: " << product->quantity << endl;
                if (store) cout << "   Store: " << store->name << endl;
                cout << endl;
            }
            current = current->next;
        }

        cout << "Total found: " << count << endl;
    }

    void searchProductByCategory(string category) {
        cout << "\n=== SEARCH PRODUCT BY CATEGORY ===" << endl;
        cout << "Category: " << category << endl;

        string lowerCategory = toLowerCase(category);
        LinkedList<string>* productList = productsByCategory.search(lowerCategory);

        if (!productList) {
            cout << "No products found" << endl;
            return;
        }

        LListNode<string>* current = productList->getHead();
        int count = 0;

        while (current != nullptr) {
            Product* product = productsHash.search(current->data);
            if (product) {
                count++;
                cout << count << ". " << product->name << endl;
                cout << "   Brand: " << product->brand << endl;
                cout << "   Price: $" << product->price << endl;
                cout << endl;
            }
            current = current->next;
        }

        cout << "Total found: " << count << endl;
    }

    void searchProductByBrand(string brand) {
        cout << "\n=== SEARCH PRODUCT BY BRAND ===" << endl;
        cout << "Brand: " << brand << endl;

        string lowerBrand = toLowerCase(brand);
        LinkedList<string>* productList = productsByBrand.search(lowerBrand);

        if (!productList) {
            cout << "No products found" << endl;
            return;
        }

        LListNode<string>* current = productList->getHead();
        int count = 0;

        while (current != nullptr) {
            Product* product = productsHash.search(current->data);
            if (product) {
                count++;
                cout << count << ". " << product->name << endl;
                cout << "   Category: " << product->category << endl;
                cout << "   Price: $" << product->price << endl;
                cout << endl;
            }
            current = current->next;
        }

        cout << "Total found: " << count << endl;
    }

    void searchStoreByCategory(string category) {
        cout << "\n=== STORES BY CATEGORY ===" << endl;
        cout << "Category: " << category << endl;

        string lowerCategory = toLowerCase(category);
        LinkedList<string>* storeList = storesByCategory.search(lowerCategory);

        if (!storeList) {
            cout << "No stores found" << endl;
            return;
        }

        LListNode<string>* current = storeList->getHead();
        int count = 0;

        while (current != nullptr) {
            Store* store = storesHash.search(current->data);
            if (store) {
                count++;
                Mall* mall = mallsHash.search(store->mallID);

                cout << count << ". " << store->name << endl;
                cout << "   Floor: " << store->floorNumber << endl;
                if (mall) cout << "   Mall: " << mall->name << endl;
                cout << endl;
            }
            current = current->next;
        }

        cout << "Total found: " << count << endl;
    }

    void addToCart() {
        string productID;
        int quantity;

        cout << "\n=== ADD TO SHOPPING CART ===\n";

        cout << "Enter Product ID: ";
        cin >> productID;

        Product* product = productsHash.search(productID);
        if (!product) {
            cout << "Product not found" << endl;
            return;
        }

        cout << "Product: " << product->name << endl;
        cout << "Available stock: " << product->quantity << endl;

        cout << "Enter quantity: ";
        cin >> quantity;

        shoppingCart.addToCart(product, quantity);
        cout << "Added to cart" << endl;
    }

    void viewCart() {
        shoppingCart.displayCart();
    }

    void updateCartQuantity() {
        string productID;
        int newQuantity;

        cout << "\n=== UPDATE CART QUANTITY ===\n";

        if (shoppingCart.isEmpty()) {
            cout << "Cart is empty" << endl;
            return;
        }

        cout << "Enter Product ID: ";
        cin >> productID;

        cout << "Enter new quantity: ";
        cin >> newQuantity;

        shoppingCart.updateQuantity(productID, newQuantity);
    }

    void removeFromCart() {
        string productID;

        cout << "\n=== REMOVE FROM CART ===\n";

        if (shoppingCart.isEmpty()) {
            cout << "Cart is empty" << endl;
            return;
        }

        cout << "Enter Product ID: ";
        cin >> productID;

        shoppingCart.removeFromCart(productID);
        cout << "Removed from cart" << endl;
    }

    void checkout() {
        cout << "\n=== CHECKOUT ===\n";

        if (shoppingCart.isEmpty()) {
            cout << "Cart is empty" << endl;
            return;
        }

        shoppingCart.displayCart();

        cout << "\nProceed with checkout? (y/n): ";
        char choice;
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            CartItem* cartItem = shoppingCart.getHead();
            while (cartItem != nullptr) {
                Product* product = cartItem->product;
                product->quantity -= cartItem->quantity;
                productsHash.insert(product->productID, *product);
                cartItem = cartItem->next;
            }

            cout << "Checkout completed!" << endl;
            shoppingCart.clearCart();
        }
    }

    void clearCart() {
        shoppingCart.clearCart();
        cout << "Cart cleared" << endl;
    }

    void findNearestMall(double currentLat, double currentLon, string requiredCategory = "") {
        cout << "\n=== FINDING NEAREST MALL ===" << endl;

        string nearestMallID = "";
        double minDistance = 999999.0;
        Mall* nearestMall = nullptr;

        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();

        while (nodeIDList != nullptr) {
            CityNode* cityNode = cityGraph->getNode(nodeIDList->data);

            if (cityNode && cityNode->moduleType == MODULE_COMMERCIAL) {
                if (nodeIDList->data.find("MALL") == 0) {
                    Mall* mall = mallsHash.search(nodeIDList->data);

                    if (mall) {
                        if (!requiredCategory.empty()) {
                            bool hasCategory = false;
                            LListNode<string>* storeNode = mall->storeIDs.getHead();
                            while (storeNode != nullptr) {
                                Store* store = storesHash.search(storeNode->data);
                                if (store && store->category == requiredCategory) {
                                    hasCategory = true;
                                    break;
                                }
                                storeNode = storeNode->next;
                            }
                            if (!hasCategory) {
                                nodeIDList = nodeIDList->next;
                                continue;
                            }
                        }

                        double latDiff = currentLat - mall->coords.latitude;
                        double lonDiff = currentLon - mall->coords.longitude;
                        double dist = 111.0 * sqrt((latDiff * latDiff) + (lonDiff * lonDiff));

                        if (dist < minDistance) {
                            minDistance = dist;
                            nearestMallID = nodeIDList->data;
                            nearestMall = mall;
                        }
                    }
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (nearestMall) {
            cout << "\nNearest Mall: " << nearestMall->name << endl;
            cout << "Sector: " << nearestMall->sector << endl;
   

            string tempNodeID = "CURRENT_LOC";

            if (cityGraph->getNode(tempNodeID)) {
                cityGraph->removeNode(tempNodeID);
            }

            cityGraph->addNode(tempNodeID, MODULE_PUBLIC_FACILITY, "Current Location",
                currentLat, currentLon);

            connectTempNodeToNetwork(tempNodeID, currentLat, currentLon);

            cityGraph->findShortestPath(tempNodeID, nearestMallID);
            cityGraph->removeNode(tempNodeID);
        }
        else {
            cout << "No suitable mall found" << endl;
        }
    }

    void findNearestMallFromNode(string currentNodeID, string requiredCategory = "") {
        CityNode* currentNode = cityGraph->getNode(currentNodeID);
        if (!currentNode) {
            cout << "Current node not found" << endl;
            return;
        }

        findNearestMall(currentNode->coords.latitude,
            currentNode->coords.longitude,
            requiredCategory);
    }

    void findCheapestProduct(string productName) {
        cout << "\n=== FINDING CHEAPEST " << productName << " ===" << endl;

        LinkedList<string>* productList = productsByName.search(toLowerCase(productName));

        if (!productList) {
            cout << "Product not found" << endl;
            return;
        }

        Product* cheapestProduct = nullptr;
        double minPrice = 999999.0;

        LListNode<string>* current = productList->getHead();
        while (current != nullptr) {
            Product* product = productsHash.search(current->data);
            if (product && product->quantity > 0 && product->price < minPrice) {
                minPrice = product->price;
                cheapestProduct = product;
            }
            current = current->next;
        }

        if (cheapestProduct) {
            Store* store = storesHash.search(cheapestProduct->storeID);
            Mall* mall = mallsHash.search(cheapestProduct->mallID);

            cout << "\nCheapest " << productName << ":" << endl;
            cout << "Price: $" << cheapestProduct->price << endl;
            cout << "Brand: " << cheapestProduct->brand << endl;
            if (store) cout << "Store: " << store->name << endl;
            if (mall) cout << "Mall: " << mall->name << endl;
        }
        else {
            cout << "No available stock" << endl;
        }
    }

    void displayAllMalls() {
        cout << "\n=== ALL REGISTERED MALLS ===" << endl;

        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        int count = 0;

        while (nodeIDList != nullptr) {
            if (nodeIDList->data.find("MALL") == 0) {
                Mall* mall = mallsHash.search(nodeIDList->data);
                if (mall) {
                    count++;
                    cout << count << ". " << mall->name << " (" << mall->mallID << ")" << endl;
                    cout << "   Sector: " << mall->sector << endl;
                    cout << "   Floors: " << mall->totalFloors << endl;
                    cout << "   Stores: " << mall->totalStores << endl;
                    cout << "   Category:" << mall->primaryCategory << endl;
                    cout << endl;
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (count == 0) cout << "No malls registered" << endl;
        else cout << "Total Malls: " << count << endl;
    }

    void displayAllStores() {
        cout << "\n=== ALL REGISTERED STORES ===" << endl;

        int count = 0;
        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();

        while (nodeIDList != nullptr) {
            if (nodeIDList->data.find("MALL") == 0) {
                Mall* mall = mallsHash.search(nodeIDList->data);
                if (mall) {
                    LListNode<string>* storeNode = mall->storeIDs.getHead();
                    while (storeNode != nullptr) {
                        Store* store = storesHash.search(storeNode->data);
                        if (store) {
                            count++;
                            cout << count << ". " << store->name << " (" << store->storeID << ")" << endl;
                            cout << "   Category: " << store->category << endl;
                            cout << "   Mall: " << mall->name << endl;
                            cout << endl;
                        }
                        storeNode = storeNode->next;
                    }
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (count == 0) cout << "No stores registered" << endl;
        else cout << "Total Stores: " << count << endl;
    }

    void displayStoresInMall(string mallID) {
        Mall* mall = mallsHash.search(mallID);

        if (!mall) {
            cout << "Mall not found" << endl;
            return;
        }

        cout << "\n=== STORES IN " << mall->name << " ===" << endl;

        LListNode<string>* storeNode = mall->storeIDs.getHead();
        int count = 0;

        if (!storeNode) {
            cout << "No stores in this mall" << endl;
            return;
        }

        while (storeNode != nullptr) {
            Store* store = storesHash.search(storeNode->data);
            if (store) {
                count++;
                cout << count << ". " << store->name << endl;
                cout << "   Category: " << store->category << endl;
                cout << "   Floor: " << store->floorNumber << endl;
                cout << endl;
            }
            storeNode = storeNode->next;
        }
    }

    void displayCommercialStatistics() {
        cout << "\n=== COMMERCIAL SECTOR STATISTICS ===" << endl;

        int totalMalls = 0;
        int totalStores = 0;
        int totalProducts = 0;
        double totalInventoryValue = 0.0;

        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        while (nodeIDList != nullptr) {
            if (nodeIDList->data.find("MALL") == 0) {
                Mall* mall = mallsHash.search(nodeIDList->data);
                if (mall) {
                    totalMalls++;
                    totalStores += mall->totalStores;
                }
            }
            nodeIDList = nodeIDList->next;
        }

        nodeIDList = cityGraph->getAllNodeIDs().getHead();
        while (nodeIDList != nullptr) {
            if (nodeIDList->data.find("PROD") == 0) {
                Product* product = productsHash.search(nodeIDList->data);
                if (product) {
                    totalProducts++;
                    totalInventoryValue += product->price * product->quantity;
                }
            }
            nodeIDList = nodeIDList->next;
        }

        cout << "Total Malls: " << totalMalls << endl;
        cout << "Total Stores: " << totalStores << endl;
        cout << "Total Products: " << totalProducts << endl;
        cout << "Total Inventory Value: $" << totalInventoryValue << endl;
        cout << "Shopping Cart Items: " << shoppingCart.getItemCount() << endl;
    }

    void loadSampleData() {
        cout << "\n[INFO] Loading sample commercial data...\n";

        // Reset counters
        mallCount = 0;
        storeCount = 0;
        productCount = 0;

        // Arrays to store generated IDs
        string mallIDs[5];
        string storeIDs[9];

        // Create malls - USE ACTUAL COORDINATES IN CONSTRUCTOR TOO!
        cout << "Creating malls...\n";
        mallIDs[0] = generateMallID();
        // FIX: Pass actual coordinates to Mall constructor
        Mall mall1(mallIDs[0], "Centaurus Mall", "Blue Area", 33.720, 73.065, 4, "Luxury");
        mallsHash.insert(mallIDs[0], mall1);
        cityGraph->addNodeWithAutoConnect(mallIDs[0], MODULE_COMMERCIAL, "Centaurus Mall",
            33.720, 73.065, "Blue Area", &mall1);

        mallIDs[1] = generateMallID();
        // FIX: Pass actual coordinates
        Mall mall2(mallIDs[1], "Giga Mall", "G-11", 33.670, 73.020, 5, "Family");
        mallsHash.insert(mallIDs[1], mall2);
        cityGraph->addNodeWithAutoConnect(mallIDs[1], MODULE_COMMERCIAL, "Giga Mall",
            33.670, 73.020, "G-11", &mall2);

        mallIDs[2] = generateMallID();
        // FIX: Pass actual coordinates
        Mall mall3(mallIDs[2], "Safa Gold Mall", "G-6", 33.680, 73.030, 3, "Budget");
        mallsHash.insert(mallIDs[2], mall3);
        cityGraph->addNodeWithAutoConnect(mallIDs[2], MODULE_COMMERCIAL, "Safa Gold Mall",
            33.680, 73.030, "G-6", &mall3);

        mallIDs[3] = generateMallID();
        // FIX: Pass actual coordinates
        Mall mall4(mallIDs[3], "Jinnah Super Market", "F-7", 33.721, 73.052, 2, "Grocery");
        mallsHash.insert(mallIDs[3], mall4);
        cityGraph->addNodeWithAutoConnect(mallIDs[3], MODULE_COMMERCIAL, "Jinnah Super Market",
            33.721, 73.052, "F-7", &mall4);

        mallIDs[4] = generateMallID();
        // FIX: Pass actual coordinates
        Mall mall5(mallIDs[4], "Jade Market", "F-6", 33.727, 73.075, 3, "Traditional");
        mallsHash.insert(mallIDs[4], mall5);
        cityGraph->addNodeWithAutoConnect(mallIDs[4], MODULE_COMMERCIAL, "Jade Market",
            33.727, 73.075, "F-6", &mall5);

        cout << "Malls created with IDs: ";
        for (int i = 0; i < 5; i++) {
            cout << mallIDs[i] << " ";
        }
        cout << endl;

        // Create stores for Centaurus Mall (mallIDs[0])
        cout << "\nCreating stores for Centaurus Mall...\n";
        storeIDs[0] = generateStoreID();
        Store store1(storeIDs[0], "Nike Store", "Clothing", 1, mallIDs[0]);
        storesHash.insert(storeIDs[0], store1);
        mallsHash.search(mallIDs[0])->storeIDs.insertBack(storeIDs[0]);
        mallsHash.search(mallIDs[0])->totalStores++;

        storeIDs[1] = generateStoreID();
        Store store2(storeIDs[1], "Apple Store", "Electronics", 2, mallIDs[0]);
        storesHash.insert(storeIDs[1], store2);
        mallsHash.search(mallIDs[0])->storeIDs.insertBack(storeIDs[1]);
        mallsHash.search(mallIDs[0])->totalStores++;

        storeIDs[2] = generateStoreID();
        Store store3(storeIDs[2], "Food Court", "Food", 3, mallIDs[0]);
        storesHash.insert(storeIDs[2], store3);
        mallsHash.search(mallIDs[0])->storeIDs.insertBack(storeIDs[2]);
        mallsHash.search(mallIDs[0])->totalStores++;

        storeIDs[3] = generateStoreID();
        Store store4(storeIDs[3], "Adidas Store", "Clothing", 1, mallIDs[0]);
        storesHash.insert(storeIDs[3], store4);
        mallsHash.search(mallIDs[0])->storeIDs.insertBack(storeIDs[3]);
        mallsHash.search(mallIDs[0])->totalStores++;

        storeIDs[4] = generateStoreID();
        Store store5(storeIDs[4], "Samsung Store", "Electronics", 2, mallIDs[0]);
        storesHash.insert(storeIDs[4], store5);
        mallsHash.search(mallIDs[0])->storeIDs.insertBack(storeIDs[4]);
        mallsHash.search(mallIDs[0])->totalStores++;

        // Create stores for Giga Mall (mallIDs[1])
        cout << "Creating stores for Giga Mall...\n";
        storeIDs[5] = generateStoreID();
        Store store6(storeIDs[5], "Levis Store", "Clothing", 1, mallIDs[1]);
        storesHash.insert(storeIDs[5], store6);
        mallsHash.search(mallIDs[1])->storeIDs.insertBack(storeIDs[5]);
        mallsHash.search(mallIDs[1])->totalStores++;

        storeIDs[6] = generateStoreID();
        Store store7(storeIDs[6], "Huawei Store", "Electronics", 2, mallIDs[1]);
        storesHash.insert(storeIDs[6], store7);
        mallsHash.search(mallIDs[1])->storeIDs.insertBack(storeIDs[6]);
        mallsHash.search(mallIDs[1])->totalStores++;

        storeIDs[7] = generateStoreID();
        Store store8(storeIDs[7], "KFC", "Food", 3, mallIDs[1]);
        storesHash.insert(storeIDs[7], store8);
        mallsHash.search(mallIDs[1])->storeIDs.insertBack(storeIDs[7]);
        mallsHash.search(mallIDs[1])->totalStores++;

        storeIDs[8] = generateStoreID();
        Store store9(storeIDs[8], "Pizza Hut", "Food", 3, mallIDs[1]);
        storesHash.insert(storeIDs[8], store9);
        mallsHash.search(mallIDs[1])->storeIDs.insertBack(storeIDs[8]);
        mallsHash.search(mallIDs[1])->totalStores++;

        cout << "Stores created. Store IDs: ";
        for (int i = 0; i < 9; i++) {
            cout << storeIDs[i] << " ";
        }
        cout << endl;

        // Create products
        cout << "\nCreating products...\n";

        // Products for Apple Store (storeIDs[1])
        string prodID = generateProductID();
        Product prod1(prodID, "iPhone 15", "Electronics", "Apple", 1200.0, 50, storeIDs[1], mallIDs[0]);
        productsHash.insert(prodID, prod1);

        prodID = generateProductID();
        Product prod2(prodID, "MacBook Pro", "Electronics", "Apple", 2000.0, 30, storeIDs[1], mallIDs[0]);
        productsHash.insert(prodID, prod2);

        prodID = generateProductID();
        Product prod3(prodID, "AirPods Pro", "Electronics", "Apple", 250.0, 100, storeIDs[1], mallIDs[0]);
        productsHash.insert(prodID, prod3);

        // Products for Nike Store (storeIDs[0])
        prodID = generateProductID();
        Product prod4(prodID, "Running Shoes", "Clothing", "Nike", 150.0, 200, storeIDs[0], mallIDs[0]);
        productsHash.insert(prodID, prod4);

        prodID = generateProductID();
        Product prod5(prodID, "Sports T-Shirt", "Clothing", "Nike", 40.0, 300, storeIDs[0], mallIDs[0]);
        productsHash.insert(prodID, prod5);

        // Products for Food Court (storeIDs[2])
        prodID = generateProductID();
        Product prod6(prodID, "Burger", "Food", "KFC", 8.0, 500, storeIDs[2], mallIDs[0]);
        productsHash.insert(prodID, prod6);

        // Products for Pizza Hut (storeIDs[8])
        prodID = generateProductID();
        Product prod7(prodID, "Pizza", "Food", "Pizza Hut", 12.0, 300, storeIDs[8], mallIDs[1]);
        productsHash.insert(prodID, prod7);

        // Products for Levis Store (storeIDs[5])
        prodID = generateProductID();
        Product prod8(prodID, "Jeans", "Clothing", "Levis", 80.0, 150, storeIDs[5], mallIDs[1]);
        productsHash.insert(prodID, prod8);

        // Products for Samsung Store (storeIDs[4])
        prodID = generateProductID();
        Product prod9(prodID, "Smartphone", "Electronics", "Samsung", 800.0, 75, storeIDs[4], mallIDs[0]);
        productsHash.insert(prodID, prod9);

        // Products for Huawei Store (storeIDs[6])
        prodID = generateProductID();
        Product prod10(prodID, "Smartwatch", "Electronics", "Huawei", 200.0, 120, storeIDs[6], mallIDs[1]);
        productsHash.insert(prodID, prod10);

        // Update indexing structures
        cout << "Updating search indexes...\n";

        // Index stores by category
        for (int i = 0; i < 9; i++) {
            Store* store = storesHash.search(storeIDs[i]);
            if (store) {
                string lowerCategory = toLowerCase(store->category);
                LinkedList<string>* catList = storesByCategory.search(lowerCategory);
                if (catList) {
                    catList->insertBack(storeIDs[i]);
                }
                else {
                    LinkedList<string> newList;
                    newList.insertBack(storeIDs[i]);
                    storesByCategory.insert(lowerCategory, newList);
                }

                LinkedList<string>* mallList = storesByMall.search(store->mallID);
                if (mallList) {
                    mallList->insertBack(storeIDs[i]);
                }
                else {
                    LinkedList<string> newList;
                    newList.insertBack(storeIDs[i]);
                    storesByMall.insert(store->mallID, newList);
                }
            }
        }

        // Index products
        for (int i = 1; i <= 10; i++) {
            string prodID = "PROD";
            if (i < 10) prodID += "00" + to_string(i);
            else if (i < 100) prodID += "0" + to_string(i);
            else prodID += to_string(i);

            Product* product = productsHash.search(prodID);
            if (product) {
                // Index by name
                string lowerName = toLowerCase(product->name);
                LinkedList<string>* nameList = productsByName.search(lowerName);
                if (nameList) {
                    nameList->insertBack(prodID);
                }
                else {
                    LinkedList<string> newList;
                    newList.insertBack(prodID);
                    productsByName.insert(lowerName, newList);
                }

                // Index by category
                string lowerCategory = toLowerCase(product->category);
                LinkedList<string>* catList = productsByCategory.search(lowerCategory);
                if (catList) {
                    catList->insertBack(prodID);
                }
                else {
                    LinkedList<string> newList;
                    newList.insertBack(prodID);
                    productsByCategory.insert(lowerCategory, newList);
                }

                // Index by brand
                string lowerBrand = toLowerCase(product->brand);
                LinkedList<string>* brandList = productsByBrand.search(lowerBrand);
                if (brandList) {
                    brandList->insertBack(prodID);
                }
                else {
                    LinkedList<string> newList;
                    newList.insertBack(prodID);
                    productsByBrand.insert(lowerBrand, newList);
                }
            }
        }

        cout << "[SUCCESS] Sample commercial data loaded successfully!\n";
        cout << "Created: 5 malls, 9 stores, 10 products\n";
    }

    // Main interactive menu for commercial module
    // Data Structure: Menu-driven interface with switch-case
    void displayCommercialMenu() {
        const string RESET = "\033[0m";
        const string YELLOW = "\033[33m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string WHITE = "\033[37m";
        const string BRIGHT_WHITE = "\033[97m";
        const string CYAN = "\033[36m";
        const string GREEN = "\033[32m";
        const string BRIGHT_RED = "\033[91m";

        int choice;
        do {
            cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
            cout << BRIGHT_YELLOW << "        COMMERCIAL SECTOR MENU" << RESET << endl;
            cout << BRIGHT_YELLOW << "========================================" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  REGISTRATION" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  1. Register New Mall" << RESET << endl;
            cout << BRIGHT_WHITE << "  2. Register New Store" << RESET << endl;
            cout << BRIGHT_WHITE << "  3. Add New Product" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  SEARCH OPERATIONS" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  4. Search Product by Name" << RESET << endl;
            cout << BRIGHT_WHITE << "  5. Search Product by Category" << RESET << endl;
            cout << BRIGHT_WHITE << "  6. Search Product by Brand" << RESET << endl;
            cout << BRIGHT_WHITE << "  7. Search Store by Category" << RESET << endl;
            cout << BRIGHT_WHITE << "  8. Find Cheapest Product" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  SHOPPING CART" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  9. Add to Shopping Cart" << RESET << endl;
            cout << BRIGHT_WHITE << " 10. View Shopping Cart" << RESET << endl;
            cout << BRIGHT_WHITE << " 11. Update Cart Quantity" << RESET << endl;
            cout << BRIGHT_WHITE << " 12. Remove from Cart" << RESET << endl;
            cout << BRIGHT_WHITE << " 13. Checkout" << RESET << endl;
            cout << BRIGHT_WHITE << " 14. Clear Cart" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  LOCATION SERVICES" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 15. Find Nearest Mall" << RESET << endl;
            cout << BRIGHT_WHITE << " 16. Find Nearest Mall by Category" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  DISPLAY & STATISTICS" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 17. Display All Malls" << RESET << endl;
            cout << BRIGHT_WHITE << " 18. Display All Stores" << RESET << endl;
            cout << BRIGHT_WHITE << " 19. Display Stores in Mall" << RESET << endl;
            cout << BRIGHT_WHITE << " 20. Display Statistics" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  DATA LOADING" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 21. Load Sample Data" << RESET << endl << endl;

            cout << BRIGHT_WHITE << "  0. Return to Main Menu" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "Choose: " << RESET;

            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << BRIGHT_RED << "[ERROR] Invalid input. Please enter a number." << RESET << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                continue;
            }

            string input1, input2, input3;
            double dinput1, dinput2;

            switch (choice) {
            case 1: registerMallManual(); break;
            case 2: registerStoreManual(); break;
            case 3: addProductManual(); break;
            case 4:
                cout << "Enter Product Name: ";
                cin.ignore(); getline(cin, input1);
                searchProductByName(input1);
                break;
            case 5:
                cout << "Enter Category: ";
                cin.ignore(); 
                getline(cin, input1);
                searchProductByCategory(input1);
                break;
            case 6:
                cout << "Enter Brand: ";
                cin.ignore();
                getline(cin, input1);
                searchProductByBrand(input1);
                break;
            case 7:
                cout << "Enter Store Category: ";
                cin.ignore();
                getline(cin, input1);
                searchStoreByCategory(input1);
                break;
            case 8:
                cout << "Enter Product Name: ";
                cin.ignore();
                getline(cin, input1);
                findCheapestProduct(input1);
                break;
            case 9:
                addToCart();
                break;
            case 10:
                viewCart();
                break;
            case 11:
                updateCartQuantity();
                break;
            case 12:
                removeFromCart();
                break;
            case 13:
                checkout();
                break;
            case 14:
                clearCart();
                break;
            case 15:
                cout << "Enter Current Node ID (or 'coord' for coordinates): ";
                cin >> input1;
                if (input1 == "coord") {
                    cout << "Enter Latitude: ";
                    cin >> dinput1;
                    cout << "Enter Longitude: ";
                    cin >> dinput2;
                    findNearestMall(dinput1, dinput2);
                    // lat and lon are used via dinput1 and dinput2
                }
                else {
                    findNearestMallFromNode(input1);
                }
                break;
            case 16:
                cout << "Enter Current Node ID (or 'coord' for coordinates): ";
                cin >> input1;
                cout << "Enter Required Store Category: ";
                cin.ignore();
                getline(cin, input3);
                if (input1 == "coord") {
                    cout << "Enter Latitude: ";
                    cin >> dinput1;
                    cout << "Enter Longitude: ";
                    cin >> dinput2;
                    findNearestMall(dinput1, dinput2, input3);
                }
                else {
                    findNearestMallFromNode(input1, input2);
                }
                break;
            case 17:
                displayAllMalls();
                break;
            case 18:
                displayAllStores();
                break;
            case 19:
                cout << "Enter Mall ID: ";
                cin >> input1;
                displayStoresInMall(input1);
                break;
            case 20:
                displayCommercialStatistics();
                break;
            case 21:
                loadSampleData();
                break;
          
            case 0:
                cout << GREEN << "Returning to main menu..." << RESET << endl;
                break;
            default:
                cout << BRIGHT_RED << "[ERROR] Invalid choice. Please enter a number between 0-21." << RESET << endl;
            }

            if (choice != 0) {
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
            }
        } while (choice != 0);
    }
};