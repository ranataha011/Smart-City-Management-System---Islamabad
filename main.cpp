#include <iostream>
#include <string>
#include <cctype>
using namespace std;

#include "MainCityGraph.h"
#include "Education.h"
#include "Population.h"
#include "Medical.h"
#include "TransportModule.h"
#include "Mall.h"
#include "Public_Facilities.h"
#include "IslamabadHierarchy.h"

int main() {
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
    const string BLUE = "\033[34m";

    cout << endl << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl;
    cout << BRIGHT_YELLOW << BOLD << "  ISLAMABAD SMART CITY MANAGEMENT SYSTEM" << RESET << endl;
    cout << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl;
    cout << CYAN << "[INFO] Initializing Islamabad City Graph..." << RESET << endl << endl;

    MainCityGraph cityGraph;

    // Initialize all modules
    EducationModule education(&cityGraph);
    PopulationModule population(&cityGraph);
    MedicalSector medical(&cityGraph);
    TransportModule transport(&cityGraph);
    CommercialSector commercial(&cityGraph);
    PublicFacilitiesModule facilities(cityGraph);

    cout << GREEN << "[SUCCESS] All modules initialized successfully!" << RESET << endl;

    while (true) {
        cout << endl << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "              MAIN MENU" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl << endl;

        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "  CITY MODULES" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_WHITE << "  1. Education Module" << RESET << endl;
        cout << BRIGHT_WHITE << "  2. Population & Housing Module" << RESET << endl;
        cout << BRIGHT_WHITE << "  3. Medical Module" << RESET << endl;
        cout << BRIGHT_WHITE << "  4. Transport Module" << RESET << endl;
        cout << BRIGHT_WHITE << "  5. Commercial Module (Malls & Stores)" << RESET << endl;
        cout << BRIGHT_WHITE << "  6. Public Facilities Module" << RESET << endl;
        cout << BRIGHT_WHITE << "  7. Islamabad Hierarchy & B-Tree" << RESET << endl << endl;

        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "  CITY GRAPH OPERATIONS" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_WHITE << "  8. Display Entire City Graph" << RESET << endl;
        cout << BRIGHT_WHITE << "  9. Find Shortest Path Between Nodes" << RESET << endl;
        cout << BRIGHT_WHITE << " 10. Remove Node from Graph" << RESET << endl;
        cout << BRIGHT_WHITE << " 11. Display Graph Statistics" << RESET << endl << endl;

        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "  EMERGENCY MODE" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_WHITE << " 12. Open Emergency Paths (Highway Shortcuts)" << RESET << endl;
        cout << BRIGHT_WHITE << " 13. Close Emergency Paths" << RESET << endl;
        cout << BRIGHT_WHITE << " 14. Check Emergency Mode Status" << RESET << endl << endl;

        cout << BRIGHT_WHITE << BOLD << "  0. Exit System" << RESET << endl;
        cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
        cout << BRIGHT_YELLOW << BOLD << "Choose: " << RESET;

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << BRIGHT_RED << "[ERROR] Invalid input. Please enter a number." << RESET << endl;
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            continue;
        }
        cin.ignore();

        switch (choice) {
        case 1:
            education.menu();
            break;

        case 2:
            population.menu();
            break;

        case 3:
            medical.displayMedicalMenu();
            break;

        case 4:
            TransportMenu(transport);
            break;

        case 5:
            commercial.displayCommercialMenu();
            break;

        case 6:
            facilities.displayMainMenu();
            break;

        case 7:
            hierarchyMenu(
                cityGraph.getCityGraph(),
                cityGraph.getCityNodesHash(),
                cityGraph.getSectorSystem()
            );
            break;

        case 8:
            cout << endl;
            cityGraph.displayCityGraph();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 9: {
            cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "      FIND SHORTEST PATH" << RESET << endl;
            cout << BRIGHT_YELLOW << "========================================" << RESET << endl << endl;

            cout << CYAN << "Available Node Prefixes:" << RESET << endl;
            cout << BRIGHT_WHITE << "  E_   - Education (Schools)" << RESET << endl;
            cout << BRIGHT_WHITE << "  H_   - Hospitals" << RESET << endl;
            cout << BRIGHT_WHITE << "  T_   - Transport (Bus Stops)" << RESET << endl;
            cout << BRIGHT_WHITE << "  M_   - Malls" << RESET << endl;
            cout << BRIGHT_WHITE << "  MOS_ - Mosques" << RESET << endl;
            cout << BRIGHT_WHITE << "  PRK_ - Parks" << RESET << endl;
            cout << BRIGHT_WHITE << "  WAT_ - Water Coolers" << RESET << endl;
            cout << BRIGHT_WHITE << "  POP_ - Housing/Population" << RESET << endl << endl;

            string startNode;
            string endNode;
            
            cout << CYAN << "Enter Start Node ID (e.g., T_Stop1, E_School1): " << RESET;
            getline(cin, startNode);
            
            if (startNode.empty()) {
                cout << BRIGHT_RED << "[ERROR] Start node ID cannot be empty." << RESET << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }
            
            cout << CYAN << "Enter End Node ID (e.g., H_Hospital1, M_Mall1): " << RESET;
            getline(cin, endNode);
            
            if (endNode.empty()) {
                cout << BRIGHT_RED << "[ERROR] End node ID cannot be empty." << RESET << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }

            if (startNode == endNode) {
                cout << BRIGHT_RED << "[ERROR] Start and end nodes cannot be the same." << RESET << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }

            cout << endl;
            cityGraph.findShortestPath(startNode, endNode);

            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 10: {
            cout << endl << BRIGHT_RED << "========================================" << RESET << endl;
            cout << BRIGHT_RED << BOLD << "      REMOVE NODE FROM GRAPH" << RESET << endl;
            cout << BRIGHT_RED << "========================================" << RESET << endl << endl;

            cout << BRIGHT_RED << "[WARNING] This will permanently remove the node!" << RESET << endl;
            cout << CYAN << "[INFO] All edges will be cleaned up automatically" << RESET << endl;
            cout << CYAN << "[INFO] Neighbors will be reconnected to prevent isolation" << RESET << endl << endl;

            string nodeID;
            cout << CYAN << "Enter Node ID to remove (e.g., T_Stop5, MOS_2): " << RESET;
            getline(cin, nodeID);

            if (nodeID.empty()) {
                cout << BRIGHT_RED << "[ERROR] Node ID cannot be empty." << RESET << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }

            // Check if node exists
            if (!cityGraph.getNode(nodeID)) {
                cout << BRIGHT_RED << "[ERROR] Node not found: " << nodeID << RESET << endl;
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
                break;
            }

            // Confirm deletion
            cout << endl << BRIGHT_RED << "Are you sure you want to remove " << nodeID << "? (y/n): " << RESET;
            char confirm;
            cin >> confirm;
            cin.ignore();

            if (confirm == 'y' || confirm == 'Y') {
                cityGraph.removeNode(nodeID);
            }
            else {
                cout << GREEN << "[INFO] Node removal cancelled." << RESET << endl;
            }

            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 11:
            cout << endl;
            cityGraph.displayStatistics();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 12:
            cityGraph.openEmergencyPaths();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 13:
            cityGraph.closeEmergencyPaths();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 14:
            cout << endl << BRIGHT_YELLOW << "=== EMERGENCY MODE STATUS ===" << RESET << endl;
            if (cityGraph.isEmergencyModeActive()) {
                cout << BRIGHT_RED << "[ACTIVE] Emergency mode is ON" << RESET << endl;
                cout << CYAN << "[INFO] Highway shortcuts are available for fastest routes" << RESET << endl;
            }
            else {
                cout << GREEN << "[INACTIVE] Emergency mode is OFF" << RESET << endl;
                cout << CYAN << "[INFO] Only normal city roads are available" << RESET << endl;
            }
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 0:
            cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  Thank you for using" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  Islamabad Smart City Management System!" << RESET << endl;
            cout << BRIGHT_YELLOW << "========================================" << RESET << endl;
            return 0;

        default:
            cout << BRIGHT_RED << "[ERROR] Invalid choice. Please enter a number between 0-14." << RESET << endl;
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
        }
    }

    return 0;
}
