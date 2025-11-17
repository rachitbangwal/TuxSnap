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

### **3. GPG Encryption Setup**

Setting up GPG encryption for TuxSnap involves creating a personal key pair and configuring the application to use it.

#### **Step 1: Generate Your Personal GPG Key**

**Install GPG (if not already installed):**

```bash
sudo apt update
sudo apt install gnupg
```

**Generate your key pair:**

```bash
gpg --full-generate-key
```

Follow the prompts:
- **Key type:** Choose `1` (RSA and RSA)
- **Key size:** Enter `4096` 
- **Expiration:** Press Enter (no expiration)
- **Real name:** Enter your name (e.g., "TuxSnap Admin")
- **Email:** Enter your email (e.g., "admin@wsl.local")
- **Comment:** Leave blank or add optional description
- **Confirm:** Type `O` (Okay) and press Enter

**Create a passphrase when prompted.** This secures your private key and will be required for decryption.

#### **Step 2: Configure TuxSnap to Use Your Key**

**Find your GPG recipient ID:**

```bash
gpg --list-keys
```

Look for your email address in the output (e.g., `admin@wsl.local`). This will be your GPG recipient.

**Configure the application:**

1. Run TuxSnap: `sudo ./alpsnap`
2. Click "Settings" 
3. Set `ENCRYPT_MODE` to `gpg`
4. Set `GPG_RECIPIENT` to your email address
5. Click "Apply"

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
