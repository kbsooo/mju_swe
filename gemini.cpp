#include <iostream>
#include <vector>
#include <string>
#include <filesystem> // C++17 filesystem library
#include <numeric>    // std::accumulate (optional for size calculation)
#include <stdexcept>  // For exception handling
#include <iomanip>    // For std::setw, std::left for formatting (optional)

namespace fs = std::filesystem; // Alias for convenience

// C++17 filesystem 라이브러리를 사용하기 위한 헤더입니다.
// 컴파일 시 C++17 표준을 사용하도록 설정해야 합니다 (예: g++ -std=c++17 gemini.cpp).

// --- Component Interface (컴포넌트 인터페이스) ---
// 파일과 디렉터리 객체들이 공통으로 가져야 할 인터페이스를 정의합니다.
class FilesystemComponent
{
protected:
  std::string name; // 파일 또는 디렉터리의 이름

public:
  // 생성자: 이름을 인자로 받아 초기화합니다.
  FilesystemComponent(const std::string &n) : name(n) {}
  // 가상 소멸자: 파생 클래스의 객체가 소멸될 때 올바르게 메모리 해제를 하기 위해 필요합니다.
  virtual ~FilesystemComponent() = default;

  // 정보를 출력하는 순수 가상 함수입니다. (이름, 크기 등)
  // indent 매개변수는 계층 구조를 시각적으로 표현하기 위한 들여쓰기 수준입니다.
  virtual void display(int indent = 0) const = 0;
  // 컴포넌트의 크기를 반환하는 순수 가상 함수입니다.
  // 파일의 경우 파일 크기, 디렉터리의 경우 내용물의 총 크기입니다.
  virtual long long getSize() const = 0;
  // 컴포넌트의 이름을 반환하는 가상 함수입니다.
  virtual std::string getName() const { return name; }

  // 참고: Composite 패턴에 따라 add/remove 같은 자식 관리 메서드는
  // 모든 컴포넌트에 공통적이지 않으므로 기본 인터페이스에는 포함하지 않았습니다.
  // 이러한 메서드는 Directory 클래스에만 존재합니다.
};

// --- Leaf Class (리프 클래스) ---
// 파일 시스템의 '파일'에 해당하며, 자식을 가질 수 없는 단일 객체입니다.
class File : public FilesystemComponent
{
private:
  long long size; // 파일의 크기 (바이트 단위)

public:
  // 생성자: 파일 이름과 크기를 받아 초기화합니다.
  File(const std::string &n, long long s) : FilesystemComponent(n), size(s) {}

  // 파일 정보를 출력합니다. (요구사항에 맞춘 형식)
  void display(int indent = 0) const override
  {
    // 들여쓰기를 적용하여 출력합니다.
    std::cout << std::string(indent * 2, ' ')
              << getName() << " (" << size << " bytes)" << std::endl;
  }

  // 파일의 크기를 반환합니다.
  long long getSize() const override
  {
    return size;
  }
};

// --- Composite Class (컴포지트 클래스) ---
// 파일 시스템의 '디렉터리'에 해당하며, 다른 File 또는 Directory 객체들(자식)을 포함할 수 있습니다.
class Directory : public FilesystemComponent
{
private:
  // 자식 컴포넌트들을 저장하는 벡터입니다. FilesystemComponent 포인터를 사용합니다.
  std::vector<FilesystemComponent *> children;

public:
  // 생성자: 디렉터리 이름을 받아 초기화합니다.
  Directory(const std::string &n) : FilesystemComponent(n) {}

  // 소멸자: 동적으로 할당된 자식 컴포넌트들의 메모리를 해제합니다.
  ~Directory() override
  {
    for (FilesystemComponent *child : children)
    {
      delete child; // 각 자식 컴포넌트 삭제
    }
    // children 벡터 자체는 스코프를 벗어날 때 자동으로 메모리가 해제됩니다.
  }

  // 이 디렉터리에 파일 또는 하위 디렉터리(컴포넌트)를 추가합니다.
  void add(FilesystemComponent *component)
  {
    if (component) // null 포인터가 아닌지 확인
    {
      children.push_back(component);
    }
  }

  // 디렉터리 내 모든 내용물(파일 및 하위 디렉터리)의 총 크기를 재귀적으로 계산하여 반환합니다.
  long long getSize() const override
  {
    long long totalSize = 0;
    for (const FilesystemComponent *child : children)
    {
      if (child) // null 체크
      {
        totalSize += child->getSize(); // 각 자식의 크기를 누적
      }
    }
    return totalSize;
  }

  // 디렉터리 정보를 출력하고, 자식 컴포넌트들을 재귀적으로 출력합니다. (요구사항에 맞춘 형식)
  void display(int indent = 0) const override
  {
    std::cout << std::string(indent * 2, ' '); // 들여쓰기 적용
    if (getName() == ".") // 루트 디렉터리 이름이 "."인 경우 특별 처리
    {
      std::cout << getName() << " (total " << getSize() << " bytes)" << std::endl;
    }
    else
    {
      std::cout << getName() << "/ (total " << getSize() << " bytes)" << std::endl;
    }

    // 자식 컴포넌트들을 증가된 들여쓰기로 재귀 호출하여 출력합니다.
    for (const FilesystemComponent *child : children)
    {
      if (child) // null 체크
      {
        child->display(indent + 1);
      }
    }
  }

  // 외부에서 자식 목록에 접근해야 할 경우를 위한 헬퍼 함수 (주의해서 사용)
  const std::vector<FilesystemComponent *> &getChildren() const
  {
    return children;
  }
};

// --- Filesystem Traversal Function (파일 시스템 순회 함수) ---

// 'currentPath'로부터 시작하여 파일 시스템 트리를 재귀적으로 구축하고,
// 'parentDir'에 컴포넌트들을 추가합니다.
void buildFilesystemTree(const fs::path &currentPath, Directory *parentDir, bool isRoot = false)
{
  if (!parentDir) // 안전 검사: 부모 디렉터리가 유효하지 않으면 반환
    return;

  // directory_iterator를 사용하여 디렉터리 내 항목들을 순회합니다.
  // 오류 발생 시 예외를 던지지 않도록 error_code를 사용합니다.
  std::error_code ec;
  fs::directory_iterator iterator(currentPath, fs::directory_options::skip_permission_denied, ec);
  fs::directory_iterator end_iterator; // 기본 생성자로 end iterator 생성

  if (ec) // 디렉터리 접근 오류 처리
  {
    // 접근 오류가 발생하면 메시지를 출력하고 해당 디렉터리 처리를 중단합니다.
    // fs::current_path()와 currentPath가 다를 경우 currentPath.string()이 상대 경로일 수 있으므로 절대 경로로 변환하는 것이 좋을 수 있습니다.
    // 여기서는 간결성을 위해 그대로 사용합니다.
    std::cerr << "Error accessing directory: " << currentPath.string() << " - " << ec.message() << std::endl;
    return;
  }

  while (iterator != end_iterator) // 디렉터리 내 모든 항목을 순회
  {
    const fs::directory_entry &entry = *iterator; // 현재 항목에 대한 참조
    fs::path entryPath = entry.path();            // 현재 항목의 경로
    std::string entryName = entryPath.filename().string(); // 파일 또는 디렉터리 이름만 추출

    try
    {
      // entry.status()를 사용하여 파일 타입을 확인합니다. symlink 등을 고려하여 fs::status()를 사용할 수도 있습니다.
      if (fs::is_regular_file(entry.status(ec)) && !ec) // 일반 파일인 경우
      {
        std::error_code fileSizeEc;
        long long fileSize = fs::file_size(entryPath, fileSizeEc); // 파일 크기 조회
        if (fileSizeEc) // 파일 크기 조회 오류 처리
        {
          std::cerr << "Warning: Could not get size for file: " << entryPath.string() << " - " << fileSizeEc.message() << std::endl;
          fileSize = 0; // 오류 시 크기를 0으로 처리
        }
        FilesystemComponent *file = new File(entryName, fileSize); // File 객체 생성
        parentDir->add(file); // 부모 디렉터리에 추가
      }
      else if (fs::is_directory(entry.status(ec)) && !ec) // 디렉터리인 경우
      {
        Directory *subDir = new Directory(entryName); // 하위 Directory 객체 생성
        parentDir->add(subDir);                       // 현재 디렉터리에 하위 디렉터리 추가
        // 하위 디렉터리에 대해 재귀적으로 buildFilesystemTree 호출
        buildFilesystemTree(entryPath, subDir);
      }
      // 기타 파일 타입(심볼릭 링크 등)은 여기서는 무시합니다. 필요시 추가 처리 가능합니다.
      else if (ec) { // entry.status() 에서 오류 발생 시
          std::cerr << "Warning: Could not determine type for: " << entryPath.string() << " - " << ec.message() << std::endl;
      }
    }
    catch (const fs::filesystem_error &e) // 파일 시스템 관련 예외 처리
    {
      std::cerr << "Filesystem error processing entry " << entryPath.string() << ": " << e.what() << std::endl;
    }
    catch (const std::exception &e) // 기타 표준 예외 처리
    {
      std::cerr << "Standard exception processing entry " << entryPath.string() << ": " << e.what() << std::endl;
    }

    // 다음 항목으로 이동 (오류 처리 포함)
    iterator.increment(ec);
    if (ec) // 반복자 증가 중 오류 발생 시
    {
      std::cerr << "Error iterating directory: " << currentPath.string() << " - " << ec.message() << std::endl;
      break; // 순회 중단
    }
  }
}

// --- Main Function (메인 함수) ---
int main()
{
  // 시작 경로를 정의합니다. 현재 디렉터리(".") 또는 특정 경로를 지정할 수 있습니다.
  // 예: fs::path startPath = "/home/user/documents";
  std::string originalStartPathString = "."; // 초기 경로 문자열 저장 (루트 디렉터리 이름 결정용)
  fs::path startPath = originalStartPathString;

  // 경로를 정규화하거나 절대 경로로 변환하여 표시 이름을 개선합니다.
  std::error_code ec;
  fs::path absoluteStartPath = fs::canonical(startPath, ec); // 정규 경로 시도
  if (ec) // 정규 경로 변환 실패 시 절대 경로로 대체
  {
    absoluteStartPath = fs::absolute(startPath, ec);
    if (ec) // 절대 경로 변환도 실패 시 오류 처리
    {
      std::cerr << "Error resolving starting path: " << startPath.string() << " - " << ec.message() << std::endl;
      return 1; // 프로그램 종료
    }
  }

  // 루트 Directory 객체를 생성합니다.
  // 시작 경로가 "." 이었으면 루트 디렉터리 이름을 "."으로, 아니면 경로의 마지막 부분을 이름으로 사용합니다.
  std::string rootDirName = (originalStartPathString == ".") ? "." : absoluteStartPath.filename().string();
  Directory *rootDirectory = new Directory(rootDirName);

  std::cout << "Scanning directory: " << absoluteStartPath.string() << "\n"
            << std::endl;

  // 파일 시스템 트리를 재귀적으로 구축합니다.
  try
  {
    buildFilesystemTree(absoluteStartPath, rootDirectory, true);
  }
  catch (const fs::filesystem_error &e) // 파일 시스템 스캔 중 치명적 오류 처리
  {
    std::cerr << "Fatal filesystem error during scan: " << e.what() << std::endl;
    delete rootDirectory; // 할당된 메모리 정리
    return 1;
  }
  catch (const std::exception &e) // 기타 치명적 오류 처리
  {
    std::cerr << "Fatal error during scan: " << e.what() << std::endl;
    delete rootDirectory; // 할당된 메모리 정리
    return 1;
  }

  // 구축된 전체 파일 시스템 트리를 출력합니다.
  std::cout << "\n--- Filesystem Hierarchy ---" << std::endl;
  if (rootDirectory)
  {
    rootDirectory->display(); // 루트 디렉터리의 display 호출 (재귀적으로 전체 트리 출력)
  }

  // 할당된 메모리를 정리합니다.
  // rootDirectory를 삭제하면 Directory의 소멸자가 호출되어 모든 자식 컴포넌트들이 재귀적으로 삭제됩니다.
  delete rootDirectory;
  rootDirectory = nullptr; // 댕글링 포인터 방지를 위해 nullptr로 설정

  return 0; // 프로그램 정상 종료
}

// C++17 이상에서 컴파일해야 합니다.
// 예: g++ -std=c++17 gemini.cpp -o filesystem_composite
// 또는 clang++ -std=c++17 gemini.cpp -o filesystem_composite
// MSVC의 경우: cl /std:c++17 gemini.cpp