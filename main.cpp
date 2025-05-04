#include <filesystem>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;
namespace fs = std::filesystem;

// 목표: Composite 패턴을 이용하여 파일과 디렉터리를 구상하고, 계층 구조에서의
// 동작을 처리하는 프로그램을 작성하는 것이 목표임

class FilesystemComponent {
 private:
  string name;

 public:
  FilesystemComponent(const string &n) : name(n) {}
  virtual ~FilesystemComponent() = default;
  // 이름과 크기 출력
  virtual void display(int indent = 0) const = 0;
  virtual string getName() const { return name; }
  virtual long long getSize() = 0;
};

class File : public FilesystemComponent {
 private:
  long long size;

 public:
  File(const string &n, long long s) : FilesystemComponent(n), size(s) {}
  void display(int indent = 0) const override {
    cout << string(indent * 2, ' ') << "File " << getName() << " (" << size
         << "bytes)" << endl;
  }
};

class Directory : public FilesystemComponent {
 private:
  vector<FilesystemComponent *> components;

 public:
  Directory(const string &n) : FilesystemComponent(n) {}
  ~Directory() override { for (FilesystemComponent) }
  void display() override {
    // 디렉터리 이름과 디렉터리에 포함된 모든 파일의 크기 합
    cout << "directory display" << endl;
  }
  // 파일이나 디렉터리를 추가
  void add() {}
};

int main() {
  // 현재 디렉터리 기준으로 파일 시스템 순회하며 FileSystemComponent 타입 객체를
  // 만든다 <filesystem> 안의 기능 활용 현재 디렉터리에 대응되는 Directory객체를
  // 먼저 만든다 현재 디렉터리 기준으로 파일 시스템을 순회하며 파일을 만날 경우
  // 이름과 크기 정보를 얻어서 File 객체를 만들어서 Directory 객체에 추가
  // 디렉터리를 만날 경우 DFS 방식으로 재귀적으로 처리한다
  // 즉 새 Directory 객체를 만들고 이를 현재 Directory 객체에 포함시킨다
  // 그리고 이 하위 디렉터리를 계속 순회하며 앞에 언급된 File과 Directory 객체
  // 만드는 과정을 반복한다 생성된 FilesystemComponent 타입 객체에 대해 (실제는
  // 현재 디렉터리를 나타내는 Directory 객체) display() 메서드를 호출한다. 현재
  // 디렉터리에 대응되는 Directory의 display()가 호출될 것이므로 재귀적으롣
  // 동작할 것 디렉터리 깊이에 따라 들여쓰기 해준다

  FilesystemComponent *current = new Directory();

  for (현재 디렉터리부터 시작; 디렉터리 하위 항목이 있는 동안;
       다음 하위 항목 읽기) {
    if (디렉터리의 세부 항목이 파일이면) {
      FilesystemComponent *file = new File(파일 이름, 파일 크기);
      current->add(file);
    } else if (디렉터리의 세부 항목이 디렉터리면) {
      FilesystemComponent *dir = new Directory(디렉터리 이름);
      current->add(dir);
      // 새로 읽은 디렉터리에 대해서 재귀적으로 읽는 작업 적절히 수행하게 함
    }
  }

  // 이제 디렉터리의 모든 파일과 디렉터리를 읽었으므로 출력. 재귀적으로 동작할
  // 것이다.
  current->display();

  return 0;
}