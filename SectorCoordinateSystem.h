#pragma once

#include "CityEntities.h"

// ==========================================
// ISLAMABAD SECTOR GRID SYSTEM
// ==========================================

struct SectorBounds {
    string sectorName;      // "F-6", "G-10", "Blue Area"
    double latMin, latMax;
    double lonMin, lonMax;

    SectorBounds(string name = "", double minLat = 0, double maxLat = 0, double minLon = 0, double maxLon = 0)
        : sectorName(name), latMin(minLat), latMax(maxLat),
        lonMin(minLon), lonMax(maxLon) {
    }
};

class SectorCoordinateSystem {
private:
    LinkedList<SectorBounds> sectors;

    // Generate random coordinate within range
    double randomInRange(double min, double max) {
        // Simple pseudo-random (you can improve this)
        double range = max - min;
        double random = (double)(rand() % 1000) / 1000.0;  // 0.0 to 1.0
        return min + (random * range);
    }

public:
    SectorCoordinateSystem() {
        // Initialize Islamabad sectors with real coordinate ranges [web:39]
        initializeIslamabadSectors();
    }

    void initializeIslamabadSectors() {
        // ==========================================
        // ZONE 1: F-SECTORS (Western/Central)
        // ==========================================
        sectors.insertBack(SectorBounds("F-5", 33.7180, 33.7280, 73.0600, 73.0720));
        sectors.insertBack(SectorBounds("F-6", 33.7180, 33.7280, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("F-7", 33.7080, 33.7180, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("F-8", 33.7080, 33.7180, 73.0360, 73.0480));
        sectors.insertBack(SectorBounds("F-9", 33.7080, 33.7180, 73.0720, 73.0840));  // East side
        sectors.insertBack(SectorBounds("F-10", 33.6880, 33.6980, 73.0360, 73.0480));
        sectors.insertBack(SectorBounds("F-11", 33.6780, 33.6880, 73.0360, 73.0480));

        // ==========================================
        // ZONE 2: G-SECTORS (Central)
        // ==========================================
        sectors.insertBack(SectorBounds("G-5", 33.7180, 33.7280, 73.0720, 73.0840));
        sectors.insertBack(SectorBounds("G-6", 33.7080, 33.7180, 73.0600, 73.0720));
        sectors.insertBack(SectorBounds("G-7", 33.6980, 33.7080, 73.0600, 73.0720));
        sectors.insertBack(SectorBounds("G-8", 33.6980, 33.7080, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("G-9", 33.6880, 33.6980, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("G-10", 33.6780, 33.6880, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("G-11", 33.6680, 33.6780, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("G-13", 33.6580, 33.6680, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("G-14", 33.6480, 33.6580, 73.0480, 73.0600));
        sectors.insertBack(SectorBounds("G-15", 33.6380, 33.6480, 73.0480, 73.0600));

        // ==========================================
        // ZONE 3: H-SECTORS (Eastern)
        // ==========================================
        sectors.insertBack(SectorBounds("H-8", 33.6980, 33.7080, 73.0720, 73.0840));
        sectors.insertBack(SectorBounds("H-9", 33.6880, 33.6980, 73.0720, 73.0840));
        sectors.insertBack(SectorBounds("H-10", 33.6780, 33.6880, 73.0720, 73.0840));
        sectors.insertBack(SectorBounds("H-11", 33.6680, 33.6780, 73.0720, 73.0840));
        sectors.insertBack(SectorBounds("H-12", 33.6580, 33.6680, 73.0720, 73.0840));
        sectors.insertBack(SectorBounds("H-13", 33.6480, 33.6580, 73.0720, 73.0840));

        // ==========================================
        // ZONE 4: I-SECTORS (Far Eastern)
        // ==========================================
        sectors.insertBack(SectorBounds("I-8", 33.6980, 33.7080, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-9", 33.6880, 33.6980, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-10", 33.6780, 33.6880, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-11", 33.6680, 33.6780, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-12", 33.6580, 33.6680, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-14", 33.6480, 33.6580, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-15", 33.6380, 33.6480, 73.0840, 73.0960));
        sectors.insertBack(SectorBounds("I-16", 33.6280, 33.6380, 73.0840, 73.0960));

        // ==========================================
        // SPECIAL AREAS
        // ==========================================
        sectors.insertBack(SectorBounds("Blue Area", 33.7150, 33.7230, 73.0580, 73.0660));
        sectors.insertBack(SectorBounds("Zero Point", 33.6980, 33.7030, 73.0540, 73.0600));
        sectors.insertBack(SectorBounds("E-7", 33.7180, 33.7280, 73.0360, 73.0480));  // Western sector

        cout << "[INFO] Initialized Islamabad Sector Grid System (30+ sectors)" << endl;
    }

    // Get random coordinates within a sector [web:36][web:39]
    bool getCoordinatesInSector(string sectorName, double& lat, double& lon) {
        LListNode<SectorBounds>* current = sectors.getHead();

        while (current != nullptr) {
            if (current->data.sectorName == sectorName) {
                lat = randomInRange(current->data.latMin, current->data.latMax);
                lon = randomInRange(current->data.lonMin, current->data.lonMax);
                return true;
            }
            current = current->next;
        }

        cout << "[ERROR] Sector not found: " << sectorName << endl;
        return false;
    }

    // Determine sector from coordinates
    string getSectorFromCoordinates(double lat, double lon) {
        LListNode<SectorBounds>* current = sectors.getHead();

        while (current != nullptr) {
            SectorBounds& sector = current->data;
            if (lat >= sector.latMin && lat <= sector.latMax &&
                lon >= sector.lonMin && lon <= sector.lonMax) {
                return sector.sectorName;
            }
            current = current->next;
        }

        return "Unknown";
    }

    // Check if two sectors are adjacent [web:36]
    bool areSectorsAdjacent(string sector1, string sector2) {
        // Parse sector names (e.g., "F-6" -> letter='F', number=6)
        char letter1 = sector1[0];
        char letter2 = sector2[0];

        // Extract number (handle "F-10" vs "F-6")
        int num1 = 0, num2 = 0;
        for (int i = 2; i < sector1.length(); i++) {
            if (sector1[i] >= '0' && sector1[i] <= '9') {
                num1 = num1 * 10 + (sector1[i] - '0');
            }
        }
        for (int i = 2; i < sector2.length(); i++) {
            if (sector2[i] >= '0' && sector2[i] <= '9') {
                num2 = num2 * 10 + (sector2[i] - '0');
            }
        }

        // Adjacent if: same letter, consecutive numbers OR same number, consecutive letters
        bool sameLetterConsecutiveNum = (letter1 == letter2) && (myAbs(num1 - num2) == 1);
        bool consecutiveLetterSameNum = (myAbs(letter1 - letter2) == 1) && (num1 == num2);

        return sameLetterConsecutiveNum || consecutiveLetterSameNum;
    }

    void displayAllSectors() {
        cout << "\n=== ISLAMABAD SECTORS (30+ Total) ===\n";
        LListNode<SectorBounds>* current = sectors.getHead();
        int count = 1;

        while (current != nullptr) {
            cout << count++ << ". " << current->data.sectorName << endl;
            current = current->next;
        }
    }

    // Get all sector names as a linked list
    LinkedList<string> getAllSectorNames() {
        LinkedList<string> sectorNames;
        LListNode<SectorBounds>* current = sectors.getHead();
        while (current != nullptr) {
            sectorNames.insertBack(current->data.sectorName);
            current = current->next;
        }
        return sectorNames;
    }
};
