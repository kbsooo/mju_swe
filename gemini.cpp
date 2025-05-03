#include <iostream>
#include <vector>
#include <string>
#include <filesystem> // C++17 filesystem library
#include <numeric>    // std::accumulate (optional for size calculation)
#include <stdexcept>  // For exception handling
#include <iomanip>    // For std::setw, std::left for formatting (optional)

namespace fs = std::filesystem; // Alias for convenience

// --- Component Interface ---
class FilesystemComponent
{
protected:
  std::string name;

public:
  FilesystemComponent(const std::string &n) : name(n) {}
  virtual ~FilesystemComponent() = default; // Virtual destructor for proper cleanup

  // Display information (name, size) with indentation
  virtual void display(int indent = 0) const = 0;
  // Get the size of the component (file size or total size of directory contents)
  virtual long long getSize() const = 0;
  // Get the name of the component
  virtual std::string getName() const { return name; }

  // --- Composite-specific methods (not pure virtual) ---
  // We avoid putting add/remove in the base interface to adhere strictly
  // to the pattern where Leaves shouldn't have these operations.
  // We'll use dynamic_cast or similar if needed, but prefer direct calls
  // on Directory objects.
};

// --- Leaf Class ---
class File : public FilesystemComponent
{
private:
  long long size;

public:
  File(const std::string &n, long long s) : FilesystemComponent(n), size(s) {}

  void display(int indent = 0) const override
  {
    std::cout << std::string(indent * 2, ' ') // Indentation
              << "File: " << getName() << " (" << size << " bytes)" << std::endl;
  }

  long long getSize() const override
  {
    return size;
  }
};

// --- Composite Class ---
class Directory : public FilesystemComponent
{
private:
  std::vector<FilesystemComponent *> children; // Stores pointers to children

public:
  Directory(const std::string &n) : FilesystemComponent(n) {}

  // Destructor: Clean up owned children components
  ~Directory() override
  {
    // std::cout << "Destroying Directory: " << name << std::endl; // Debug output
    for (FilesystemComponent *child : children)
    {
      delete child; // Delete each child allocated with new
    }
    // The vector itself (children) is automatically destroyed
  }

  // Add a component (File or Directory) to this directory
  void add(FilesystemComponent *component)
  {
    if (component)
    {
      children.push_back(component);
    }
  }

  // Calculate the total size of all contents recursively
  long long getSize() const override
  {
    long long totalSize = 0;
    for (const FilesystemComponent *child : children)
    {
      totalSize += child->getSize();
    }
    // Alternative using std::accumulate:
    // long long totalSize = std::accumulate(children.begin(), children.end(), 0LL,
    //     [](long long sum, const FilesystemComponent* comp) {
    //         return sum + (comp ? comp->getSize() : 0); // Add null check if needed
    //     });
    return totalSize;
  }

  // Display directory info and recursively display children
  void display(int indent = 0) const override
  {
    std::cout << std::string(indent * 2, ' ') // Indentation
              << "Directory: " << getName() << " [Total Size: " << getSize() << " bytes]" << std::endl;
    // Recursively display children with increased indentation
    for (const FilesystemComponent *child : children)
    {
      if (child)
      { // Basic null check
        child->display(indent + 1);
      }
    }
  }

  // Helper to get direct access to children if needed (use with caution)
  const std::vector<FilesystemComponent *> &getChildren() const
  {
    return children;
  }
};

// --- Filesystem Traversal Function ---

// Recursively builds the filesystem tree starting from 'currentPath'
// and adds components to the 'parentDir'.
void buildFilesystemTree(const fs::path &currentPath, Directory *parentDir)
{
  if (!parentDir)
    return; // Safety check

  // Use directory_iterator with error handling options
  std::error_code ec;
  fs::directory_iterator iterator(currentPath, fs::directory_options::skip_permission_denied, ec);
  fs::directory_iterator end_iterator; // Default constructor creates end iterator

  if (ec)
  {
    std::cerr << "Error accessing directory: " << currentPath.string() << " - " << ec.message() << std::endl;
    return; // Stop processing this directory on error
  }

  while (iterator != end_iterator)
  {
    const fs::directory_entry &entry = *iterator;
    fs::path entryPath = entry.path();
    std::string entryName = entryPath.filename().string(); // Get just the filename/dirname

    try
    {
      if (fs::is_regular_file(entry.status()))
      {
        std::error_code fileSizeEc;
        long long fileSize = fs::file_size(entryPath, fileSizeEc);
        if (fileSizeEc)
        {
          std::cerr << "Warning: Could not get size for file: " << entryPath.string() << " - " << fileSizeEc.message() << std::endl;
          fileSize = 0; // Treat as 0 size on error
        }
        FilesystemComponent *file = new File(entryName, fileSize);
        parentDir->add(file);
      }
      else if (fs::is_directory(entry.status()))
      {
        // Create a new Directory object for the subdirectory
        Directory *subDir = new Directory(entryName);
        // Add the new subdirectory to the current parent directory
        parentDir->add(subDir);
        // Recursively build the tree for the subdirectory
        buildFilesystemTree(entryPath, subDir);
      }
      // else: Ignore other types like symlinks, block devices, etc. for simplicity
      //       Could add specific handling here if needed.
    }
    catch (const fs::filesystem_error &e)
    {
      std::cerr << "Filesystem error processing entry " << entryPath.string() << ": " << e.what() << std::endl;
      // Decide whether to continue to the next entry or stop
    }
    catch (const std::exception &e)
    {
      std::cerr << "Standard exception processing entry " << entryPath.string() << ": " << e.what() << std::endl;
    }

    // Increment the iterator (handle potential errors during increment)
    iterator.increment(ec);
    if (ec)
    {
      std::cerr << "Error iterating directory: " << currentPath.string() << " - " << ec.message() << std::endl;
      break; // Stop iteration on error
    }
  }
}

// --- Main Function ---
int main()
{
  // Define the starting path (current directory)
  fs::path startPath = "."; // Or provide a specific path like "/home/user/documents"

  // Get the canonical/absolute path for better display name
  std::error_code ec;
  fs::path absoluteStartPath = fs::canonical(startPath, ec);
  if (ec)
  {
    absoluteStartPath = fs::absolute(startPath, ec); // Fallback to absolute
    if (ec)
    {
      std::cerr << "Error resolving starting path: " << startPath.string() << " - " << ec.message() << std::endl;
      return 1;
    }
  }

  // Create the root Directory object representing the starting path
  // Use the last component of the path as the root directory name
  Directory *rootDirectory = new Directory(absoluteStartPath.filename().string());

  std::cout << "Scanning directory: " << absoluteStartPath.string() << "\n"
            << std::endl;

  // Build the filesystem tree recursively starting from the current path
  try
  {
    buildFilesystemTree(absoluteStartPath, rootDirectory);
  }
  catch (const fs::filesystem_error &e)
  {
    std::cerr << "Fatal filesystem error during scan: " << e.what() << std::endl;
    delete rootDirectory; // Clean up allocated memory before exiting
    return 1;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error during scan: " << e.what() << std::endl;
    delete rootDirectory;
    return 1;
  }

  // Display the entire constructed filesystem tree
  std::cout << "\n--- Filesystem Hierarchy ---" << std::endl;
  rootDirectory->display(); // This will trigger recursive display

  // Clean up the allocated memory
  // Deleting the rootDirectory will trigger the recursive deletion
  // of all child components due to the Directory destructor.
  delete rootDirectory;
  rootDirectory = nullptr; // Good practice to nullify dangling pointers

  return 0;
}