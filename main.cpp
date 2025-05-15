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

// int main() {
//   // 현재 디렉터리 기준으로 파일 시스템 순회하며 FileSystemComponent 타입 객체를 만든다
//   // <filesystem> 안의 기능 활용
//   // 현재 디렉터리에 대응되는 Directory객체를 먼저 만든다
//   // 현재 디렉터리 기준으로 파일 시스템을 순회하며
//   // 파일을 만날 경우 이름과 크기 정보를 얻어서 File 객체를 만들어서 Directory 객체에 추가
//   // 디렉터리를 만날 경우 DFS 방식으로 재귀적으로 처리한다
//   // 즉 새 Directory 객체를 만들고 이를 현재 Directory 객체에 포함시킨다
//   // 그리고 이 하위 디렉터리를 계속 순회하며 앞에 언급된 File과 Directory 객체 만드는 과정을 반복한다
//   // 생성된 FilesystemComponent 타입 객체에 대해 (실제는 현재 디렉터리를 나타내는 Directory 객체) display() 메서드를 호출한다.
//   // 현재 디렉터리에 대응되는 Directory의 display()가 호출될 것이므로 재귀적으롣 동작할 것
//   // 디렉터리 깊이에 따라 들여쓰기 해준다

//   FilesystemComponent *current = new Directory();

//   for (현재 디렉터리부터 시작; 디렉터리 하위 항목이 있는 동안; 다음 하위 항목 읽기) {
// 		if (디렉터리의 세부 항목이 파일이면) {
// 			FilesystemComponent *file = new File(파일 이름, 파일 크기);
// 			current->add(file);
// 		} else if (디렉터리의 세부 항목이 디렉터리면) {
// 			FilesystemComponent *dir = new Directory(디렉터리 이름);
// 			current->add(dir);
// 			// 새로 읽은 디렉터리에 대해서 재귀적으로 읽는 작업 적절히 수행하게 함
// 		}
// 	}
	
// 	// 이제 디렉터리의 모든 파일과 디렉터리를 읽었으므로 출력. 재귀적으로 동작할 것이다.
// 	current->display();

//   return 0;
// }