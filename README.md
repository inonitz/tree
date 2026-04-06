[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![MIT][license-shield]][license-url]



<!-- PROJECT LOGO -->
<div align="center">

<h3 align="center">Treelib</h3>

  <p align="center">
    AVL Tree Implementation in C & C++
  </p>

</div>

<!-- ABOUT THE PROJECT -->
## About The Project
A Long while ago I had to write a Virtual Address Space Manager for [primOS](https://github.com/inonitz/primOS) -  

At the time, reading forum posts, some educational materials and some of the linux kernel documentation  
led me to believe that Searching the Address Space would be way more common than insertion/deletion.  
Moreover, since AVL Trees **[Do keep their height to a minimum boundary](https://en.wikipedia.org/wiki/AVL_tree#Properties)**, I found it most fitting to pick an AVL Tree as my first choice  

I did eventually find a working, open-source implementation online that fit the bill (See Acknowledgements Section),  
But I never had the time to research the inner-workings of AVL Trees.  

This project Implements Generic AVL Trees in C & C++, with recursive & iterative implementations available.  
Specifically, the C++ interface offers both Iterative & recursive versions, while C only offers an iterative version  
All relevant data structures in this project are fully working, tested and benchmarked

<br></br>

### Project Structure
The project has the same structure as my other project [util2](https://github.com/inonitz/util2), except that I added benchmarks & Proper Unit Testing
<br></br>

<!-- ### Built With
[![CMake][CMake.js]][CMake-url]
[![CMocka][CMocka.js]][CMocka-url]
[![GoogleTest][GoogleTest.js]][GoogleTest-url]
[![GoogleBench][GoogleBench.js]][GoogleBench-url] -->

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

* CMake
* Working compiler toolchain, preferably clang  
  Windows: You should use [llvm](https://github.com/llvm/llvm-project/releases)  
  Linux:  
    1. [installing-specific-llvm-version](https://askubuntu.com/questions/1508260/how-do-i-install-clang-18-on-ubuntu)
    2. [configuring-symlinks](https://unix.stackexchange.com/questions/596226/how-to-change-clang-10-llvm-10-etc-to-clang-llvm-etc)
    3. **You Don't have to use LLVM, gcc works too**

### Building & (Maybe) Running

#### Downloading  

If you're only interested in the library itself, i.e no Testing/Benchmarking:
```sh
git clone https://github.com/inonitz/tree/tree.git --branch onlylibrary desired_folder_path_from_cwd
cd desired_folder_path_from_cwd/tree
```

If you want to build Tests/Benchmarks Too:  

```sh
git clone --recurse-submodules https://github.com/inonitz/tree/tree.git --branch master desired_folder_path_from_cwd
cd desired_folder_path_from_cwd/tree
```

<br></br>

#### Configuring

Because This project is somewhat big and building manually is cumbersome,  
I Wrote build scripts [build.sh](https://github.com/inonitz/tree/build.sh), [build.ps1](https://github.com/inonitz/tree/build.ps1) for Linux & Windows Respectively  

**By Default, The project will try to build EVERYTHING** - If you do not want that,  
add the following flags to your cmake invocation (Or If Building with the scripts, disable in your platforms' Script):

* ```-DTREELIB_BUILD_TESTS=OFF (Enabled By Default)```
* ```-DENABLE_SANITIZER_ADDRESS=OFF (Enabled By Default)```
* ```-DENABLE_SANITIZER_UNDEFINED=OFF (Enabled By Default)```
* ```-DENABLE_SANITIZER_MEMORY=OFF (Enabled By Default)```

Windows:

```sh
.\build.ps1 -Help
.\build.ps1 -BuildType release -LinkType shared -Action configure
.\build.ps1 -BuildType release -LinkType shared -Action build
.\build.ps1 -BuildType release -LinkType shared -Action runtests/runbench
```

Linux:

```sh
./build.sh --help
./build.sh release static configure
./build.sh release static build
./build.sh release static runtests/runbench
```

<br></br>

<!-- USAGE EXAMPLES -->
## Usage

### In Source Build

In your CMakeLists.txt:

```sh
add_subdirectory(your_directory/tree)
```
Also, Don't forget to link to the library:
```sh
target_link_libraries(your_target_executable/library PRIVATE TREELIB::treelib)
```

### Out-Of-Source (Submodule/etc...) Build

```sh
git submodule add https://github.com/inonitz/tree your_dependency_folder/tree
git submodule init
git submodule update
```

In your CMakeLists.txt:

```sh
add_subdirectory(your_dependency_folder/tree)
```
Also, Don't forget to link to the library:
```sh
target_link_libraries(your_target_executable/library PRIVATE TREELIB::treelib)
```

<br></br>


<!-- Benchmarks -->
## Benchmarks

Benchmarks will be added soon enough, thanks to google benchmarks' Over engineering :)

<br></br>

## Roadmap/TODO

* Optimize binaryTree::AVLDeleteIterative in AVLTreeGeneric.tpp, similarly to AVLInsertIterative in AVLTree.c
* Add Benchmarking Footnotes, and append link to top-of README
* Add Conclusion/What I learned
* Add Automatic Testing Matrix For architecture/OS/Build Type (Shared/Dynamic/Static, Debug/Release, ...)  
  * **(involves CI/CD which I'm not very familiar with currently)**

<br></br>

<!-- CONTRIBUTING -->
## Contributing

If you have a suggestion, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".  

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` file for more information.

<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements

* [GoogleBenchmark](https://github.com/google/benchmark)
* [GoogleTest](https://google.github.io/googletest)
* [CMocka](https://cmocka.org)
* [CMake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
* [Intrusive AVL Tree by Eric Biggers](https://github.com/ebiggers/avl_tree)

<!-- References -->
## References

* [Modern CMake](https://cliutils.gitlab.io/modern-cmake/README.html)
* [Best-README](https://github.com/othneildrew/Best-README-Template)
* [AVL Tree Playlist by William Fiset](https://youtube.com/playlist?list=PLDV1Zeh2NRsD06x59fxczdWLhDDszUHKt&si=N7kZmzkVAIHU4jjc)
* [Jenny's Data Structures & Algorithm Course](https://youtube.com/playlist?list=PLdo5W4Nhv31bbKJzrsKfMpo_grxuLl8LU&si=UGaS5lt1SiFYFAN-)  
  * In particular, her videos regarding AVL Tree Rotations
* [W3Schools AVL Trees](https://www.w3schools.com/dsa/dsa_data_avltrees.php)
* [Typed Tests (GoogleTest)](https://google.github.io/googletest/advanced.html#typed-tests)
* [GoogleBenchmark User Guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md)
  * I found the following references most relevant:
  * [Setup & Teardown](https://github.com/google/benchmark/blob/main/docs/user_guide.md#setupteardown)
  * [Templates](https://github.com/google/benchmark/blob/main/docs/user_guide.md#templated-benchmarks)
  * [Fixtures](https://github.com/google/benchmark/blob/main/docs/user_guide.md#Fixtures)
  * [Custom Counters](https://github.com/google/benchmark/blob/main/docs/user_guide.md#custom-counters)
  * [PauseTiming() & ResumeTiming()](https://github.com/google/benchmark/blob/main/docs/user_guide.md#controlling-timers)
* [Tick Formatting in Matplotlib](https://matplotlib.org/stable/gallery/ticks/tick-formatters.html)
* [Axis Ticks in Matplotlib](https://matplotlib.org/stable/users/explain/axes/axes_ticks.html)
* [Pathlib Basic Usage](https://stackoverflow.com/a/35188296)

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/inonitz/tree?style=for-the-badge&color=blue
[contributors-url]: https://github.com/inonitz/tree/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/inonitz/tree?style=for-the-badge&color=blue
[forks-url]: https://github.com/inonitz/tree/network/members
[stars-shield]: https://img.shields.io/github/stars/inonitz/tree?style=for-the-badge&color=blue
[stars-url]: https://github.com/inonitz/tree/stargazers
[license-shield]: https://img.shields.io/github/license/inonitz/tree?style=for-the-badge
[license-url]: https://github.com/inonitz/tree/blob/main/LICENSE

<!-- [CMake-url]: https://cmake.org/cmake/help/latest/guide/tutorial/index.html
[CMake.js]: https://gitlab.kitware.com/uploads/-/system/project/avatar/541/cmakelogo-centered.png?width=128

[CMocka-url]: https://cmocka.org
[CMocka.js]: https://avatars.githubusercontent.com/u/5657447?s=128&v=0

[GoogleTest-url]: https://google.github.io/googletest
[GoogleTest.js]: https://avatars.githubusercontent.com/u/1342004?s=128&v=4

[GoogleBench-url]: https://github.com/google/benchmark
[GoogleBench.js]: https://avatars.githubusercontent.com/u/1342004?s=128&v=4 -->
