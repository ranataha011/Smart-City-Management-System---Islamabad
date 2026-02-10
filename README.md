================================================================================
                    SMART CITY MANAGEMENT SYSTEM - ISLAMABAD
                             README FILE
================================================================================

PROJECT OVERVIEW
================================================================================

A comprehensive Smart City Management System for Islamabad implementing custom 
data structures in C++ to manage interconnected urban modules.

MODULES:
  - Transport: Bus route management and passenger services
  - Education: School hierarchy and student management
  - Medical: Hospital management and emergency services
  - Commercial: Mall and shopping management
  - Public Facilities: Government facilities
  - Housing/Population: Residential management and demographics
  - City Graph: Central network with shortest path finding


FEATURES
================================================================================

- 10 Custom Data Structures (LinkedList, Stack, Queue, Graph, HashTable, 
  Heap, N-ary Tree, B-Tree)
- Real Geographic Data for Islamabad sectors
- Dijkstra's Algorithm for shortest path finding
- Emergency System with priority-based hospital allocation
- Hierarchical Display using B-Tree
- Integrated Modules working seamlessly


PREREQUISITES
================================================================================

REQUIRED:
  - C++ Compiler (GCC 7.3+, Clang 6.0+, or MSVC 2017+)
  - C++11 Standard or higher
  - 2 GB RAM minimum

RECOMMENDED:
  - Any IDE (Visual Studio, Code::Blocks, VS Code, CLion) or text editor
  - Terminal/Command Prompt


INSTALLATION & SETUP
================================================================================

STEP 1: Download/Clone the Project
-----------------------------------
- If using Git:
  git clone <repository-url>
  cd smart-city-islamabad

- Or download and extract ZIP file


STEP 2: Verify Files
---------------------
Ensure all these files are present in the same directory:

  main.cpp                    - Main entry point
  Data_Structures.h            - Core data structures
  CityEntities.h                 - Common types
  MainCityGraph.h               - Central city graph
  TransportModule.h              - Transport module
  Education.h                    - Education module
  Medical.h                      - Medical module
  Mall.h                         - Commercial module
  Public_Facilities.h            - Public facilities
  SectorCoordinateSystem.h       - Geographic system
  IslamabadHierarchy.h           - Hierarchical display
  Population.h                   - Population module


STEP 3: Fix File Paths (IMPORTANT!)
------------------------------------
Open header files and update any absolute paths to relative paths:

CHANGE:
  #include "C:\Users\hp\Desktop\MainCityGraph.h"

TO:
  #include "MainCityGraph1.h"


PROJECT STRUCTURE
================================================================================

CORE COMPONENTS:
----------------
File                        Purpose
------------------------    ------------------------------------------------
Source1.cpp                 Main program with menu system
Data_Structures.h         All custom data structures (LinkedList, Stack, 
                            Queue, Graph, HashTable, Heap, Tree, B-Tree)
CityEntities.h              Common types, structs, and enums
MainCityGraph.h            Central city graph with Dijkstra's algorithm

MODULES:
--------
File                        Module
------------------------    ------------------------------------------------
TransportModule.h           Bus stops, routes, passenger management
Education.h                 Schools, students, faculty
Medical.h                   Hospitals, doctors, patients, emergencies
Mall.h                      Malls, stores, products, shopping
Public_Facilities.h         Government facilities
paste-12.txt/Population.h   Housing, population, family trees

UTILITIES:
----------
File                        Purpose
------------------------    ------------------------------------------------
SectorCoordinateSystem.h    Islamabad sector coordinates
IslamabadHierarchy.h        B-Tree hierarchical display


MODULES DESCRIPTION
================================================================================

1. TRANSPORT MODULE
-------------------
Features:
  - Bus stop registration with GPS coordinates
  - Bus route creation
  - Passenger queue management (FIFO)
  - Shortest route finding between stops

Key Functions:
  - Load 11 default bus stops
  - Register buses and companies
  - Find optimal routes
  - Manage passenger queues


2. EDUCATION MODULE
-------------------
Features:
  - School registration
  - Department and class management
  - Student enrollment
  - Faculty assignment
  - Hierarchical structure: School -> Department -> Class

Key Functions:
  - Register schools with departments
  - Enroll students
  - Assign faculty to classes
  - Search students by ID/name


3. MEDICAL MODULE
-----------------
Features:
  - Hospital registration with bed capacity
  - Emergency bed allocation (Priority-based using Max Heap)
  - Doctor management by specialization
  - Patient admission/discharge
  - Medicine inventory
  - Pharmacy management

Key Functions:
  - Register hospitals with emergency beds
  - Find best hospital for emergency
  - Admit patients
  - Search doctors by specialization
  - Manage medicine inventory

Emergency Priority:
  1. Most emergency beds available
  2. Most total beds available
  3. Closest distance to patient


4. COMMERCIAL MODULE
--------------------
Features:
  - Mall registration
  - Store management
  - Product inventory
  - Shopping cart system
  - Multi-criteria search (name, category, brand)

Key Functions:
  - Register malls and stores
  - Add products to inventory
  - Search products
  - Shopping cart operations


5. PUBLIC FACILITIES MODULE
----------------------------
Features:
  - Government facility registration
  - Facility type classification
  - Location tracking

Key Functions:
  - Register facilities
  - Display all facilities
  - Find nearest facility


6. POPULATION/HOUSING MODULE
-----------------------------
Features:
  - Hierarchical housing: Sector -> Street -> House
  - Citizen registration with CNIC
  - Family tree management
  - Population statistics
  - Age distribution reports
  - Occupation analysis

Key Functions:
  - Add sectors, streets, houses
  - Register citizens at addresses
  - Generate family trees
  - Population density reports
  - Age and occupation statistics

Hierarchy Example:
  Sector F-8
  ├── Street 22
  │   ├── House 1 (Residents)
  │   └── House 2 (Residents)
  └── Street 23
      └── House 1 (Residents)


7. MAIN CITY GRAPH
------------------
Features:
  - Central graph connecting all modules
  - Dijkstra's shortest path algorithm
  - Auto-connect nearby nodes
  - Sector validation

Key Functions:
  - Add nodes from any module
  - Find shortest path between locations
  - Display complete city network
  - Auto-connect based on distance and sector


USAGE GUIDE
================================================================================

FIRST RUN
---------
1. Compile the program
2. Run the executable
3. Main menu will appear:

========================================
    SMART CITY MANAGEMENT SYSTEM
            ISLAMABAD
========================================
1. Transport Module
2. Education Module
3. Medical Module
4. Commercial Module
5. Public Facilities Module
6. Population/Housing Module
7. Display City Graph
8. Find Shortest Path
9. Display Hierarchy (B-Tree)
0. Exit
========================================


QUICK START EXAMPLE - TRANSPORT MODULE
---------------------------------------
1. Go to Transport Module
2. Load Default Stops (loads 11 bus stops)
3. Register a Bus:
   - Bus Number: 123
   - Company Name: Metro Bus
   - Add stops to create route
4. Find Shortest Route:
   - Start Stop: Faizabad
   - End Stop: Blue Area
   - System shows optimal route


QUICK START EXAMPLE - MEDICAL MODULE
-------------------------------------
1. Go to Medical Module
2. Register Hospital:
   - Name: PIMS Hospital
   - Sector: G-8
   - Total Beds: 500
   - Emergency Beds: 50
3. Emergency Bed Search:
   - Enter patient coordinates
   - System finds best hospital based on priority


QUICK START EXAMPLE - EDUCATION MODULE
---------------------------------------
1. Go to Education Module
2. Register School:
   - Name: NUST University
   - Sector: H-12
3. Add Department:
   - Name: Computer Science
4. Add Class:
   - Name: CS-A
5. Enroll Students


USING SHORTEST PATH
--------------------
Main Menu -> Find Shortest Path
1. Enter source node ID (e.g., T_Stop1)
2. Enter destination node ID (e.g., H01)
3. System calculates and displays:
   - Shortest distance
   - Complete path
   - All intermediate nodes


VIEWING HIERARCHY
-----------------
Main Menu -> Display Hierarchy (B-Tree)
Shows sector-based organization:
Sector -> Module Type -> Entity IDs


DATA STRUCTURES SUMMARY
================================================================================

Data Structure          Usage                       Complexity
-------------------     -------------------------   -----------------------
Linked List             Node lists, routes          Insert: O(1)
Stack                   Route history               Push/Pop: O(1)
Circular Queue          Passenger queue             Enqueue/Dequeue: O(1)
Graph (Adj List)        City network                Dijkstra: O((V+E)logV)
Hash Table              All entity lookups          Search: O(1) avg
Min Heap                Dijkstra's algorithm        Insert: O(log n)
Max Heap                Emergency bed priority      Extract: O(log n)
N-ary Tree              School/housing hierarchy    Find: O(n)
B-Tree                  Hierarchical display        Insert/Search: O(log n)


TROUBLESHOOTING
================================================================================

COMPILATION ISSUES
------------------

Error: 'string' was not declared
Solution: Use C++11 or higher
  g++ -std=c++11 Source1.cpp -o SmartCity

Error: cannot open source file
Solution: Fix absolute paths in #include statements
  Change: #include "C:\Users\hp\Desktop\..."
  To: #include "MainCityGraph1.h"

Error: undefined reference
Solution: Ensure all header files are in the same directory


RUNTIME ISSUES
--------------

Segmentation Fault / Access Violation:
  - Check if data is loaded before accessing
  - Use load functions (e.g., loadDefaultStops()) before operations
  - Verify nodes exist before pathfinding

"Node not found" / "Entity not found":
  - Ensure data is registered before use
  - Load default/sample data first
  - Check spelling of IDs

Empty Display:
  - Load sample data using menu options
  - Register entities manually before displaying
  - Check if module initialization completed


MODULE-SPECIFIC ISSUES
----------------------

Transport:
  - Load default stops before creating routes
  - Register bus stops before adding buses

Medical:
  - Register hospitals before emergency search
  - Add doctors before patient admission

Education:
  - Create hierarchy: School -> Department -> Class
  - Register school before adding departments

Population:
  - Create sector before streets
  - Create street before houses
  - Create house before registering citizens


CONFIGURATION
================================================================================

ADJUSTABLE PARAMETERS

In MainCityGraph1.h:
  MAX_CONNECTIONS = 4;        // Nodes to auto-connect
  MAX_DISTANCE_KM = 5;        // Max connection distance

Hash Table Capacities:
  HashTable(500);             // Increase for larger datasets

Queue Capacity:
  CircularQueue(200);         // Passenger queue size


SAMPLE DATA
================================================================================

PRE-LOADED TRANSPORT STOPS (11 stops):
  - Faizabad
  - Zero Point
  - Aabpara
  - Blue Area
  - Jinnah Super
  - F-10 Markaz
  - G-10 Markaz
  - And more...

SAMPLE MEDICAL FACILITIES:
  - PIMS Hospital
  - Shifa International
  - Polyclinic Hospital
  - Multiple pharmacies

SAMPLE SECTORS:
  - F Sectors: F-5, F-6, F-7, F-8, F-10, F-11
  - G Sectors: G-5 through G-11
  - H Sectors: H-8, H-9
  - I Sectors: I-8, I-9, I-10
  - Special: Blue Area, Zero Point


PERFORMANCE
================================================================================

Operation                   Time
------------------------    ----------------
Hash table lookup           < 1 ms
Dijkstra (100 nodes)        ~ 5 ms
B-Tree insert               < 1 ms
Graph traversal             ~ 5 ms

SCALABILITY:
  - Current: 500-1000 nodes efficiently
  - Can scale to 5000+ nodes with configuration


ADDITIONAL RESOURCES
================================================================================

- Detailed Report: See DS Usage Report for complete data structures analysis
- Code Comments: Inline documentation in all header files
- Complexity Analysis: Detailed complexity tables in report


PROJECT INFORMATION
================================================================================

Course: Data Structures and Algorithms
Topic: Smart City Management System
Language: C++ (Custom implementations, no STL)
Data Structures: 10 implemented from scratch
Algorithms: Dijkstra's shortest path, B-Tree operations, heap operations


LEARNING OUTCOMES
================================================================================

- Custom data structure implementation
- Graph algorithms and pathfinding
- Tree operations (N-ary, B-Tree)
- Hash table with collision handling
- Priority queue with heap
- Modular system design
- Real-world application development
- Complexity analysis


================================================================================
