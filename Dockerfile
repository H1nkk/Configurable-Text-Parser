FROM gcc:latest

WORKDIR /app

# install dependencies
RUN apt-get update && apt-get install -y cmake 

# copy source
COPY . .

# build
RUN cmake -B build && cmake --build build

# run
CMD ["./bin/parser", "--config", "test/example_test/configs/example.json", "--data", "test/example_test/sensor_data/"]