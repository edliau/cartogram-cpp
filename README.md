# cartogram_cpp
Cartogram generator in C++

## Note: This is still a work-in-progress and, hence, cartograms are not outputted at the moment.

---

## Installing Dependencies on Ubuntu

#### Installing nlohmann's JSON parser
1. Go to https://github.com/nlohmann/json
2. Click on "Code" -> "Download Zip"
3. Go to Downloads folder
4. `cmake .`
5. `make`
6. `sudo make install`

#### Installing CGAL

[CGAL Homepage](https://www.cgal.org/)

`sudo apt-get install libcgal-dev`

#### Installing OpenMP

[OpenMP Homepage](https://www.openmp.org/)

`sudo apt-get install libomp-dev`


#### Installing FFTW3
1. Go to [FFTW's website](http://www.fftw.org/download.html "FFTW Downloads Page").
2. Install the latest version of FFTW
3. Unarchive the file with: `tar zxvf fftw-3.3.9.tar.gz`
4. Go to the directory with: `cd fftw-3.3.9`
5. `./configure`
6. `make`
7. `sudo make install`

---

## Compiling and running on Ubuntu

#### Compile by running:

1. `cd ./build`
2. `cmake .`
3. `make`

#### To use the cartogram generator:

1. `cd ./build`
2. `./cartogram -g your-geojson-file.geojson -v your-csv-file.csv`


- The `-g` flag accepts a GeoJSON or JSON file, in the standard GeoJSON format.
- The `-v` flag accepts a .csv file with your target areas data.

The csv file should be in the following format:

| NAME_1        | Data (eg: Population)| Color   |
| :------------ |:---------------------| :-------|
| Bruxelles     | 1208542              | #e74c3c |
| Vlaanderen    | 6589069              | #f1c40f |
| Wallonie      | 3633795              | #34495e |

- `NAME_1` should be the same as the identifying property's name in the GeoJSON. The rows should also have the same data as is present in the identifying property.
- `Data` contains the data you would like your cartogram to based on.
- `Color` is the hex color code you would like the geographic region to be colored.


---

## Setting up a Docker Ubuntu Container (with volume mounting) (Mac instructions)

1. If you have not already, download Docker Desktop from https://www.docker.com/products/docker-desktop.

2. Start Docker Desktop.

3. Change directories to your desired folder. (`vm` stands for "volume mount")

```
$ cd ~
$ mkdir cartogram_cpp_docker_vm
$ cd cartogram_cpp_docker_vm/
```

4. Pull the Ubuntu Docker image.

```
/cartogram_cpp_docker_vm$ docker pull ubuntu
```

5. View your Docker images.
```
/cartogram_cpp_docker_vm$ docker image ls
```

6. Create a container named `ubuntu-cartogram_cpp_vm` based on the Ubuntu image. The `-i` flag allows for an interactive container, the `-t` flag allows for terminal support within the container, and the `-v` flag allows for volume mounting. 

`"$(pwd)":/home` allows for the localhost present working directory (`cartogram_cpp_docker_vm/`) to sync with the Docker container `home/` directory (for example). 

Thus, any file you create in your localhost `cartogram_cpp_docker_vm/` directory will also appear in your Docker container `home/` directory, and vice versa.
```
/cartogram_cpp_docker_vm$ docker create  -it --name=ubuntu-cartogram_cpp_vm -v "$(pwd)":/home ubuntu bash
```

7. View your Docker containers and their relevant information. The `-a` flag means "all" and the `-s` flag means "size".
```
/cartogram_cpp_docker_vm$ docker ps -as
```

8. Start the `ubuntu-cartogram_cpp_vm` container. You will enter the root directory of this Ubuntu container in an environment where you can run your normal terminal commands (e.g., `cd`, `cp`, `mv` `pwd`).
```
/cartogram_cpp_docker_vm$ docker start -i ubuntu-cartogram_cpp_vm
```

9. To start a second (or third, fourth, etc.) terminal window in this container, open a new terminal tab and run the following command.
```
/cartogram_cpp_docker_vm$ docker exec -it ubuntu-cartogram_cpp_vm bash
```

10. To exit the container, run the following command.
```
root@<number>:/# exit
```
11. Start the container and install the necessary dependencies for cartogram_cpp.
```
/cartogram_cpp_docker_vm$ docker start -i ubuntu-cartogram_cpp_vm
root@<number>:/#
root@<number>:/# apt-get update
root@<number>:/# apt-get install sudo git vim cmake build-essential llvm libboost-all-dev g++-10 libcgal-dev libomp-dev
```

12. Download and copy the nlohmann JSON parser library to your localhost `cartogram_cpp_docker_vm/` directory.
    1. Go to https://github.com/nlohmann/json
    2. Click on "Code" -> "Download Zip"
    3. Go to your Downloads folder.
    4. Move the downloaded file from your Downloads folder to `cartogram_cpp_docker_vm/`.

13. To install nlohmann's JSON parser library, go back to your Ubuntu container terminal window and run the following commands.
```
root@<number>:/# cd home/json-develop/
root@<number>:/home/json-develop# cmake .
root@<number>:/home/json-develop# make
root@<number>:/home/json-develop# make install
```

14. Download and copy the FFTW library to your localhost `cartogram_cpp_docker_vm/` directory.
    1. Go to [FFTW's website](http://www.fftw.org/download.html "FFTW Downloads Page").
    2. Install the latest version of FFTW (http)
    3. Go to Downloads folder
    4. Move the downloaded file from the Downloads folder to `cartogram_cpp_docker_vm/`.

15. To unarchive and install the fftw3 library, go back to your Ubuntu container terminal window and run the following commands.
```
root@<number>:/# cd home/
root@<number>:/home# tar -xf fftw-3.3.9.tar
root@<number>:/home# rm fftw-3.3.9.tar
root@<number>:/home# cd fftw-3.3.9
root@<number>:/home/fftw-3.3.9# ./configure
root@<number>:/home/fftw-3.3.9# make
root@<number>:/home/fftw-3.3.9# sudo make install
```

16. In your localhost `cartogram_cpp_docker_vm/` directory, clone the `cartogram_cpp` repository.
```
/cartogram_cpp_docker_vm$ git clone git@github.com:mgastner/cartogram_cpp.git
```

17. In your Docker container, compile and execute the files and begin developing as per normal.
```
root@<number>:/# cd home/cartogram_cpp/build/
root@<number>:/home/cartogram_cpp/build# cmake .
root@<number>:/home/cartogram_cpp/build# make
root@<number>:/home/cartogram_cpp/build# ./cartogram -h
```

Again, the changes you make in your localhost `cartogram_cpp_docker_vm/` directory will be reflected in your Docker container `home/` directory and vice versa. This means that you may use the text editor of your choice to develop.