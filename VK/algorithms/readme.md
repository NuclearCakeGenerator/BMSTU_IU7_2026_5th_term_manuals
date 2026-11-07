Структура рабочей директории:

```
.
├── dynamic_deque       # Папка одной из задач
│   ├── main.cpp
│   ├── test1.in
│   ├── test1.out
│   ├── test2.in
│   └── test2.out
├── set_intersection    # Еще одна папка
│   ├── main.cpp
│   ├── main.exe
│   ├── test1.in
│   ├── test1.out
│   ├── test2.in
│   └── test2.out
└── .vscode
    ├── extensions.json
    ├── launch.json
    ├── run_tests.sh
    └── tasks.json
```

папка `.vscode` содержит настройки для упрощения разработки

`extensions.json`:

```json
{
  "recommendations": [
    "ms-vscode.cpptools", // Для подсветки
    "ms-vscode.cpptools-themes" // Для форматирования
  ]
}
```

`launch.json` - отладка твоей задачки по клавише `F5`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "g++ - Build and Debug active folder",
      "type": "cppdbg",
      "request": "launch",
      "program": "${fileDirname}/${fileBasenameNoExtension}.exe",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${fileDirname}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ],
      "preLaunchTask": "C/C++: g++ build active folder",
      "miDebuggerPath": "/usr/bin/gdb"
    }
  ]
}
```

`tasks.json` - сборка и автотесты открытой программы:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "type": "shell",
      "label": "C/C++: g++ build active folder",
      "command": "/usr/bin/g++",
      "args": [
        "-g",
        "${fileDirname}/*.cpp",
        "-o",
        "${fileDirname}/${fileBasenameNoExtension}.exe"
      ],
      "options": {
        "cwd": "${fileDirname}"
      },
      "problemMatcher": ["$gcc"],
      "presentation": {
        "echo": true,
        "reveal": "silent",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": false,
        "clear": true
      },
      "group": {
        "kind": "build",
        "isDefault": true
      }
    },
    {
      "type": "shell",
      "label": "Run Suite of Tests",
      "command": "${workspaceFolder}/.vscode/run_tests.sh",
      "args": [
        "${fileDirname}/${fileBasenameNoExtension}.exe",
        "${fileDirname}"
      ],
      "dependsOn": ["C/C++: g++ build active folder"],
      "problemMatcher": [],
      "presentation": {
        "echo": true,
        "reveal": "silent",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": true,
        "clear": false
      },
      "group": {
        "kind": "test",
        "isDefault": true
      }
    }
  ]
}
```

`run_tests.sh` - скрипт автотестирования, тестирует твою открытую программу
с файлами `test*.in` и `testN.out`:

```bash
#!/bin/bash

# Назначение аргументов для понятности
EXE="$1"
DIR="$2"

cd "$DIR" || exit 1

echo "=== RUNNING AUTO-TESTS ==="
passed=0
total=0

# Проходим по всем .in файлам в папке
for infile in *.in; do
    # Проверяем, есть ли вообще такие файлы
    [ -e "$infile" ] || continue

    # Формируем имя файла-эталона
    outfile="${infile%.in}.out"
    total=$((total+1))

    echo -n "Test $infile: "

    # Запускаем программу
    "$EXE" < "$infile" > /tmp/test_out.tmp

    # Сравниваем с эталоном (игнорируя пробелы -w и переводы строк)
    if diff -w --strip-trailing-cr /tmp/test_out.tmp "$outfile" > /tmp/test_diff.tmp; then
        echo -e "\033[0;32mPASSED\033[0m"
        passed=$((passed+1))
    else
        echo -e "\033[0;31mFAILED\033[0m"
        cat /tmp/test_diff.tmp
    fi
done

echo "=========================="
echo -e "Result: $passed / $total passed"

# Возвращаем статус успешности для VS Code
[ $passed -eq $total ]
```

Перед отладкой по клавише `F5` сборка происходит автоматически.

Ручной запуск сборки сочетанием `ctrl + shift + B`.

Запуск автотестов сочетанием `ctrl + shift + T` - мне почему-то пришлось добавить вручную.

Код к каждой задаче писать в отдельной папке. Программа может состоять из произвольного количества `.cpp` и `.hpp` файлов.