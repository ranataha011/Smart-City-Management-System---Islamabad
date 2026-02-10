#pragma once
#include "CityEntities.h"
#include "MainCityGraph.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib> // For rand()
#include <ctime>   // For time()

using namespace std;

// ==========================================
// MEDICAL ENTITY STRUCTURES
// ==========================================

struct Medicine {
    string medicineID;      // e.g., "MED001"
    string name;            // e.g., "Panadol"
    string formula;         // e.g., "Paracetamol"
    int price;              // Price in PKR
    int stockQuantity;      // Available quantity
    string pharmacyID;      // Which pharmacy has this medicine

    Medicine() : price(0), stockQuantity(0) {}

    Medicine(string id, string n, string f, int p, int stock, string pharmID)
        : medicineID(id), name(n), formula(f), price(p),
        stockQuantity(stock), pharmacyID(pharmID) {
    }
};

struct Doctor {
    string doctorID;        // e.g., "DOC001"
    string name;
    string specialization;  // e.g., "Cardiology", "General"
    string hospitalID;      // Assigned hospital
    string contactNo;
    bool isAvailable;

    Doctor() : isAvailable(true) {}

    Doctor(string id, string n, string spec, string hospID, string contact)
        : doctorID(id), name(n), specialization(spec),
        hospitalID(hospID), contactNo(contact), isAvailable(true) {
    }
};

struct Patient {
    string patientID;       // e.g., "PAT001"
    string name;
    string cnic;
    int age;
    string disease;
    string assignedDoctorID;
    string hospitalID;
    bool isAdmitted;

    Patient() : age(0), isAdmitted(false) {}

    Patient(string id, string n, string c, int a, string dis)
        : patientID(id), name(n), cnic(c), age(a), disease(dis),
        isAdmitted(false) {
    }
};

struct Hospital {
    string hospitalID;      // e.g., "H01"
    string name;
    string sector;
    Coordinates coords;
    int totalBeds;
    int availableBeds;      // For emergency priority
    int emergencyBeds;      // Separate emergency beds
    string specialization;  // Primary specialization
    LinkedList<string> doctorIDs;  // List of doctors

    Hospital() : totalBeds(0), availableBeds(0), emergencyBeds(0) {}

    Hospital(string id, string n, string sec, double lat, double lon,
        int beds, int emergency, string spec)
        : hospitalID(id), name(n), sector(sec), coords(lat, lon),
        totalBeds(beds), availableBeds(beds), emergencyBeds(emergency), specialization(spec) {
    }
};

struct Pharmacy {
    string pharmacyID;      // e.g., "P01"
    string name;
    string sector;
    Coordinates coords;
    LinkedList<string> medicineIDs;  // List of medicines available

    Pharmacy() {}

    Pharmacy(string id, string n, string sec, double lat, double lon)
        : pharmacyID(id), name(n), sector(sec), coords(lat, lon) {
    }
};

// ==========================================
// EMERGENCY BED PRIORITY SYSTEM (MAX HEAP)
// ==========================================

struct EmergencyCase {
    string hospitalID;
    int availableBeds;
    int emergencyBeds;      // Added for emergency bed priority
    double distanceFromPatient;  // Used for secondary sorting
    string hospitalName;

    EmergencyCase() : availableBeds(0), emergencyBeds(0), distanceFromPatient(0.0) {}

    EmergencyCase(string id, int beds, int emergency, double dist, string name)
        : hospitalID(id), availableBeds(beds), emergencyBeds(emergency),
        distanceFromPatient(dist), hospitalName(name) {
    }
};

class EmergencyBedMaxHeap {
private:
    EmergencyCase* heap;
    int capacity;
    int size;

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;

            // Max heap based on emergency beds (more emergency beds = higher priority)
            // If emergency beds equal, use total available beds
            // If both equal, closer hospital has priority
            bool shouldSwap = false;

            // First priority: Emergency beds
            if (heap[index].emergencyBeds > heap[parent].emergencyBeds) {
                shouldSwap = true;
            }
            else if (heap[index].emergencyBeds == heap[parent].emergencyBeds) {
                // Second priority: Total available beds
                if (heap[index].availableBeds > heap[parent].availableBeds) {
                    shouldSwap = true;
                }
                // Third priority: Distance
                else if (heap[index].availableBeds == heap[parent].availableBeds &&
                    heap[index].distanceFromPatient < heap[parent].distanceFromPatient) {
                    shouldSwap = true;
                }
            }

            if (shouldSwap) {
                EmergencyCase temp = heap[index];
                heap[index] = heap[parent];
                heap[parent] = temp;
                index = parent;
            }
            else {
                break;
            }
        }
    }

    void heapifyDown(int index) {
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;

            if (left < size) {
                if (heap[left].emergencyBeds > heap[largest].emergencyBeds) {
                    largest = left;
                }
                else if (heap[left].emergencyBeds == heap[largest].emergencyBeds) {
                    if (heap[left].availableBeds > heap[largest].availableBeds) {
                        largest = left;
                    }
                    else if (heap[left].availableBeds == heap[largest].availableBeds &&
                        heap[left].distanceFromPatient < heap[largest].distanceFromPatient) {
                        largest = left;
                    }
                }
            }

            if (right < size) {
                if (heap[right].emergencyBeds > heap[largest].emergencyBeds) {
                    largest = right;
                }
                else if (heap[right].emergencyBeds == heap[largest].emergencyBeds) {
                    if (heap[right].availableBeds > heap[largest].availableBeds) {
                        largest = right;
                    }
                    else if (heap[right].availableBeds == heap[largest].availableBeds &&
                        heap[right].distanceFromPatient < heap[largest].distanceFromPatient) {
                        largest = right;
                    }
                }
            }

            if (largest != index) {
                EmergencyCase temp = heap[index];
                heap[index] = heap[largest];
                heap[largest] = temp;
                index = largest;
            }
            else {
                break;
            }
        }
    }

public:
    EmergencyBedMaxHeap(int cap = 100) : capacity(cap), size(0) {
        heap = new EmergencyCase[capacity];
    }

    ~EmergencyBedMaxHeap() {
        delete[] heap;
    }

    void insert(EmergencyCase ec) {
        if (size >= capacity) {
            cout << "[ERROR] Emergency heap is full!" << endl;
            return;
        }

        heap[size] = ec;
        heapifyUp(size);
        size++;
    }

    EmergencyCase extractMax() {
        if (size == 0) {
            return EmergencyCase();
        }

        EmergencyCase maxCase = heap[0];
        heap[0] = heap[size - 1];
        size--;
        heapifyDown(0);

        return maxCase;
    }

    bool isEmpty() {
        return size == 0;
    }

    int getSize() {
        return size;
    }

    void clear() {
        size = 0;
    }
};

// ==========================================
// MEDICAL SECTOR MAIN CLASS
// ==========================================

class MedicalSector {
private:
    MainCityGraph* cityGraph;  // Reference to main city graph

    // Hash Tables for fast lookup
    HashTable<Hospital> hospitalsHash;
    HashTable<Pharmacy> pharmaciesHash;
    HashTable<Doctor> doctorsHash;
    HashTable<Patient> patientsHash;
    HashTable<Medicine> medicinesHash;

    // Additional indexing for search operations
    HashTable<LinkedList<string>> medicinesByFormula;    // Formula -> List of MedicineIDs
    HashTable<LinkedList<string>> medicinesByName;       // Name -> List of MedicineIDs
    HashTable<LinkedList<string>> doctorsBySpecialization; // Specialization -> List of DoctorIDs

    // Keep a list of patient IDs for listing
    LinkedList<string> patientIDs;

    // Counters for ID generation
    int hospitalCount;
    int pharmacyCount;
    int doctorCount;
    int patientCount;
    int medicineCount;

    // Helper function to convert integer to string
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

    // Convert string to lowercase for case-insensitive search
    string toLowerCase(string str) {
        string result = str;
        for (size_t i = 0; i < result.length(); i++) {
            if (result[i] >= 'A' && result[i] <= 'Z') {
                result[i] = result[i] + 32;
            }
        }
        return result;
    }

    // Calculate distance between two coordinates (Option 1: keep original simple formula)
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        double latDiff = lat1 - lat2;
        double lonDiff = lon1 - lon2;
        return 111.0 * sqrt((latDiff * latDiff) + (lonDiff * lonDiff));
    }

    // Generate random coordinate within sector with better distribution
    void generateRandomCoordinatesInSector(string sector, double& lat, double& lon) {
        // Seed random number generator
        static bool seeded = false;
        if (!seeded) {
            srand((unsigned int)time(0));
            seeded = true;
        }

        // Try to get coordinates from sector system first
        if (cityGraph->getSectorSystem().getCoordinatesInSector(sector, lat, lon)) {
            // Add small random variation to avoid same coordinates
            double latRange = 0.005; // ~0.5 km variation
            double lonRange = 0.005;

            lat += ((rand() % 1000) / 1000.0 - 0.5) * latRange;
            lon += ((rand() % 1000) / 1000.0 - 0.5) * lonRange;
        }
        else {
            // Fallback: Generate based on sector name
            if (sector.find("F-") != string::npos) {
                lat = 33.70 + ((rand() % 100) / 1000.0);
                lon = 73.03 + ((rand() % 100) / 1000.0);
            }
            else if (sector.find("G-") != string::npos) {
                lat = 33.69 + ((rand() % 100) / 1000.0);
                lon = 73.04 + ((rand() % 100) / 1000.0);
            }
            else if (sector.find("H-") != string::npos) {
                lat = 33.68 + ((rand() % 100) / 1000.0);
                lon = 73.05 + ((rand() % 100) / 1000.0);
            }
            else if (sector.find("I-") != string::npos) {
                lat = 33.67 + ((rand() % 100) / 1000.0);
                lon = 73.06 + ((rand() % 100) / 1000.0);
            }
            else if (sector == "Blue Area") {
                lat = 33.720 + ((rand() % 50) / 1000.0);
                lon = 73.065 + ((rand() % 50) / 1000.0);
            }
            else {
                // Default Islamabad coordinates
                lat = 33.684 + ((rand() % 200) / 1000.0);
                lon = 73.047 + ((rand() % 200) / 1000.0);
            }
        }
    }

public:
    MedicalSector(MainCityGraph* graph)
        : cityGraph(graph),
        hospitalsHash(100),
        pharmaciesHash(100),
        doctorsHash(200),
        patientsHash(500),
        medicinesHash(500),
        medicinesByFormula(100),
        medicinesByName(100),
        doctorsBySpecialization(50),
        hospitalCount(0),
        pharmacyCount(0),
        doctorCount(0),
        patientCount(0),
        medicineCount(0) {

        // Initialize random seed
        srand((unsigned int)time(0));
    }

    // ==========================================
    // ID GENERATION FUNCTIONS (CORRECTED)
    // ==========================================

    string generateHospitalID() {
        hospitalCount++;
        string id = "H";
        if (hospitalCount < 10) {
            id += "0";
        }
        id += intToString(hospitalCount);
        return id;
    }

    string generatePharmacyID() {
        pharmacyCount++;
        string id = "P";
        if (pharmacyCount < 10) {
            id += "0";
        }
        id += intToString(pharmacyCount);
        return id;
    }

    string generateDoctorID() {
        doctorCount++;
        string id = "DOC";
        if (doctorCount < 10) {
            id += "00";
        }
        else if (doctorCount < 100) {
            id += "0";
        }
        id += intToString(doctorCount);
        return id;
    }

    string generatePatientID() {
        patientCount++;
        string id = "PAT";
        if (patientCount < 10) {
            id += "00";
        }
        else if (patientCount < 100) {
            id += "0";
        }
        id += intToString(patientCount);
        return id;
    }

    string generateMedicineID() {
        medicineCount++;
        string id = "MED";
        if (medicineCount < 10) {
            id += "00";
        }
        else if (medicineCount < 100) {
            id += "0";
        }
        id += intToString(medicineCount);
        return id;
    }

    // ==========================================
    // HOSPITAL MANAGEMENT (UPDATED WITH EMERGENCY BEDS)
    // ==========================================

    // Registers a new hospital in the system
    // Data Structure: HashTable - O(1) average insertion for hospital lookup
    void registerHospital(string name, string sector, int totalBeds, int emergencyBeds, string specialization) {
        // Edge case: Empty name
        if (name.empty()) {
            cout << "[ERROR] Hospital name cannot be empty." << endl;
            return;
        }

        // Edge case: Empty sector
        if (sector.empty()) {
            cout << "[ERROR] Sector cannot be empty." << endl;
            return;
        }

        // Edge case: Invalid bed counts
        if (totalBeds <= 0) {
            cout << "[ERROR] Total beds must be greater than 0." << endl;
            return;
        }

        if (emergencyBeds < 0) {
            cout << "[ERROR] Emergency beds cannot be negative." << endl;
            return;
        }

        // Validate emergency beds don't exceed total beds
        if (emergencyBeds > totalBeds) {
            cout << "[ERROR] Emergency beds (" << emergencyBeds
                << ") cannot exceed total beds (" << totalBeds << ")" << endl;
            return;
        }

        // Edge case: Empty specialization
        if (specialization.empty()) {
            cout << "[ERROR] Specialization cannot be empty." << endl;
            return;
        }

        // Edge case: Check if cityGraph is null
        if (!cityGraph) {
            cout << "[ERROR] City graph is not initialized." << endl;
            return;
        }

        string hospitalID = generateHospitalID();
        double lat, lon;

        // Get unique coordinates for the sector
        generateRandomCoordinatesInSector(sector, lat, lon);

        Hospital hospital(hospitalID, name, sector, lat, lon, totalBeds, emergencyBeds, specialization);
        hospitalsHash.insert(hospitalID, hospital);

        // Add to main city graph as a node
        cityGraph->addNodeWithAutoConnect(hospitalID, MODULE_MEDICAL, name,
            lat, lon, sector, &hospital);

        cout << "[SUCCESS] Hospital registered: " << name << " (" << hospitalID << ")" << endl;
        cout << "          Sector: " << sector << endl;
        cout << "          Total Beds: " << totalBeds << endl;
        cout << "          Emergency Beds: " << emergencyBeds << endl;
        cout << "          Specialization: " << specialization << endl;
        cout << "          Location: (" << lat << ", " << lon << ")" << endl;
    }

    void registerHospitalManual() {
        string name, sector, specialization;
        int totalBeds, emergencyBeds;

        cout << "\n=== REGISTER NEW HOSPITAL ===\n";

        cout << "Available sectors:" << endl;
        cityGraph->getSectorSystem().displayAllSectors();

        cout << "\nEnter Hospital Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Sector (e.g., F-8, G-10): ";
        getline(cin, sector);

        cout << "Enter Total Beds: ";
        cin >> totalBeds;

        cout << "Enter Emergency Beds (must be <= Total Beds): ";
        cin >> emergencyBeds;

        // Validate emergency beds
        while (emergencyBeds > totalBeds) {
            cout << "[ERROR] Emergency beds cannot exceed total beds!" << endl;
            cout << "Enter Emergency Beds (must be <= " << totalBeds << "): ";
            cin >> emergencyBeds;
        }

        cout << "Enter Specialization: ";
        cin.ignore();
        getline(cin, specialization);

        registerHospital(name, sector, totalBeds, emergencyBeds, specialization);
    }

    bool admitPatient(string patientID, string hospitalID, bool isEmergency = false) {
        Patient* patient = patientsHash.search(patientID);
        Hospital* hospital = hospitalsHash.search(hospitalID);

        if (!patient) {
            cout << "[ERROR] Patient not found: " << patientID << endl;
            return false;
        }

        if (!hospital) {
            cout << "[ERROR] Hospital not found: " << hospitalID << endl;
            return false;
        }

        if (isEmergency) {
            if (hospital->emergencyBeds <= 0) {
                cout << "[ERROR] No emergency beds available at " << hospital->name << endl;
                return false;
            }
            // Per your instruction: emergency admission consumes an emergency bed.
            hospital->emergencyBeds--;
            cout << "[INFO] Emergency bed allocated. Remaining emergency beds: "
                << hospital->emergencyBeds << endl;
        }
        else {
            if (hospital->availableBeds <= 0) {
                cout << "[ERROR] No beds available at " << hospital->name << endl;
                return false;
            }
            // Regular admission consumes an available (regular) bed.
            hospital->availableBeds--;
        }

        patient->hospitalID = hospitalID;
        patient->isAdmitted = true;

        // Update hospital & patient in hash table
        hospitalsHash.insert(hospitalID, *hospital);
        patientsHash.insert(patientID, *patient);

        cout << "[SUCCESS] Patient " << patient->name << " admitted to "
            << hospital->name << endl;
        if (isEmergency) {
            cout << "          Type: Emergency admission" << endl;
        }
        cout << "          Remaining total beds: " << hospital->availableBeds << endl;

        return true;
    }

    bool admitEmergencyPatient(string patientID, string hospitalID) {
        return admitPatient(patientID, hospitalID, true);
    }

    void dischargePatient(string patientID) {
        Patient* patient = patientsHash.search(patientID);

        if (!patient || !patient->isAdmitted) {
            cout << "[ERROR] Patient not found or not admitted" << endl;
            return;
        }

        Hospital* hospital = hospitalsHash.search(patient->hospitalID);
        if (hospital) {
            hospital->availableBeds++;
            // Note: We don't automatically return emergency beds
            // as we don't track which type of bed was used
            hospitalsHash.insert(patient->hospitalID, *hospital);
        }

        patient->isAdmitted = false;
        patient->hospitalID = "";
        patientsHash.insert(patientID, *patient);

        cout << "[SUCCESS] Patient discharged: " << patient->name << endl;
    }

    // ==========================================
    // PHARMACY MANAGEMENT (UPDATED COORDINATES)
    // ==========================================

    // Registers a new pharmacy in the system
    // Data Structure: HashTable - O(1) average insertion for pharmacy lookup
    void registerPharmacy(string name, string sector) {
        // Edge case: Empty name
        if (name.empty()) {
            cout << "[ERROR] Pharmacy name cannot be empty." << endl;
            return;
        }

        // Edge case: Empty sector
        if (sector.empty()) {
            cout << "[ERROR] Sector cannot be empty." << endl;
            return;
        }

        // Edge case: Check if cityGraph is null
        if (!cityGraph) {
            cout << "[ERROR] City graph is not initialized." << endl;
            return;
        }

        string pharmacyID = generatePharmacyID();
        double lat, lon;

        // Get unique coordinates for the sector
        generateRandomCoordinatesInSector(sector, lat, lon);

        Pharmacy pharmacy(pharmacyID, name, sector, lat, lon);
        pharmaciesHash.insert(pharmacyID, pharmacy);

        // Add to main city graph
        cityGraph->addNodeWithAutoConnect(pharmacyID, MODULE_MEDICAL, name,
            lat, lon, sector, &pharmacy);

        cout << "[SUCCESS] Pharmacy registered: " << name << " (" << pharmacyID << ")" << endl;
        cout << "          Sector: " << sector << endl;
        cout << "          Location: (" << lat << ", " << lon << ")" << endl;
    }

    void registerPharmacyManual() {
        string name, sector;

        cout << "\n=== REGISTER NEW PHARMACY ===\n";

        cout << "Available sectors:" << endl;
        cityGraph->getSectorSystem().displayAllSectors();

        cout << "\nEnter Pharmacy Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Sector (e.g., F-8, G-10): ";
        getline(cin, sector);

        registerPharmacy(name, sector);
    }

    // ==========================================
    // MEDICINE MANAGEMENT
    // ==========================================

    // Adds a new medicine to a pharmacy
    // Data Structure: HashTable - O(1) average lookup for pharmacy and medicine storage
    void addMedicine(string pharmacyID, string medicineName, string formula,
        int price, int stockQuantity) {
        // Edge case: Empty pharmacy ID
        if (pharmacyID.empty()) {
            cout << "[ERROR] Pharmacy ID cannot be empty." << endl;
            return;
        }

        // Edge case: Empty medicine name
        if (medicineName.empty()) {
            cout << "[ERROR] Medicine name cannot be empty." << endl;
            return;
        }

        // Edge case: Empty formula
        if (formula.empty()) {
            cout << "[ERROR] Medicine formula cannot be empty." << endl;
            return;
        }

        // Edge case: Invalid price
        if (price < 0) {
            cout << "[ERROR] Price cannot be negative." << endl;
            return;
        }

        // Edge case: Invalid stock quantity
        if (stockQuantity < 0) {
            cout << "[ERROR] Stock quantity cannot be negative." << endl;
            return;
        }

        Pharmacy* pharmacy = pharmaciesHash.search(pharmacyID);

        if (!pharmacy) {
            cout << "[ERROR] Pharmacy not found: " << pharmacyID << endl;
            return;
        }

        string medicineID = generateMedicineID();
        Medicine medicine(medicineID, medicineName, formula, price,
            stockQuantity, pharmacyID);

        medicinesHash.insert(medicineID, medicine);
        pharmacy->medicineIDs.insertBack(medicineID);
        pharmaciesHash.insert(pharmacyID, *pharmacy);

        // Index by formula (case-insensitive)
        string lowerFormula = toLowerCase(formula);
        LinkedList<string>* formulaList = medicinesByFormula.search(lowerFormula);
        if (formulaList) {
            formulaList->insertBack(medicineID);
        }
        else {
            LinkedList<string> newList;
            newList.insertBack(medicineID);
            medicinesByFormula.insert(lowerFormula, newList);
        }

        // Index by name (case-insensitive)
        string lowerName = toLowerCase(medicineName);
        LinkedList<string>* nameList = medicinesByName.search(lowerName);
        if (nameList) {
            nameList->insertBack(medicineID);
        }
        else {
            LinkedList<string> newList;
            newList.insertBack(medicineID);
            medicinesByName.insert(lowerName, newList);
        }

        cout << "[SUCCESS] Medicine added: " << medicineName << " (" << medicineID << ")" << endl;
        cout << "          Pharmacy: " << pharmacy->name << ", Stock: " << stockQuantity << endl;
        cout << "          Price: Rs. " << price << endl;
    }

    void addMedicineManual() {
        string pharmacyID, name, formula;
        int price, stock;

        cout << "\n=== ADD NEW MEDICINE ===\n";
        cout << "Enter Pharmacy ID: ";
        cin >> pharmacyID;

        cout << "Enter Medicine Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Formula: ";
        getline(cin, formula);

        cout << "Enter Price (PKR): ";
        cin >> price;

        cout << "Enter Stock Quantity: ";
        cin >> stock;

        addMedicine(pharmacyID, name, formula, price, stock);
    }

    // ==========================================
    // DOCTOR MANAGEMENT
    // ==========================================

    // Registers a new doctor in the system
    // Data Structure: HashTable - O(1) average lookup for hospital and doctor storage
    void registerDoctor(string name, string specialization, string hospitalID,
        string contactNo) {
        // Edge case: Empty name
        if (name.empty()) {
            cout << "[ERROR] Doctor name cannot be empty." << endl;
            return;
        }

        // Edge case: Empty specialization
        if (specialization.empty()) {
            cout << "[ERROR] Specialization cannot be empty." << endl;
            return;
        }

        // Edge case: Empty hospital ID
        if (hospitalID.empty()) {
            cout << "[ERROR] Hospital ID cannot be empty." << endl;
            return;
        }

        // Edge case: Empty contact
        if (contactNo.empty()) {
            cout << "[ERROR] Contact number cannot be empty." << endl;
            return;
        }

        Hospital* hospital = hospitalsHash.search(hospitalID);

        if (!hospital) {
            cout << "[ERROR] Hospital not found: " << hospitalID << endl;
            return;
        }

        string doctorID = generateDoctorID();
        Doctor doctor(doctorID, name, specialization, hospitalID, contactNo);

        doctorsHash.insert(doctorID, doctor);
        hospital->doctorIDs.insertBack(doctorID);
        hospitalsHash.insert(hospitalID, *hospital);

        // Index by specialization
        string lowerSpec = toLowerCase(specialization);
        LinkedList<string>* specList = doctorsBySpecialization.search(lowerSpec);
        if (specList) {
            specList->insertBack(doctorID);
        }
        else {
            LinkedList<string> newList;
            newList.insertBack(doctorID);
            doctorsBySpecialization.insert(lowerSpec, newList);
        }

        cout << "[SUCCESS] Doctor registered: " << name << " (" << doctorID << ")" << endl;
        cout << "          Specialization: " << specialization << endl;
        cout << "          Hospital: " << hospital->name << endl;
        cout << "          Contact: " << contactNo << endl;
    }

    void registerDoctorManual() {
        string name, spec, hospitalID, contact;

        cout << "\n=== REGISTER NEW DOCTOR ===\n";

        // Show available hospitals
        cout << "Available Hospitals:\n";
        displayAllHospitals();

        cout << "\nEnter Doctor Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Specialization: ";
        getline(cin, spec);

        cout << "Enter Hospital ID: ";
        cin >> hospitalID;

        cout << "Enter Contact Number: ";
        cin >> contact;

        registerDoctor(name, spec, hospitalID, contact);
    }

    // ==========================================
    // PATIENT MANAGEMENT
    // ==========================================

    // Registers a new patient in the system
    // Data Structure: HashTable - O(1) average insertion for patient lookup
    void registerPatient(string name, string cnic, int age, string disease) {
        // Edge case: Empty name
        if (name.empty()) {
            cout << "[ERROR] Patient name cannot be empty." << endl;
            return;
        }

        // Edge case: Empty CNIC
        if (cnic.empty()) {
            cout << "[ERROR] CNIC cannot be empty." << endl;
            return;
        }

        // Edge case: Invalid age
        if (age < 0 || age > 150) {
            cout << "[ERROR] Invalid age. Age must be between 0 and 150." << endl;
            return;
        }

        // Edge case: Empty disease
        if (disease.empty()) {
            cout << "[ERROR] Disease cannot be empty." << endl;
            return;
        }

        string patientID = generatePatientID();
        Patient patient(patientID, name, cnic, age, disease);

        patientsHash.insert(patientID, patient);
        // Track patient IDs for listing
        patientIDs.insertBack(patientID);

        cout << "[SUCCESS] Patient registered: " << name << " (" << patientID << ")" << endl;
        cout << "          Disease: " << disease << ", Age: " << age << endl;
        cout << "          CNIC: " << cnic << endl;
    }

    void registerPatientManual() {
        string name, cnic, disease;
        int age;

        cout << "\n=== REGISTER NEW PATIENT ===\n";
        cout << "Enter Patient Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter CNIC (without dashes): ";
        getline(cin, cnic);

        cout << "Enter Age: ";
        cin >> age;

        cout << "Enter Disease: ";
        cin.ignore();
        getline(cin, disease);

        registerPatient(name, cnic, age, disease);
    }

    void assignDoctorToPatient(string patientID, string doctorID) {
        Patient* patient = patientsHash.search(patientID);
        Doctor* doctor = doctorsHash.search(doctorID);

        if (!patient || !doctor) {
            cout << "[ERROR] Patient or Doctor not found" << endl;
            return;
        }

        patient->assignedDoctorID = doctorID;
        patientsHash.insert(patientID, *patient);

        cout << "[SUCCESS] Dr. " << doctor->name << " assigned to "
            << patient->name << endl;
    }

    // ==========================================
    // SEARCH OPERATIONS
    // ==========================================

    void searchMedicineByFormula(string formula) {
        cout << "\n=== SEARCH MEDICINE BY FORMULA ===\n";
        cout << "Formula: " << formula << endl;
        cout << "-----------------------------------\n";

        string lowerFormula = toLowerCase(formula);
        LinkedList<string>* medicineList = medicinesByFormula.search(lowerFormula);

        if (!medicineList) {
            cout << "No medicines found with formula: " << formula << endl;
            return;
        }

        LListNode<string>* current = medicineList->getHead();
        int count = 0;

        while (current != nullptr) {
            Medicine* med = medicinesHash.search(current->data);
            if (med) {
                count++;
                Pharmacy* pharm = pharmaciesHash.search(med->pharmacyID);

                cout << count << ". " << med->name << " (" << med->medicineID << ")\n";
                cout << "   Formula: " << med->formula << endl;
                cout << "   Price: Rs. " << med->price << endl;
                cout << "   Stock: " << med->stockQuantity << " units" << endl;
                if (pharm) {
                    cout << "   Pharmacy: " << pharm->name << " [" << pharm->sector << "]" << endl;
                    cout << "   Pharmacy Location: (" << pharm->coords.latitude << ", "
                        << pharm->coords.longitude << ")" << endl;
                }
                cout << endl;
            }
            current = current->next;
        }

        if (count == 0) {
            cout << "No medicines found." << endl;
        }
        else {
            cout << "Total found: " << count << endl;
        }
    }

    void searchMedicineByName(string name) {
        cout << "\n=== SEARCH MEDICINE BY NAME ===\n";
        cout << "Name: " << name << endl;
        cout << "-----------------------------------\n";

        string lowerName = toLowerCase(name);
        LinkedList<string>* medicineList = medicinesByName.search(lowerName);

        if (!medicineList) {
            cout << "No medicines found with name: " << name << endl;
            return;
        }

        LListNode<string>* current = medicineList->getHead();
        int count = 0;

        while (current != nullptr) {
            Medicine* med = medicinesHash.search(current->data);
            if (med) {
                count++;
                Pharmacy* pharm = pharmaciesHash.search(med->pharmacyID);

                cout << count << ". " << med->name << " (" << med->medicineID << ")\n";
                cout << "   Formula: " << med->formula << endl;
                cout << "   Price: Rs. " << med->price << endl;
                cout << "   Stock: " << med->stockQuantity << " units" << endl;
                if (pharm) {
                    cout << "   Pharmacy: " << pharm->name << " [" << pharm->sector << "]" << endl;
                    cout << "   Pharmacy Location: (" << pharm->coords.latitude << ", "
                        << pharm->coords.longitude << ")" << endl;
                }
                cout << endl;
            }
            current = current->next;
        }

        if (count == 0) {
            cout << "No medicines found." << endl;
        }
        else {
            cout << "Total found: " << count << endl;
        }
    }

    void searchDoctorBySpecialization(string specialization) {
        cout << "\n=== DOCTORS BY SPECIALIZATION ===\n";
        cout << "Specialization: " << specialization << endl;
        cout << "-----------------------------------\n";

        string lowerSpec = toLowerCase(specialization);
        LinkedList<string>* doctorList = doctorsBySpecialization.search(lowerSpec);

        if (!doctorList) {
            cout << "No doctors found with specialization: " << specialization << endl;
            return;
        }

        LListNode<string>* current = doctorList->getHead();
        int count = 0;

        while (current != nullptr) {
            Doctor* doc = doctorsHash.search(current->data);
            if (doc) {
                count++;
                Hospital* hosp = hospitalsHash.search(doc->hospitalID);

                cout << count << ". Dr. " << doc->name << " (" << doc->doctorID << ")\n";
                cout << "   Specialization: " << doc->specialization << endl;
                cout << "   Contact: " << doc->contactNo << endl;
                cout << "   Status: " << (doc->isAvailable ? "Available" : "Busy") << endl;
                if (hosp) {
                    cout << "   Hospital: " << hosp->name << " [" << hosp->sector << "]" << endl;
                    cout << "   Hospital Location: (" << hosp->coords.latitude << ", "
                        << hosp->coords.longitude << ")" << endl;
                }
                cout << endl;
            }
            current = current->next;
        }

        if (count == 0) {
            cout << "No doctors found." << endl;
        }
        else {
            cout << "Total found: " << count << endl;
        }
    }
    // Add this helper function to MedicalSector class:
    void connectTempNodeToNearby(string tempNodeID, double currentLat, double currentLon) {
        cout << "[CONNECTING] Finding nearby nodes for " << tempNodeID << "...\n";

        int connectionsMade = 0;
        const int MAX_CONNECTIONS = 3;

        // Get all node IDs
        LListNode<string>* allNodes = cityGraph->getAllNodeIDs().getHead();

        // Simple connection strategy: connect to nearest hospital and pharmacy
        while (allNodes != nullptr && connectionsMade < MAX_CONNECTIONS) {
            string nodeID = allNodes->data;

            // Skip temp node itself
            if (nodeID == tempNodeID) {
                allNodes = allNodes->next;
                continue;
            }

            CityNode* otherNode = cityGraph->getNode(nodeID);
            if (otherNode) {
                // Calculate distance
                double dist = calculateDistance(currentLat, currentLon,
                    otherNode->coords.latitude,
                    otherNode->coords.longitude);

                int distanceKm = (int)dist;
                if (distanceKm < 1) distanceKm = 1;

                // Connect if within reasonable distance
                if (distanceKm <= 5) { // 5 km max
                    cityGraph->connectNodes(tempNodeID, nodeID, distanceKm);
                    cout << "  Connected to " << nodeID << " (" << otherNode->name
                        << ") - " << distanceKm << " km\n";
                    connectionsMade++;
                }
            }

            allNodes = allNodes->next;
        }

        // If no connections made, try to connect to any hospital
        if (connectionsMade == 0) {
            allNodes = cityGraph->getAllNodeIDs().getHead();
            while (allNodes != nullptr && connectionsMade < 1) {
                string nodeID = allNodes->data;
                if (nodeID[0] == 'H') { // Hospital
                    CityNode* hospitalNode = cityGraph->getNode(nodeID);
                    if (hospitalNode) {
                        double dist = calculateDistance(currentLat, currentLon,
                            hospitalNode->coords.latitude,
                            hospitalNode->coords.longitude);
                        int distanceKm = max(1, (int)dist);
                        cityGraph->connectNodes(tempNodeID, nodeID, distanceKm);
                        cout << "  Direct connection to hospital " << nodeID
                            << " - " << distanceKm << " km\n";
                        connectionsMade++;
                    }
                }
                allNodes = allNodes->next;
            }
        }

        cout << "[CONNECTING] Made " << connectionsMade << " connection(s)\n";
    }

    // ==========================================
    // NEAREST HOSPITAL LOOKUP
    // ==========================================
    void findNearestHospital(double currentLat, double currentLon) {
        cout << "\n=== FINDING NEAREST HOSPITAL ===\n";
        cout << "Your Location: (" << currentLat << ", " << currentLon << ")\n";
        cout << "-----------------------------------\n";

        string nearestHospitalID = "";
        double minDistance = 999999.0;
        Hospital* nearestHospital = nullptr;

        // Iterate through all nodes in city graph to find hospitals
        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();

        while (nodeIDList != nullptr) {
            CityNode* cityNode = cityGraph->getNode(nodeIDList->data);

            if (cityNode && cityNode->moduleType == MODULE_MEDICAL) {
                // Check if it's a hospital (ID starts with 'H')
                if (nodeIDList->data[0] == 'H' && nodeIDList->data.length() >= 2) {
                    Hospital* hospital = hospitalsHash.search(nodeIDList->data);

                    if (hospital) {
                        double dist = calculateDistance(currentLat, currentLon,
                            hospital->coords.latitude,
                            hospital->coords.longitude);

                        if (dist < minDistance) {
                            minDistance = dist;
                            nearestHospitalID = nodeIDList->data;
                            nearestHospital = hospital;
                        }
                    }
                }
            }

            nodeIDList = nodeIDList->next;
        }

        if (nearestHospital) {
            cout << "[FOUND] Nearest Hospital:\n";
            cout << "  Name: " << nearestHospital->name << endl;
            cout << "  Hospital ID: " << nearestHospitalID << endl;
            cout << "  Sector: " << nearestHospital->sector << endl;
            cout << "  Distance: " << (int)minDistance << " km" << endl;
            cout << "  Available Beds: " << nearestHospital->availableBeds
                << "/" << nearestHospital->totalBeds << endl;
            cout << "  Emergency Beds: " << nearestHospital->emergencyBeds << endl;
            cout << "  Specialization: " << nearestHospital->specialization << endl;
            cout << "  Location: (" << nearestHospital->coords.latitude << ", "
                << nearestHospital->coords.longitude << ")" << endl;

            // Show route from current location to hospital
            cout << "\nFinding route from your location...\n";

            // Create a temporary node for current location
            string tempNodeID = "TEMP_LOC";

            // FIRST check if node already exists (in case of multiple searches)
            if (cityGraph->getNode(tempNodeID)) {
                // If existing, remove it first (we don't call deleteNode here yet)
                // cityGraph->removeNode(tempNodeID); // intentionally not used for temp node cleanup
            }

            // Add temporary node
            cityGraph->addNode(tempNodeID, MODULE_PUBLIC_FACILITY, "Current Location",
                currentLat, currentLon);

            cout << "[SUCCESS] Temporary node created: " << tempNodeID << endl;

            // Connect temporary node to nearby existing nodes
            cout << "[CONNECTING] Connecting temporary node to nearby nodes...\n";

            // First, try to connect to the nearest hospital directly
            int directDistance = max(1, (int)minDistance);
            cityGraph->connectNodes(tempNodeID, nearestHospitalID, directDistance);
            cout << "  Direct connection to nearest hospital: " << tempNodeID
                << " <-> " << nearestHospitalID << " (" << directDistance << " km)\n";

            // Also connect to a few other nearby nodes for better routing
            int connectionsMade = 1; // Already made 1 connection
            const int MAX_CONNECTIONS = 3;
            const double MAX_DISTANCE_KM = 3.0;

            // Find and connect to other nearby nodes
            LListNode<string>* allNodes = cityGraph->getAllNodeIDs().getHead();
            while (allNodes != nullptr && connectionsMade < MAX_CONNECTIONS) {
                string otherNodeID = allNodes->data;

                // Skip temp node and the hospital we already connected to
                if (otherNodeID == tempNodeID || otherNodeID == nearestHospitalID) {
                    allNodes = allNodes->next;
                    continue;
                }

                CityNode* otherNode = cityGraph->getNode(otherNodeID);
                if (otherNode) {
                    double dist = calculateDistance(currentLat, currentLon,
                        otherNode->coords.latitude,
                        otherNode->coords.longitude);

                    // Connect if within reasonable distance and not too far
                    if (dist <= MAX_DISTANCE_KM) {
                        int distanceKm = max(1, (int)dist);
                        cityGraph->connectNodes(tempNodeID, otherNodeID, distanceKm);
                        cout << "  Connected to " << otherNodeID << " (" << otherNode->name
                            << ") - " << distanceKm << " km\n";
                        connectionsMade++;
                    }
                }

                allNodes = allNodes->next;
            }

            cout << "[CONNECTING] Made " << connectionsMade << " total connection(s)\n";

            // Now find the shortest path
            cout << "\n=== CALCULATING SHORTEST PATH ===\n";
            cityGraph->findShortestPath(tempNodeID, nearestHospitalID);

            // Clean up temp node
            cout << "\n[INFO] Cleaning up temporary node...\n";

            // Use deleteNode in MainCityGraph when available. For now leave commented so you can implement later:
            // cityGraph->deleteNode(tempNodeID);   // TODO: Implement deleteNode in MainCityGraph
            // (We intentionally do not call removeNode here to avoid inconsistent behavior)
            cout << "[NOTE] Temporary node cleanup deferred (call cityGraph->deleteNode when implemented).\n";

        }
        else {
            cout << "[ERROR] No hospitals found in the system" << endl;
        }
    }

    void findNearestHospitalFromNode(string currentNodeID) {
        CityNode* currentNode = cityGraph->getNode(currentNodeID);

        if (!currentNode) {
            cout << "[ERROR] Current node not found: " << currentNodeID << endl;
            return;
        }

        findNearestHospital(currentNode->coords.latitude, currentNode->coords.longitude);
    }

    // ==========================================
    // EMERGENCY BED AVAILABILITY SYSTEM
    // ==========================================

    void findEmergencyHospitals(double patientLat, double patientLon, int minEmergencyBedsRequired = 1) {
        cout << "\n=== EMERGENCY HOSPITAL SEARCH ===\n";
        cout << "Patient Location: (" << patientLat << ", " << patientLon << ")\n";
        cout << "Minimum Emergency Beds Required: " << minEmergencyBedsRequired << endl;
        cout << "-----------------------------------\n";

        EmergencyBedMaxHeap emergencyHeap(100);

        // Collect all hospitals with emergency beds
        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        int hospitalCount = 0;

        while (nodeIDList != nullptr) {
            if (nodeIDList->data[0] == 'H' && nodeIDList->data.length() >= 2) {
                Hospital* hospital = hospitalsHash.search(nodeIDList->data);

                if (hospital && hospital->emergencyBeds >= minEmergencyBedsRequired) {
                    double dist = calculateDistance(patientLat, patientLon,
                        hospital->coords.latitude,
                        hospital->coords.longitude);

                    EmergencyCase ec(nodeIDList->data, hospital->availableBeds,
                        hospital->emergencyBeds, dist, hospital->name);
                    emergencyHeap.insert(ec);
                    hospitalCount++;
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (emergencyHeap.isEmpty()) {
            cout << "[CRITICAL] No hospitals with emergency beds found!" << endl;
            cout << "Searching for hospitals with any available beds..." << endl;

            // Fallback: Search for any hospital with available beds
            nodeIDList = cityGraph->getAllNodeIDs().getHead();
            while (nodeIDList != nullptr) {
                if (nodeIDList->data[0] == 'H' && nodeIDList->data.length() >= 2) {
                    Hospital* hospital = hospitalsHash.search(nodeIDList->data);
                    if (hospital && hospital->availableBeds > 0) {
                        double dist = calculateDistance(patientLat, patientLon,
                            hospital->coords.latitude,
                            hospital->coords.longitude);
                        EmergencyCase ec(nodeIDList->data, hospital->availableBeds,
                            0, dist, hospital->name);
                        emergencyHeap.insert(ec);
                    }
                }
                nodeIDList = nodeIDList->next;
            }

            if (emergencyHeap.isEmpty()) {
                cout << "[CRITICAL] No hospitals with any beds available!" << endl;
                return;
            }

            cout << "Found " << emergencyHeap.getSize() << " hospitals with regular beds (no emergency beds available)\n";
        }
        else {
            cout << "Found " << hospitalCount << " hospitals with emergency beds\n";
        }

        cout << "\n=== TOP HOSPITALS FOR EMERGENCY ===\n";
        cout << "(Sorted by emergency beds, then available beds, then distance)\n";
        cout << "------------------------------------------\n";

        // Store top hospitals for later use
        EmergencyCase topHospitals[5];
        int topCount = 0;

        // Display top 5 hospitals
        int displayCount = min(5, emergencyHeap.getSize());
        for (int i = 0; i < displayCount && !emergencyHeap.isEmpty(); i++) {
            EmergencyCase ec = emergencyHeap.extractMax();
            topHospitals[topCount++] = ec;
            Hospital* hospital = hospitalsHash.search(ec.hospitalID);

            if (hospital) {
                cout << i + 1 << ". " << hospital->name << " (" << ec.hospitalID << ")\n";
                cout << "   Emergency Beds: " << ec.emergencyBeds << endl;
                cout << "   Total Available Beds: " << ec.availableBeds << endl;
                cout << "   Distance: " << (int)ec.distanceFromPatient << " km" << endl;
                cout << "   Sector: " << hospital->sector << endl;
                cout << "   Specialization: " << hospital->specialization << endl;
                cout << "   Location: (" << hospital->coords.latitude << ", "
                    << hospital->coords.longitude << ")" << endl;

                // Show emergency contact doctors
                if (!hospital->doctorIDs.getHead()) {
                    cout << "   Emergency Doctors: None registered" << endl;
                }
                else {
                    cout << "   Emergency Contact Doctors: ";
                    LListNode<string>* docNode = hospital->doctorIDs.getHead();
                    int docCount = 0;
                    while (docNode && docCount < 3) {
                        Doctor* doc = doctorsHash.search(docNode->data);
                        if (doc && doc->isAvailable) {
                            if (docCount > 0) cout << ", ";
                            cout << "Dr. " << doc->name << " (" << doc->specialization << ")";
                            docCount++;
                        }
                        docNode = docNode->next;
                    }
                    if (docCount == 0) cout << "None available";
                    cout << endl;
                }
                cout << endl;
            }
        }

        // Offer to admit to the top hospital
        if (topCount > 0) {
            cout << "Would you like to be admitted to the top hospital? (y/n): ";
            char choice;
            cin >> choice;

            if (choice == 'y' || choice == 'Y') {
                cout << "\nEnter Patient ID for admission: ";
                string patientID;
                cin >> patientID;

                bool isEmergencyCase = (topHospitals[0].emergencyBeds > 0);

                if (isEmergencyCase) {
                    if (admitEmergencyPatient(patientID, topHospitals[0].hospitalID)) {
                        cout << "\n[EMERGENCY ADMISSION SUCCESSFUL]\n";
                    }
                }
                else {
                    if (admitPatient(patientID, topHospitals[0].hospitalID, false)) {
                        cout << "\n[REGULAR ADMISSION SUCCESSFUL - No emergency beds available]\n";
                    }
                }

                Hospital* hospital = hospitalsHash.search(topHospitals[0].hospitalID);
                if (hospital) {
                    cout << "Patient admitted to " << hospital->name << endl;
                    cout << "Remaining emergency beds: " << hospital->emergencyBeds << endl;
                    cout << "Remaining total beds: " << hospital->availableBeds << endl;

                    // Show route from patient location to hospital
                    cout << "\nFinding emergency route...\n";
                    string tempNodeID = "EMERGENCY_LOC";
                    cityGraph->addNode(tempNodeID, MODULE_PUBLIC_FACILITY, "Emergency Location",
                        patientLat, patientLon);
                    cityGraph->findShortestPath(tempNodeID, topHospitals[0].hospitalID);

                    // Defer cleanup until deleteNode is implemented:
                    // cityGraph->deleteNode(tempNodeID); // TODO: implement
                    cout << "[NOTE] Temporary emergency node cleanup deferred (call cityGraph->deleteNode when implemented).\n";
                }
            }
        }
    }

    void emergencyBedSearchManual() {
        double lat, lon;
        int minEmergencyBeds;

        cout << "\n=== EMERGENCY HOSPITAL SEARCH ===\n";
        cout << "Enter Patient's Latitude: ";
        cin >> lat;
        cout << "Enter Patient's Longitude: ";
        cin >> lon;
        cout << "Enter Minimum Emergency Beds Required: ";
        cin >> minEmergencyBeds;

        findEmergencyHospitals(lat, lon, minEmergencyBeds);
    }

    void emergencyBedSearchFromNode(string currentNodeID, int minBedsRequired = 1) {
        CityNode* currentNode = cityGraph->getNode(currentNodeID);

        if (!currentNode) {
            cout << "[ERROR] Current node not found: " << currentNodeID << endl;
            return;
        }

        findEmergencyHospitals(currentNode->coords.latitude,
            currentNode->coords.longitude,
            minBedsRequired);
    }

    // ==========================================
    // DATA LOADING FROM CSV FILES
    // ==========================================

    void loadHospitalsFromCSV(string filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "[ERROR] Could not open file: " << filename << endl;
            return;
        }

        string line;
        getline(file, line); // Skip header

        int loadedCount = 0;
        while (getline(file, line)) {
            stringstream ss(line);
            string id, name, sector, bedsStr, specialization;

            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, sector, ',');
            getline(ss, bedsStr, ',');
            getline(ss, specialization);

            // Clean up specialization string (handle quotes and commas)
            if (!specialization.empty()) {
                // Remove quotes if present
                if (specialization[0] == '"') specialization = specialization.substr(1);
                if (!specialization.empty() && specialization.back() == '"')
                    specialization.pop_back();
            }

            int totalBeds = 0;
            try {
                totalBeds = stoi(bedsStr);
            }
            catch (...) {
                totalBeds = 0;
            }
            // For CSV data, assume 20% of beds are emergency beds
            int emergencyBeds = max(1, totalBeds / 5);

            // Check if hospital already exists
            Hospital* existing = hospitalsHash.search(id);
            if (existing) {
                cout << "[WARNING] Hospital " << id << " already exists, updating..." << endl;
                existing->totalBeds = totalBeds;
                existing->availableBeds = totalBeds;
                existing->emergencyBeds = emergencyBeds;
                existing->specialization = specialization;
                hospitalsHash.insert(id, *existing);
                continue;
            }

            double lat, lon;
            // Get unique coordinates for the sector
            generateRandomCoordinatesInSector(sector, lat, lon);

            Hospital hospital(id, name, sector, lat, lon, totalBeds, emergencyBeds, specialization);
            hospitalsHash.insert(id, hospital);

            // Update counters for ID generation
            if (id.size() > 0 && id[0] == 'H' && id.length() >= 2) {
                string numStr = id.substr(1);
                int num = 0;
                try { num = stoi(numStr); }
                catch (...) { num = 0; }
                if (num > hospitalCount) hospitalCount = num;
            }

            // Add to main city graph
            cityGraph->addNodeWithAutoConnect(id, MODULE_MEDICAL, name, lat, lon, sector, &hospital);

            loadedCount++;
        }

        file.close();
        cout << "[SUCCESS] Loaded " << loadedCount << " hospitals from " << filename << endl;
        cout << "[INFO] Emergency beds set to 20% of total beds for CSV data" << endl;
    }

    void loadPharmaciesFromCSV(string filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "[ERROR] Could not open file: " << filename << endl;
            return;
        }

        string line;
        getline(file, line); // Skip header

        int loadedCount = 0;
        while (getline(file, line)) {
            stringstream ss(line);
            string id, name, sector, medicineName, formula, priceStr;

            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, sector, ',');
            getline(ss, medicineName, ',');
            getline(ss, formula, ',');
            getline(ss, priceStr);

            int price = 0;
            try { price = stoi(priceStr); }
            catch (...) { price = 0; }

            // Update counters for ID generation
            if (id.size() > 0 && id[0] == 'P' && id.length() >= 2) {
                string numStr = id.substr(1);
                int num = 0;
                try { num = stoi(numStr); }
                catch (...) { num = 0; }
                if (num > pharmacyCount) pharmacyCount = num;
            }

            // Check if pharmacy already exists
            Pharmacy* existing = pharmaciesHash.search(id);
            if (!existing) {
                double lat, lon;
                // Get unique coordinates for the sector
                generateRandomCoordinatesInSector(sector, lat, lon);

                Pharmacy pharmacy(id, name, sector, lat, lon);
                pharmaciesHash.insert(id, pharmacy);

                // Add to main city graph
                cityGraph->addNodeWithAutoConnect(id, MODULE_MEDICAL, name, lat, lon, sector, &pharmacy);
                loadedCount++;
            }

            // Add the medicine
            addMedicine(id, medicineName, formula, price, 50); // Default stock 50
        }

        file.close();
        cout << "[SUCCESS] Loaded " << loadedCount << " pharmacies from " << filename << endl;
    }

    // ==========================================
    // DISPLAY AND STATISTICS
    // ==========================================

    void displayAllHospitals() {
        cout << "\n=== ALL REGISTERED HOSPITALS ===\n";
        cout << "-----------------------------------\n";

        // Iterate through all city nodes to find hospitals
        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        int count = 0;

        while (nodeIDList != nullptr) {
            if (nodeIDList->data.size() > 0 && nodeIDList->data[0] == 'H' && nodeIDList->data.length() >= 2) {
                Hospital* hospital = hospitalsHash.search(nodeIDList->data);
                if (hospital) {
                    count++;
                    cout << count << ". " << hospital->name << " (" << hospital->hospitalID << ")\n";
                    cout << "   Sector: " << hospital->sector << endl;
                    cout << "   Total Beds: " << hospital->totalBeds << endl;
                    cout << "   Available Beds: " << hospital->availableBeds
                        << " (" << (hospital->totalBeds > 0 ?
                            (hospital->availableBeds * 100 / hospital->totalBeds) : 0) << "%)" << endl;
                    cout << "   Emergency Beds: " << hospital->emergencyBeds << endl;
                    cout << "   Specialization: " << hospital->specialization << endl;
                    cout << "   Location: (" << hospital->coords.latitude << ", "
                        << hospital->coords.longitude << ")" << endl;

                    // Count doctors
                    int doctorCountLocal = 0;
                    LListNode<string>* docNode = hospital->doctorIDs.getHead();
                    while (docNode) {
                        doctorCountLocal++;
                        docNode = docNode->next;
                    }
                    cout << "   Doctors: " << doctorCountLocal << " registered" << endl;

                    cout << endl;
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (count == 0) {
            cout << "No hospitals registered in the system." << endl;
        }
        else {
            cout << "Total Hospitals: " << count << endl;
        }
    }

    void displayAllPharmacies() {
        cout << "\n=== ALL REGISTERED PHARMACIES ===\n";
        cout << "-----------------------------------\n";

        // Iterate through all city nodes to find pharmacies
        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        int count = 0;

        while (nodeIDList != nullptr) {
            if (nodeIDList->data.size() > 0 && nodeIDList->data[0] == 'P' && nodeIDList->data.length() >= 2) {
                Pharmacy* pharmacy = pharmaciesHash.search(nodeIDList->data);
                if (pharmacy) {
                    count++;
                    cout << count << ". " << pharmacy->name << " (" << pharmacy->pharmacyID << ")\n";
                    cout << "   Sector: " << pharmacy->sector << endl;
                    cout << "   Location: (" << pharmacy->coords.latitude << ", "
                        << pharmacy->coords.longitude << ")" << endl;

                    // Count and list medicines
                    int medicineCountLocal = 0;
                    LListNode<string>* medNode = pharmacy->medicineIDs.getHead();
                    cout << "   Medicines (" << (medNode ? "first 5 shown" : "none") << "): ";
                    while (medNode && medicineCountLocal < 5) {
                        Medicine* med = medicinesHash.search(medNode->data);
                        if (med) {
                            if (medicineCountLocal > 0) cout << ", ";
                            cout << med->name;
                            medicineCountLocal++;
                        }
                        medNode = medNode->next;
                    }
                    if (medicineCountLocal == 0) cout << "None";
                    else if (medicineCountLocal == 5) cout << "...";
                    cout << endl;

                    cout << endl;
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (count == 0) {
            cout << "No pharmacies registered in the system." << endl;
        }
        else {
            cout << "Total Pharmacies: " << count << endl;
        }
    }

    void displayAllDoctors() {
        cout << "\n=== ALL REGISTERED DOCTORS ===\n";
        cout << "-----------------------------------\n";

        // We need to iterate through all doctors in hash table
        // Since we don't have direct iterator, we'll use hospital links
        int count = 0;

        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        while (nodeIDList != nullptr) {
            if (nodeIDList->data.size() > 0 && nodeIDList->data[0] == 'H' && nodeIDList->data.length() >= 2) {
                Hospital* hospital = hospitalsHash.search(nodeIDList->data);
                if (hospital) {
                    LListNode<string>* docNode = hospital->doctorIDs.getHead();
                    while (docNode) {
                        Doctor* doctor = doctorsHash.search(docNode->data);
                        if (doctor) {
                            count++;
                            cout << count << ". Dr. " << doctor->name << " (" << doctor->doctorID << ")\n";
                            cout << "   Specialization: " << doctor->specialization << endl;
                            cout << "   Hospital: " << hospital->name << endl;
                            cout << "   Contact: " << doctor->contactNo << endl;
                            cout << "   Status: " << (doctor->isAvailable ? "Available" : "Busy") << endl;
                            cout << endl;
                        }
                        docNode = docNode->next;
                    }
                }
            }
            nodeIDList = nodeIDList->next;
        }

        if (count == 0) {
            cout << "No doctors registered in the system." << endl;
        }
        else {
            cout << "Total Doctors: " << count << endl;
        }
    }

    void displayAllPatients() {
        cout << "\n=== ALL REGISTERED PATIENTS ===\n";
        cout << "-----------------------------------\n";

        LListNode<string>* pid = patientIDs.getHead();
        if (!pid) {
            cout << "No patients registered.\n";
            return;
        }

        int idx = 1;
        while (pid) {
            Patient* p = patientsHash.search(pid->data);
            if (p) {
                cout << idx++ << ". " << p->name << " (" << p->patientID << ") - Age: " << p->age
                    << " - Disease: " << p->disease << " - Admitted: " << (p->isAdmitted ? "Yes" : "No") << endl;
            }
            pid = pid->next;
        }
    }

    void displayMedicalStatistics() {
        cout << "\n=== MEDICAL SECTOR STATISTICS ===\n";
        cout << "-----------------------------------\n";

        int totalHospitals = 0;
        int totalPharmacies = 0;
        int totalDoctors = 0;
        int totalPatients = 0;
        int totalMedicines = 0;
        int availableBeds = 0;
        int totalBeds = 0;
        int emergencyBeds = 0;

        // Count hospitals and beds
        LListNode<string>* nodeIDList = cityGraph->getAllNodeIDs().getHead();
        while (nodeIDList != nullptr) {
            if (nodeIDList->data.size() > 0 && nodeIDList->data[0] == 'H' && nodeIDList->data.length() >= 2) {
                Hospital* hospital = hospitalsHash.search(nodeIDList->data);
                if (hospital) {
                    totalHospitals++;
                    availableBeds += hospital->availableBeds;
                    totalBeds += hospital->totalBeds;
                    emergencyBeds += hospital->emergencyBeds;

                    // Count doctors in this hospital
                    LListNode<string>* docNode = hospital->doctorIDs.getHead();
                    while (docNode) {
                        totalDoctors++;
                        docNode = docNode->next;
                    }
                }
            }
            nodeIDList = nodeIDList->next;
        }

        // Count pharmacies and medicines
        nodeIDList = cityGraph->getAllNodeIDs().getHead();
        while (nodeIDList != nullptr) {
            if (nodeIDList->data.size() > 0 && nodeIDList->data[0] == 'P' && nodeIDList->data.length() >= 2) {
                Pharmacy* pharmacy = pharmaciesHash.search(nodeIDList->data);
                if (pharmacy) {
                    totalPharmacies++;
                    // Count medicines in this pharmacy
                    LListNode<string>* medNode = pharmacy->medicineIDs.getHead();
                    while (medNode) {
                        totalMedicines++;
                        medNode = medNode->next;
                    }
                }
            }
            nodeIDList = nodeIDList->next;
        }

        // Count patients from patientIDs list
        LListNode<string>* pnode = patientIDs.getHead();
        while (pnode) {
            totalPatients++;
            pnode = pnode->next;
        }

        cout << "Hospitals: " << totalHospitals << endl;
        cout << "  Total Beds: " << totalBeds << endl;
        cout << "  Available Beds: " << availableBeds << " ("
            << (totalBeds > 0 ? (availableBeds * 100 / totalBeds) : 0) << "% available)" << endl;
        cout << "  Emergency Beds: " << emergencyBeds << endl;
        cout << "Pharmacies: " << totalPharmacies << endl;
        cout << "Doctors: " << totalDoctors << endl;
        cout << "Patients: " << totalPatients << endl;
        cout << "Medicines in System: " << totalMedicines << endl;
        cout << "-----------------------------------\n";
    }

    // ==========================================
    // GETTER METHODS
    // ==========================================

    Hospital* getHospital(string hospitalID) {
        return hospitalsHash.search(hospitalID);
    }

    Pharmacy* getPharmacy(string pharmacyID) {
        return pharmaciesHash.search(pharmacyID);
    }

    Doctor* getDoctor(string doctorID) {
        return doctorsHash.search(doctorID);
    }

    Patient* getPatient(string patientID) {
        return patientsHash.search(patientID);
    }

    Medicine* getMedicine(string medicineID) {
        return medicinesHash.search(medicineID);
    }

    // ==========================================
    // TESTING FUNCTIONS
    // ==========================================

    void runTestCases() {
        cout << "\n=== RUNNING MEDICAL MODULE TEST CASES ===\n";

        // Test 1: Register hospitals with emergency beds
        cout << "\n[TEST 1] Registering test hospitals with emergency beds...\n";
        registerHospital("Test Hospital A", "F-8", 50, 10, "General");
        registerHospital("Test Hospital B", "G-10", 30, 5, "Cardiology");

        // Test 2: Emergency bed search
        cout << "\n[TEST 2] Emergency bed search test...\n";
        findEmergencyHospitals(33.684, 73.025, 1);

        // Test 3: Medicine search
        cout << "\n[TEST 3] Medicine search test...\n";
        searchMedicineByName("Panadol");

        // Test 4: Nearest hospital
        cout << "\n[TEST 4] Nearest hospital test...\n";
        findNearestHospital(33.684, 73.025);

        cout << "\n=== TEST CASES COMPLETED ===\n";
    }

    // ==========================================
    // INTERACTIVE MENU
    // ==========================================

    // Main interactive menu for medical module
    // Data Structure: Menu-driven interface with switch-case
    void displayMedicalMenu() {
        const string RESET = "\033[0m";
        const string YELLOW = "\033[33m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string WHITE = "\033[37m";
        const string BRIGHT_WHITE = "\033[97m";
        const string CYAN = "\033[36m";
        const string GREEN = "\033[32m";

        int choice;
        do {
            cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
            cout << BRIGHT_YELLOW << "         MEDICAL SECTOR MENU" << RESET << endl;
            cout << BRIGHT_YELLOW << "========================================" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  REGISTRATION" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  1. Register New Hospital" << RESET << endl;
            cout << BRIGHT_WHITE << "  2. Register New Pharmacy" << RESET << endl;
            cout << BRIGHT_WHITE << "  3. Register New Doctor" << RESET << endl;
            cout << BRIGHT_WHITE << "  4. Register New Patient" << RESET << endl;
            cout << BRIGHT_WHITE << "  5. Add Medicine to Pharmacy" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  PATIENT MANAGEMENT" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  6. Admit Patient to Hospital (Regular)" << RESET << endl;
            cout << BRIGHT_WHITE << "  7. Admit Patient to Hospital (Emergency)" << RESET << endl;
            cout << BRIGHT_WHITE << "  8. Discharge Patient" << RESET << endl;
            cout << BRIGHT_WHITE << "  9. Assign Doctor to Patient" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  SEARCH OPERATIONS" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 10. Search Medicine by Name" << RESET << endl;
            cout << BRIGHT_WHITE << " 11. Search Medicine by Formula" << RESET << endl;
            cout << BRIGHT_WHITE << " 12. Search Doctor by Specialization" << RESET << endl;
            cout << BRIGHT_WHITE << " 13. Find Nearest Hospital" << RESET << endl;
            cout << BRIGHT_WHITE << " 14. Emergency Bed Search" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  DISPLAY & STATISTICS" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 15. Display All Hospitals" << RESET << endl;
            cout << BRIGHT_WHITE << " 16. Display All Pharmacies" << RESET << endl;
            cout << BRIGHT_WHITE << " 17. Display All Doctors" << RESET << endl;
            cout << BRIGHT_WHITE << " 18. Display All Patients" << RESET << endl;
            cout << BRIGHT_WHITE << " 19. Display Medical Statistics" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "  DATA LOADING & TESTING" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << " 20. Load Hospitals from CSV" << RESET << endl;
            cout << BRIGHT_WHITE << " 21. Load Pharmacies from CSV" << RESET << endl;
            cout << BRIGHT_WHITE << " 22. Run Test Cases" << RESET << endl << endl;

            cout << BRIGHT_WHITE << "  0. Return to Main Menu" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << "Choose: " << RESET;

            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "[ERROR] Invalid input. Please enter a number." << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                continue;
            }

            cin.ignore();

            string input1, input2, input3, input4;
            double dblInput1, dblInput2;

            switch (choice) {
            case 1:
                registerHospitalManual();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 2:
                registerPharmacyManual();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 3:
                registerDoctorManual();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 4:
                registerPatientManual();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 5:
                addMedicineManual();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 6: {
                cout << endl << CYAN << "Enter Patient ID: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Patient ID cannot be empty." << endl;
                }
                else {
                    cout << CYAN << "Enter Hospital ID: " << RESET;
                    getline(cin, input2);
                    if (input2.empty()) {
                        cout << "[ERROR] Hospital ID cannot be empty." << endl;
                    }
                    else {
                        admitPatient(input1, input2, false);
                    }
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 7: {
                cout << endl << CYAN << "Enter Patient ID: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Patient ID cannot be empty." << endl;
                }
                else {
                    cout << CYAN << "Enter Hospital ID: " << RESET;
                    getline(cin, input2);
                    if (input2.empty()) {
                        cout << "[ERROR] Hospital ID cannot be empty." << endl;
                    }
                    else {
                        admitEmergencyPatient(input1, input2);
                    }
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 8: {
                cout << endl << CYAN << "Enter Patient ID: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Patient ID cannot be empty." << endl;
                }
                else {
                    dischargePatient(input1);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 9: {
                cout << endl << CYAN << "Enter Patient ID: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Patient ID cannot be empty." << endl;
                }
                else {
                    cout << CYAN << "Enter Doctor ID: " << RESET;
                    getline(cin, input2);
                    if (input2.empty()) {
                        cout << "[ERROR] Doctor ID cannot be empty." << endl;
                    }
                    else {
                        assignDoctorToPatient(input1, input2);
                    }
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 10: {
                cout << endl << CYAN << "Enter Medicine Name: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Medicine name cannot be empty." << endl;
                }
                else {
                    searchMedicineByName(input1);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 11: {
                cout << endl << CYAN << "Enter Medicine Formula: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Medicine formula cannot be empty." << endl;
                }
                else {
                    searchMedicineByFormula(input1);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 12: {
                cout << endl << CYAN << "Enter Specialization: " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Specialization cannot be empty." << endl;
                }
                else {
                    searchDoctorBySpecialization(input1);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 13: {
                cout << endl << CYAN << "Enter Current Node ID (or 'coord' for coordinates): " << RESET;
                getline(cin, input1);
                if (input1.empty()) {
                    cout << "[ERROR] Input cannot be empty." << endl;
                }
                else if (input1 == "coord") {
                    cout << CYAN << "Enter Latitude: " << RESET;
                    if (!(cin >> dblInput1)) {
                        cin.clear();
                        cin.ignore(1000, '\n');
                        cout << "[ERROR] Invalid latitude." << endl;
                    }
                    else {
                        cout << CYAN << "Enter Longitude: " << RESET;
                        if (!(cin >> dblInput2)) {
                            cin.clear();
                            cin.ignore(1000, '\n');
                            cout << "[ERROR] Invalid longitude." << endl;
                        }
                        else {
                            findNearestHospital(dblInput1, dblInput2);
                        }
                    }
                    cin.ignore();
                }
                else {
                    findNearestHospitalFromNode(input1);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 14:
                emergencyBedSearchManual();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 15:
                displayAllHospitals();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 16:
                displayAllPharmacies();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 17:
                displayAllDoctors();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 18:
                displayAllPatients();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 19:
                displayMedicalStatistics();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 20: {
                string filename;
                cout << endl << CYAN << "Enter hospitals CSV filename: " << RESET;
                getline(cin, filename);
                if (filename.empty()) {
                    cout << "[ERROR] Filename cannot be empty." << endl;
                }
                else {
                    loadHospitalsFromCSV(filename);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 21: {
                string filename;
                cout << endl << CYAN << "Enter pharmacies CSV filename: " << RESET;
                getline(cin, filename);
                if (filename.empty()) {
                    cout << "[ERROR] Filename cannot be empty." << endl;
                }
                else {
                    loadPharmaciesFromCSV(filename);
                }
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            case 22:
                runTestCases();
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            case 0:
                cout << GREEN << "Returning to main menu..." << RESET << endl;
                break;
            default:
                cout << "[ERROR] Invalid choice. Please enter a number between 0-22." << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
            }
        } while (choice != 0);
    }
};
