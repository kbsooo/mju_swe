// 60212158 강병수
// c++ 17 이상 버전으로 컴파일 해야합니다!
// $ g++ -std=c++17 main.cpp

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <numeric>
#include <stdexcept>
#include <iomanip>
#include <sstream>
// using namespace std;
namespace fs = std::filesystem;
// 목표: Composite 패턴을 이용하여 파일과 디렉터리를 구상하고, 계층 구조에서의 동작을 처리하는 프로그램을 작성하는 것이 목표임

class FilesystemComponent {
  protected:
    std::string name;
  public:
    FilesystemComponent(const std::string &n): name(n) {}
    virtual ~FilesystemComponent() = default;
    // 이름과 크기 출력
    virtual void display(int indent = 0) = 0;
    virtual long long getSize() = 0;
    virtual std::string getName() const { return name; }

    // 과제2
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
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

    // 과제2
    std::string serialize() const override {
      std::ostringstream oss;
      oss << "F|" << getName() << "|" << size;
      return oss.str();
    }

    void deserialize(const std::string& data) override {
      std::istringstream iss(data.substr(2));
      std::string name_str;
      std::string size_str;

      if (std::getline(iss, name_str, '|') && std::getline(iss, size_str)) {
        this->name = name_str;
        this->size = std::stoll(size_str);
      }
    }
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
        std::cout << getName() << "/ (total" << getSize() << "bytes)" << std::endl;
      } else {
        std::cout << getName() << "/ (total " << getSize() << " bytes)" << std::endl;
      }

      for (FilesystemComponent *child: children) {
        if (child) { child->display(indent + 1); }
      }
    }
    // 외부에서 자식 목록에 접근해야 할 경우 사용하는 함수
    std::vector<FilesystemComponent *> &getChildren() { return children; }

    // 과제2
    std::string serialize() const override {
      std::ostringstream oss;
      oss << "D|" << getName() << "|" << children.size() << "[";
      // 모든 자식들 재귀적으로 serialiez 호출하여 문자열에 추가
      for (const auto* child: children) {
        if (child) { oss << child->serialize(); }
      }
      oss << "]";
      return oss.str();
    }

    // 직렬화된 문자열로부터 디렉터리 객체의 상태와 하위 구조 복원
    void deserialize(const std::string& data) override {
      for (FilesystemComponent* child: this->children) { delete child; }
      this->children.clear();

      size_t first_pipe = data.find('|');
      size_t second_pipe = data.find('|', first_pipe+1);
      size_t bracket_open = data.find('[', second_pipe+1);

      this->name = data.substr(first_pipe+1, second_pipe - (first_pipe+1));
      int num_children = 0;
      if (second_pipe != std::string::npos && bracket_open != std::string::npos && bracket_open > second_pipe+1) {
        num_children = std::stoi(data.substr(second_pipe + 1, bracket_open - (second_pipe + 1)));
      }

      // 자식 없으면 바로 반환
      if (num_children == 0) { return; }

      size_t current_pos = bracket_open+1;

      for (int i = 0; i < num_children; ++i) {
        if (current_pos >= data.length()-1 || data[current_pos] == ']')
          break;

        char child_type = data[current_pos];
        size_t segment_start = current_pos;
        size_t scan_pos = current_pos;

        if (child_type == 'F') {
          size_t p1 = data.find('|', scan_pos+1);
          size_t p2 = std::string::npos;
          if (p1 != std::string::npos) p2 = data.find('|', p1+1);

          size_t temp_end = (p2 != std::string::npos) ? p2+1 : data.length();
          while (temp_end < data.length() && std::isdigit(data[temp_end]))
          temp_end++;
          scan_pos = temp_end;
        } else if (child_type == 'D') {
          scan_pos++;
          int bracket_balance = 0;
          bool found_child_opening_bracket = false;
          size_t child_content_start_bracket = data.find('[', scan_pos);

          if (child_content_start_bracket != std::string::npos) {
            scan_pos = child_content_start_bracket;
          } else {
            scan_pos = data.length();
          }

          while(scan_pos < data.length()) {
            if (data[scan_pos] == '[') {
              bracket_balance++;
              found_child_opening_bracket = true;
            } else if (data[scan_pos] == ']') {
              bracket_balance--;
            }

            if (found_child_opening_bracket && bracket_balance == 0) {
              scan_pos++;
              break;
            }
            scan_pos++;
          }
        } else {
          break;
        }

        std::string child_segment_str = data.substr(segment_start, scan_pos - segment_start);

        FilesystemComponent* child = nullptr;
        if (child_type == 'F') {
          child = new File("", 0); 
        } else if (child_type == 'D') {
          child = new Directory(""); 
        } else {
          continue; 
        }

        if (child) {
          child->deserialize(child_segment_str); 
          this->add(child); 
        }
        current_pos = scan_pos; 
      }
    }
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
  std::cout << "과제1:" << std::endl;
  root->display();

  std::cout << "\n 과제2:" << std::endl;
  std::string opaque_data = "";
  opaque_data = root->serialize();
  Directory *newRoot = new Directory("");
  newRoot->deserialize(opaque_data);
  newRoot->display(); 
  delete newRoot;

  delete root;
  return 0;
}