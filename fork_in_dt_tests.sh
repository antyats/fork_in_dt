#!/usr/bin/env bash

# Директория с тестовыми файлами
test_dir="test"
mkdir -p "$test_dir"

# Создаем тестовые файлы
cat > "$test_dir/test.c" <<EOL
#include <stdio.h>
int main() {
    printf("Hello, World!\n");
    return 0;
}
EOL

cat > "$test_dir/test.cpp" <<EOL
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
EOL

cat > "$test_dir/test.py" <<EOL
print("Hello, World!")
EOL

declare -A expected_output
expected_output["test.c"]='{"tokens": ["#include", "<stdio.h>", "int", "main", "(", ")", "{", "printf", "(", "\"Hello, World!\\n\"", ")", ";", "return", "0", ";", "}"]}'
expected_output["test.cpp"]='{"tokens": ["#include", "<iostream>", "int", "main", "(", ")", "{", "std", "::", "cout", "<<", "\"Hello, World!\"", "<<", "std", "::", "endl", ";", "return", "0", ";", "}"]}'
expected_output["test.py"]='{"tokens": ["print", "(", "\"Hello, World!\"", ")"]}'

# Запускаем тесты
for file in "$test_dir"/*; do
    filename=$(basename -- "$file")
    output=$(./fork-in-dt "$file")
    
    if [ "$output" == "${expected_output[$filename]}" ]; then
        echo "✅ $filename: Тест пройден"
    else
        echo "❌ $filename: Ошибка! Ожидалось: ${expected_output[$filename]}, Получено: $output"
    fi
done