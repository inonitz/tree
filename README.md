[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![MIT][license-shield]][license-url]



<!-- PROJECT LOGO -->
<div align="center">

<h3 align="center">Kernel Terminal Emulator</h3>

  <p align="center">
    Kernel Terminal Emulator for a freestanding Environment
  </p>

</div>

<!-- ABOUT THE PROJECT -->
## About The Project
Long ago I wrote up a terminal emulator / mini-graphics library for [primOS](https://github.com/inonitz/primOS) - it was required mostly for debugging the kernel bootup, printing status, etc...  
Due to many issues caused by hard-to-spot bugs, I decided to move this part of the kernel to its own independant project, that'll be integrated back with testing.  
  
This is a terminal emulator written in C, mainly for a free-standing environment  
More relevant details will be added in the future
<br></br>

### Project Structure
The project has the same structure as my other project [cmake_cuda_physics](https://github.com/inonitz/cmake_cuda_physics)
<br></br>

### Built With

[<img width="200" height="100" src="https://cmake.org/wp-content/uploads/2023/08/CMake-Logo.svg">](https://cmake.org)  
[![SDL3][SDL3.js]][SDL3-url]

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

* CMake
* Working compiler toolchain, preferably clang
  * Windows: You should use [llvm](https://github.com/llvm/llvm-project/releases)
  * Linux:
      1. [installing-specific-llvm-version](https://askubuntu.com/questions/1508260/how-do-i-install-clang-18-on-ubuntu)
      2. [configuring-symlinks](https://unix.stackexchange.com/questions/596226/how-to-change-clang-10-llvm-10-etc-to-clang-llvm-etc)
      3. **You Don't have to use LLVM, gcc works too**

### Installation
```sh
git clone --recurse-submodules https://github.com/inonitz/tree/tree.git
cd tree
```
<br></br>

<!-- USAGE EXAMPLES -->
## Usage

### Build Process

```sh
./build.sh debug/release/release_dbginfo static/shared clean/noclean run/norun
```

<br></br>

<!-- ROADMAP -->
## Roadmap

* None Yet

<!-- CONTRIBUTING -->
## Contributing

If you have a suggestion, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".  

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` file for more information.

<!-- ACKNOWLEDGEMENTS -->
<!-- ## Acknowledgement -->

<!-- References -->
## References

* [Modern CMake](https://cliutils.gitlab.io/modern-cmake/README.html)
* [Best-README](https://github.com/othneildrew/Best-README-Template)


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/inonitz/cmake_cuda_physics?style=for-the-badge&color=blue
[contributors-url]: https://github.com/inonitz/cmake_cuda_physics/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/inonitz/cmake_cuda_physics?style=for-the-badge&color=blue
[forks-url]: https://github.com/inonitz/cmake_cuda_physics/network/members
[stars-shield]: https://img.shields.io/github/stars/inonitz/cmake_cuda_physics?style=for-the-badge&color=blue
[stars-url]: https://github.com/inonitz/cmake_cuda_physics/stargazers
[issues-shield]: https://img.shields.io/github/issues/inonitz/cmake_cuda_physics.svg?style=for-the-badge
[issues-url]: https://github.com/inonitz/cmake_cuda_physics/issues
[license-shield]: https://img.shields.io/github/license/inonitz/cmake_cuda_physics?style=for-the-badge
[license-url]: https://github.com/inonitz/cmake_cuda_physics/blob/main/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username

[SDL3-url]: https://github.com/libsdl-org
[SDL3.js]: https://libsdl.org/media/SDL_logo.png