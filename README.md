# UASim3D 

## Requirements

* **Operating System:** Windows
* **Environment:** Visual Studio with the "Desktop development with C++" workload installed.
* **Git:** Installed Git client.

---

## Getting Started (Step by step)

1. **Cloning the repository (Clone):**
   Open a terminal in the folder where you want to keep the project and type the following command:
   ```bash
      git clone https://github.com/grzegorzdembek/UASim3D.git
   ```

2. **Opening the project:**
   Navigate to the downloaded folder and double-click the `UASim3D.sln` file. 

3. **Build configuration:**   
   On the top toolbar in Visual Studio, make sure you have the following options selected:
   * Build configuration: **`Release`**
   * Platform: **`x64`**

4. **Build and run:**
   * Press **Ctrl + Shift + B** (or choose `Build -> Build Solution` from the top menu) to build the project. 
   * Press **F5** (or the green "Local Windows Debugger" button) to run the application.

---

## Git Workflow 

To avoid code conflicts, **we do not work directly on the `main` branch**. Every new feature or fix should be added on a separate, dedicated branch.

### 1. Starting work (Update)
Before you start writing code, make sure you have the latest version of the project:
```bash
git checkout main
git pull
```

### 2. Creating a new branch (Branch)
Create a new branch and give it a name related to what you will be working on:
```bash
git checkout -b your-task-name
```
*(e.g., `git checkout -b add-camera`)*

### 3. Saving changes (Commit)
When you've written your code and want to save it in the history:
```bash
# Add all changed files to the staging area:
git add .

# Save changes with a short description:
git commit -m "Short, clear description of what was done"
```

### 4. Pushing changes to the server (Push)
When pushing a new branch to GitHub for the first time, use the following command (the `-u` flag links your local branch to the one on the server):
```bash
git push -u origin your-task-name
```
*For subsequent changes on the same branch, a simple `git push` is enough.*

### 5. Finishing work (Pull Request)
After pushing your changes, go to the repository page on GitHub. You will see a **"Compare & pull request"** button. Click it to submit your code for review. 