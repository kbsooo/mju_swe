# 과제: Design Pattern 구현

## 사용 언어

- C++ or Java
    - Spring 같은 framework 사용 없이
    - C++ 과 Java 기본 문법 및 표준 라이브러리로 동작해야 함

# #1 구조 패턴의 Composite을 이용한 Filesystem 처리

## 목표

- Composite 패턴을 이용하여 파일과 디렉터리를 구상하고, 계층 구조에서의 동작을 처리하는 프로그램을 작성하는 것이 목표임

## 요구사항 #1 클래스 설계

- `class FilesystemComponent`: 파일과 디렉터리의 공동 인터페이스
    - 이름과 크기를 출력하는 `display()` 메서드를 갖는다.
- `class File`: 파일 시스템의 파일에 대응되는 `FilesystemComponent`의 하위 클래스
    - `display()` 메서드를 override 하되, 파일 이름과 크기가 출력되게 한다.
- `class Directory`: 파일 시스템에서 디렉터리에 대응되는 `FilesystemComponent`의 하위 클래스
    - `display()` 메서드를 override 하되, 디렉터리의 이름과 디렉터리에 포함된 모든 파일 (하위 디렉터리의 파일까지 재귀적으로)의 크기 합을 출력한다.
    - 다른 파일이나 디렉터리를 추가하는 `add()` 메서드를 갖는다.

## 요구사항 #2 main 함수에서의 동작

- 현재 디렉터리 기준으로 파일 시스템을 순회하며 `FilesystemComponent` 타입 객체를 만든다.
    - 파일 시스템 순회는
        - Java의 경우 [`java.io`](http://java.io) 같은 패키지를 이용한다.
        - C++ 의 경우 표준 C 라이브러리에 있는 `readdir()` 을 써도 되고,
        C++ 17 이상으로 설정하고 `#include <filesystem>` 안의 기능을 활용해도 된다.
    - 현재 디렉터리에 대응되는 `Directory` 객체를 먼저 만든다.
    - 현재 디렉터리 기준으로 파일 시스템을 순회하며
        - 만일 파일을 만날 경우 이름과 크기 정보를 얻어내서 `File` 객체를 만들어서 `Directory`객체에 추가한다.
        - 디렉터리를 만날 경우 DFS 방식을 이용해서 재귀적으로 처리한다.
        즉, 새 `Directory` 객체를 만들고, 이를 현재 `Directory` 객체에 포함시킨다.
        그리고 이 하위 디렉터리를 계속 순회하며 앞에 언급된`File`과 `Directory`객체 만드는 과정을 반복한다
- 생성된 `FilesystemComponent` 타입 객체에 대해 (실제는 현재 디렉터리를 나타내는 `Directory` 객체) `display()` 메서드를 호출한다.
    - 현재 디렉터리에 대응되는 `Directory` 의 `display()` 가 호출될 것이므로 재귀적으로 동작할 것이다.
    - 이 때 디렉터리 깊이에 따라 들여쓰기를 해준다.

- main 함수 pseudo code 예시

```c
int main() {
	// 자식을 부모에 넣었으므로 문제 없음
	FilesystemComponent *current = new Directory();
	
	for (현재 디렉터리부터 시작; 디렉터리 하위 항목이 있는 동안; 다음 하위 항목 읽기) {
		if (디렉터리의 세부 항목이 파일이면) {
			FilesystemComponent *file = new File(파일 이름, 파일 크기);
			current->add(file);
		} else if (디렉터리의 세부 항목이 디렉터리면) {
			FilesystemComponent *dir = new Directory(디렉터리 이름);
			current->add(dir);
			// 새로 읽은 디렉터리에 대해서 재귀적으로 읽는 작업 적절히 수행하게 함
		}
	}
	
	// 이제 디렉터리의 모든 파일과 디렉터리를 읽었으므로 출력. 재귀적으로 동작할 것이다.
	current->display();
}
```

## 요구사항 #3 제약 조건

- 앞의 3개의 클래스 외에 필요하다면 다른 클래스를 추가할 수 있다.
- 그러나 앞의 3개 클래스는 반드시 Composite 패턴을 따라야 한다.

- 앞의 설명에서 메서드는 이름만 나열했다.
따라서 필요시 반환 타입이나 매개변수 목록을 추가해서 메서드를 정의할 것
- 필요한 경우 클래스에 다른 메서드들을 추가할 수 있다.

- 가짜 파일 목록이 아닌 프로그램을 실행하는 현재 디렉터리 기준으로 파일 시스템 정보를 출력해야 된다.

## 출력 예시

```
./ (total 1252 B)
	file1.txt (123 B)
	file2.txt (234 B)
	subDir1/ (total: 495 B)
		file3.txt (345 B)
		file4.txt (150 B)
	subDir2/ (total: 400 B)
		file5.txt (400 B)
```

# #2 행동 패턴의 Memento를 이용한 Serialize/Deserialize

## 목표

- 앞에서 설계한 `FilesystemComponent`에 Memento 패턴에 대응되는 serialize / deserialize 기능을 추가한다.

## 요구사항 #1 클래스 변경

- `FilesystemComponent` 에 `serialize()` 메서드를 추가한다.
    - 이 메서드는 파일과 디렉터리에 대한 opaque 한 문자열을 반환한다.
        - 파일의 경우 파일 이름, 크기 정보를 포함해야 된다.
        - 디렉터리의 경우 재귀적으로 하위에 가지고 있는 파일/디렉터리 목록을 포함해야 된다.
        - 앞의 `display()`가 반환하는 문자열 그대로를 opaque 한 문자열로 써도 되고, JSON 처럼 잘 알려진 format 의 문자열을 써도 된다.
- `FilesystemComponent`에 가상함수로 `deserialize()`메서드를 추가한다.
    - 이 메서드는 `serialize()`가 생성한 opaque 한 문자열을 입력으로 받는다.
    - `class FIle`의 경우 `deserialize()`를 통해 `File` 객체의 내용을 복원한다.
    - `class Directory`의 경우 `deserialize()` 를 통해 재귀적으로 하위의 `File/Directory` 객체를 생성/복원한다.
    - `File`객체의 `serialize()` 결과를 `Directory` 객체의 `deserialize()` 에 이용하거나, 반대로 `Directory` 객체의 `serialize()` 결과를 `File` 객체의 `deserialize()` 에 이용하는 일은 없다고 가정한다.

## 요구사항 #2 main 함수에서의 동작

- 앞의 과제 #1 에서 현재 디렉터리를 읽어서 이를 반영하는 `FilesystemComponent` 타입의 객체 트리 구조를 반환했다.
- 이번 과제에서는 여기서 반환한 `FilesystemComponent` 에 대해 `serialize()` 를 호출한 후, 그 결과를 새로운 `Directory` 객체를 만들어서 `deserialize()` 하게끔 한다.
- 코드 예시 (C++)

```c
int main() {
	FilesystemComponent *current = ... // 현재 디렉터리를 읽어서 Directory 객체 반환
																		 // 자식을 부모 타입에 대입하는 것이니 문제 없음
	current->display(); // 1번 과제에 의해 그 결과를 출력했을 것임
	
	// 2번 과제에서는 serialize()를 호출하고 그 결과를 deserialize()에 활용함
	string opaque = current->serialize();

	FilesystemComponent *newCurrent = new Directory(); // 새로운 디렉터리 객체 생성
	newCurrent->deserialize(opaque); // 직렬화된 문자열을 역직렬화하여 새로운 객체에 저장
	newCurrent->display(); // 1번 과제의 것과 같은 결과가 나와야 함
}
```

# 제출물 및 제출기한

- 과제 #1과 과제 #2를 모두 다 구현 후 빌드에 필요한 소스 파일을 zip으로 묶어서 제출할 것
    - 만일 과제 #1 만 구현한 경우 거기까지만 zip으로 묶어서 제출할 것
    - (논리적으로 과제 #2만 구현하는 경우는 없을 것이라고 판단함)
- 기한: 2025년 5월 16일 오후 11:59