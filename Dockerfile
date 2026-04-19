FROM gcc:latest

WORKDIR /app

# установка зависимостей
RUN apt-get update && apt-get install -y cmake 

# копируем содержимое проекта с хоста в контейнер
COPY . .

# собираем проект
RUN cmake -B build && cmake --build build

# docker run запускает это
CMD ["./bin/parser", "--config", "test/example_test/configs/example.json", "--data", "test/example_test/sensor_data/"]