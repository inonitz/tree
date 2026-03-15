#!/bin/bash


PROJECT_NAME=all
CMAKE_ARGLIST="\
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_COLOR_DIAGNOSTICS=ON \
    -DENABLE_SANITIZER_ADDRESS=ON \
    -DENABLE_SANITIZER_UNDEFINED=ON \
    -DENABLE_SANITIZER_MEMORY=ON \
"


CMAKE_ORIGINAL_SCRIPT_PATH="$PWD" # Assuming that build.sh is in the same dir as the Root CMakeLists.txt
CMAKE_ROOT_BUILD_DIR="build"
CMAKE_INTRMD_BUILD_DIR=""
CLEAN_CURRENT_ROOT_BUILD_DIR=true
CONFIGURE_CMAKE_FLAG=false
BUILD_BINARIES_FLAG=false


if [ "$1" = "help" ] || [ "$1" = "--help" ] || [ "$1" = "--h" ]; then
    echo "Usage: $0 <build_type> <library_type> <action>"
    echo ""
    echo "Arguments:"
    echo "  build_type   - Type of build: debug, release, release_dbginfo, debug_perf, release_perf"
    echo "  library_type - Type of library: shared (.dll/.so), static (.lib/.a)"
    echo "  action       - Action to take: cleanbuild, configure, build"
    echo ""
    echo "Options:"
    echo "  help         - Display this help message"
    echo ""
    echo "Examples:"
    echo "  ./build.sh debug   static build"
    echo "  ./build.sh release shared build"
    exit 0
fi




if [ $# -ne 3 ]; then
    echo "3 Arguments required to run the script"
    exit
fi


# argument parsing
if [ $1 = "debug" ];
then
    CMAKE_ARGLIST+=" -DCMAKE_BUILD_TYPE=Debug"
    CMAKE_INTRMD_BUILD_DIR+="debug/"

elif [ $1 = "debug_perf" ];
then
    CMAKE_ARGLIST+=" -DCMAKE_BUILD_TYPE=Debug"
    # CMAKE_ARGLIST+=" -DMEASURE_PERFORMANCE_TIMEOUT=1"
    CMAKE_INTRMD_BUILD_DIR+="debug_perf/"

elif [ $1 = "release" ];
then
    CMAKE_ARGLIST+=" -DCMAKE_BUILD_TYPE=Release"
    CMAKE_INTRMD_BUILD_DIR+="release/"

elif [ $1 = "release_dbginfo" ];
then
    CMAKE_ARGLIST+=" -DCMAKE_BUILD_TYPE=RelWithDbgInfo"
    CMAKE_INTRMD_BUILD_DIR+="release_dbginfo/"

elif [ $1 = "release_perf" ];
then
    CMAKE_ARGLIST+=" -DCMAKE_BUILD_TYPE=Release"
    # CMAKE_ARGLIST+=" -DMEASURE_PERFORMANCE_TIMEOUT=1"
    CMAKE_INTRMD_BUILD_DIR+="release_perf/"

else
    printf "Unknown Argument %s - valid values are: debug, release, release_dbginfo\nExiting..." $1
    exit
fi


if [ $2 = "shared" ];
then
    CMAKE_ARGLIST+=" -DBUILD_SHARED_LIBS=1"
    CMAKE_INTRMD_BUILD_DIR+="shared/"

elif [ $2 = "static" ];
then
    CMAKE_ARGLIST+=" -DBUILD_SHARED_LIBS=0"
    CMAKE_INTRMD_BUILD_DIR+="static/"

else
    printf "Unknown Argument %s - valid values are: shared, static\nExiting..." $2
    exit
fi



if [ $3 = "cleanbuild" ];
then
    CLEAN_CURRENT_ROOT_BUILD_DIR=true
    CONFIGURE_CMAKE_FLAG=false
    BUILD_BINARIES_FLAG=false

elif [ $3 = "configure" ];
then
    CLEAN_CURRENT_ROOT_BUILD_DIR=false
    CONFIGURE_CMAKE_FLAG=true
    BUILD_BINARIES_FLAG=false
    CMAKE_ARGLIST+=" -DGIT_SUBMODULE=ON"

elif [ $3 = "build" ];
then
    CLEAN_CURRENT_ROOT_BUILD_DIR=false
    CONFIGURE_CMAKE_FLAG=false
    BUILD_BINARIES_FLAG=true

else
    printf "Unknown Argument %s - valid values are: cleanbuild, configure, build\nExiting..." $3
    exit
fi





# the actual script
CMAKE_FINAL_BUILD_DIR="$CMAKE_ROOT_BUILD_DIR/$CMAKE_INTRMD_BUILD_DIR"
echo "Out-of-source Root   Build Directory is '$CMAKE_ROOT_BUILD_DIR' "
echo "Out-of-source Target Build Directory is '$CMAKE_FINAL_BUILD_DIR' "
echo "Cmake Arguments passed are ==> { "
echo "$CMAKE_ARGLIST"
echo "}"
echo "Script arguments are '$1' '$2' '$3' "


mkdir -p build
if [ "$CLEAN_CURRENT_ROOT_BUILD_DIR" = "true" ];
then
    rm -rf $CMAKE_FINAL_BUILD_DIR
fi


if [ "$CONFIGURE_CMAKE_FLAG" = "true" ]
then
mkdir -p $CMAKE_FINAL_BUILD_DIR
cmake -S . -B $CMAKE_FINAL_BUILD_DIR -G 'Ninja' $CMAKE_ARGLIST
fi


if [ "$BUILD_BINARIES_FLAG" = "true" ];
then
cd $CMAKE_FINAL_BUILD_DIR
cp "compile_commands.json" "../../compile_commands.json" 
# echo "CURRENT WORKING DIRECTORY IS $PWD"
ninja $PROJECT_NAME
fi


# if [ $RUN_TESTS_FLAG = "true" ];
# then
#     cd $CMAKE_FINAL_BUILD_DIR # This assumes we already built
#     ninja run_test_treelib
# fi

# if [ $RUN_BENCH_FLAG = "true" ];
# then
#     cd $CMAKE_FINAL_BUILD_DIR # This assumes we already built
#     ninja run_benchmark_treelib
# fi


# cd ../ # leave static/shared
# cd ../ # leave debug/release/etc...
# cd ../ # build