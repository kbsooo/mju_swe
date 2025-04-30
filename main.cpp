#include <iostream>
using namespace std;

class FilesystemComponent
{
public:
  FilesystemComponent() {}
  virtual void display() = 0;
};

class File : public FilesystemComponent
{
public:
  File() {}
  void display() override
  {
    cout << "file display" << endl;
  }
};

class Directory : public FilesystemComponent
{
public:
  Directory() {}
  void display() override
  {
    cout << "directory display" << endl;
  }
  void add() {}
};

int main()
{
  return 0;
}