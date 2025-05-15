#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <numeric>
#include <stdexcept>
#include <iomanip>
// using namespace std;
namespace fs = std::filesystem;
// 목표: Composite 패턴을 이용하여 파일과 디렉터리를 구상하고, 계층 구조에서의 동작을 처리하는 프로그램을 작성하는 것이 목표임

class FilesystemComponent {
  private:
    std::string name;
  public:
    FilesystemComponent(const std::string &n): name(n) {}
    virtual ~FilesystemComponent() = default;
    // 이름과 크기 출력
    virtual void display(int indent = 0) = 0;
    virtual long long getSize() = 0;
    virtual std::string getName() const { return name; }
};

class File : public FilesystemComponent {
  private:
    long long size;
  public:
    File(const std::string &n, long long s): FilesystemComponent(n), size(s) {}
    // 이름과 크기 출력 (override 함)
    void display(int indent = 0) override {
      std::cout << std::string(indent * 2, ' ')
              << getName() << " (" << size << " bytes)" << std::endl;
    }

    long long getSize() override { return size; }
};

class Directory : public FilesystemComponent {
  private:
    std::vector<FilesystemComponent *> children;
  public:
    Directory(const std::string &n): FilesystemComponent(n) {}
    ~Directory() override {
      for (FilesystemComponent *child: children) { delete child; }
    }
    // 이 디렉터리에 파일, 하위 디렉터리 추가
    void add(FilesystemComponent *component) {
      if (component) { children.push_back(component); }
    }

    long long getSize() override {
      long long totalSize = 0;
      for (FilesystemComponent *child: children) {
        if (child) { totalSize += child->getSize(); }
      }
      return totalSize;
    }
    // 디렉터리 이름과 디렉터리에 포함된 모든 파일의 크기 합
    void display(int indent = 0) override {
      std::cout << std::string(indent*2, ' ');
      if (getName() == ".") {
        std::cout << getName() << " (total" << getSize() << "bytes)" << std::endl;
      } else {
        std::cout << getName() << "/ (total " << getSize() << " bytes)" << std::endl;
      }

      for (FilesystemComponent *child: children) {
        if (child) { child->display(indent + 1); }
      }
    }
    // 외부에서 자식 목록에 접근해야 할 경우 사용하는 함수
    std::vector<FilesystemComponent *> &getChildren() { return children; }
};

void buildFileststemTree(const fs::path &currentPath, Directory *parentDir, bool isRoot = false) {
  if (!parentDir) return;

  for (auto &entry: fs::directory_iterator(currentPath)) {
    std::string entryName = entry.path().filename().string();

    if (fs::is_regular_file(entry.status())) {
      long long fileSize = fs::file_size(entry.path());
      parentDir->add(new File(entryName, fileSize));
    } else if (fs::is_directory(entry.status())) {
      Directory *subDir = new Directory(entryName);
      parentDir->add(subDir);
      buildFileststemTree(entry.path(), subDir);
    }
  }
}

int main() {
  fs::path currentPath = ".";
  Directory *root = new Directory(".");
  buildFileststemTree(currentPath, root);
  root->display();
  delete root;
  return 0;
}