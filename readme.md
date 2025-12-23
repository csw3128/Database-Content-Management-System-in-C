# Class Management System (CMS)

## Project Overview

The **Class Management System (CMS)** is a **Command-Line Interface (CLI)** database management application developed in **C**. It enables users to manage student records through a **file-based database system**, supporting essential operations such as inserting, querying, updating, and deleting records.

This project was developed as part of the **INF1002 Programming Fundamentals** module to demonstrate core competencies in:
- File handling  
- Dynamic memory management  
- Structured program design  

---

## Key Features

### 1. Core Database Operations

- **OPEN**  
  Loads student records from a persistent text file into memory.

- **SHOW ALL**  
  Displays all current student records in a formatted table.

- **INSERT**  
  Adds new student records with unique **7-digit IDs**.

- **QUERY**  
  Searches for specific student records by ID.

- **UPDATE**  
  Modifies existing student data (Name, Programme, or Mark).

- **DELETE**  
  Removes a student record after user confirmation.

- **SAVE**  
  Persists all in-memory changes back to the text file.

---

### 2. Enhancement Features

- **Advanced Sorting**  
  Sorts records by **ID** or **Mark** in **ASC (ascending)** or **DESC (descending)** order using an efficient **merge sort** algorithm.

- **Summary Statistics**  
  Generates reports including:
  - Total number of students  
  - Average marks  
  - Highest and lowest performers  

- **Undo / Redo System**  
  Allows users to reverse or reapply recent actions (**Insert, Update, Delete**) to prevent accidental data loss.

- **Database Backup**  
  Automatically creates a `.bak` backup file before saving changes to ensure data integrity.

---

## System Architecture

The CMS uses a **hybrid data structure** design for optimal performance:

- **Linked List**  
  - Maintains ordered storage  
  - Enables efficient sequential traversal (e.g., `SHOW ALL`)

- **Hash Table (Separate Chaining)**  
  - Provides **O(1)** average lookup time  
  - Used for `QUERY`, `UPDATE`, and `DELETE` operations

- **Stacks (LIFO)**  
  - Manages action history  
  - Supports the Undo/Redo functionality