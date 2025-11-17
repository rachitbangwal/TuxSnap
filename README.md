# TuxSnap (AlpSnap)

## Overview

**TuxSnap** (referred to as **AlpSnap** in the code) is a simple
snapshot management tool for Linux systems.\
It provides a **C++/Qt graphical user interface (GUI)** that serves as a
frontend for the **alpsnap.sh** bash script.

This tool allows you to **create, restore, remove, and list system
snapshots**, which are rsync-based backups compressed using **tar +
zstd**, and optionally encrypted using **GPG**.

> **Note:** The application requires **root privileges** to run.

------------------------------------------------------------------------

## Features

-   **Create:** Take a new snapshot of configured paths.
-   **Restore:** Restore a selected snapshot and overwrite existing
    files.
-   **Remove:** Delete an existing snapshot.
-   **List:** View all available snapshots stored in the `SNAP_ROOT`.
-   **Settings:** GUI editor for `/etc/alpsnap.conf`.

------------------------------------------------------------------------

## Dependencies

### **Runtime Dependencies (Core Logic)**

Used by `alpsnap.sh`:

-   bash
-   rsync
-   tar
-   zstd
-   gnupg (optional; required if `ENCRYPT_MODE="gpg"`)

### **Build Dependencies (GUI)**

Required to compile the Qt application:

-   g++
-   make
-   qt5-qmake (or qtbase5-dev-tools)
-   qtbase5-dev

------------------------------------------------------------------------

## Installation & Build (WSL - Ubuntu/Debian)

These steps apply to WSL using Ubuntu/Debian.

### **1. Install Dependencies**

``` bash
sudo apt update

# Runtime tools
sudo apt install rsync tar zstd gnupg

# Build tools
sudo apt install g++ make qt5-qmake qtbase5-dev
```

### **2. Build the Application**

Run these from the directory containing `alpsnap.pro`:

``` bash
qmake
make
```

This creates an executable named **alpsnap**.

------------------------------------------------------------------------

## Configuration

The tool uses `/etc/alpsnap.conf` for settings.

### **Copy the Default Config**

``` bash
sudo cp etc/alpsnap.conf /etc/alpsnap.conf
```

### **Create Snapshot Directory & Log File**

``` bash
sudo mkdir /snapshots

sudo touch /var/log/alpsnap.log
sudo chmod 600 /var/log/alpsnap.log
```

### **Optional: Edit Config**

``` bash
sudo nano /etc/alpsnap.conf
```

Adjust `INCLUDE_PATHS` and `EXCLUDE_PATHS` as needed.

------------------------------------------------------------------------

## Running the Application

### **Launch TuxSnap**

Ensure `alpsnap` and `alpsnap.sh` are in the same directory.

Run:

``` bash
sudo ./alpsnap
```

The GUI will open and allow snapshot operations.

------------------------------------------------------------------------
