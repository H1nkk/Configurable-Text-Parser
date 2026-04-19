import subprocess
import sys
import os

def run_command(cmd, cwd=None):
    print(f"\nИсполение: {cmd}")
    result = subprocess.run(cmd, shell=True, cwd=cwd)
    if result.returncode != 0:
        print(f"Ошибка исполнения команды: {cmd}")
        sys.exit(1)
    return result

def main():
    print("Начало сборки...")
    
    run_command("cmake ..", cwd="build")
    
    run_command("cmake --build build --config Release")
    
    print("\n Сборка завершена успешно")

if __name__ == "__main__":
    main()