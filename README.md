# dog_story

The backend of a multiplayer game written with Boost.Asio

## Build

### Free

```shell
mkdir build && cd build
cmake ..
cmake --build .
```

### Conan V1

```shell
mkdir build && cd build
conan install .. 
cmake -DCONANV1=ON ..
cmake --build .
```

### Conan V2

```shell
mkdir build && cd build
conan install ../conanfilev2.txt -of .
cmake -DCONANV2=ON --preset conan-release ..
cmake --build .
```