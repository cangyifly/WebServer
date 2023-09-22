#!/bash/sh

# 当前路径
current_dir=$(pwd)

# lib目录
lib_dir="${current_dir}/lib"

# bin目录
bin_dir="${current_dir}/bin"

# object目录
object_dir="${current_dir}/object"

# include目录
include_dir="${current_dir}/include"

# build目录
build_dir="${current_dir}/build"

# build makefile
build_makefile="${build_dir}/Makefile"

# export LD_LIBRARY_PATH=${lib_dir}

if [ ! -d "$lib_dir" ]; then
    mkdir "$lib_dir"
fi

if [ ! -d "$bin_dir" ]; then
    mkdir "$bin_dir"
fi

if [ ! -d "$object_dir" ]; then
    mkdir "$object_dir"
fi

if [ ! -d "$include_dir" ]; then
    mkdir "$include_dir"
fi

if [ ! -d "$build_dir" ]; then
    echo "directory [${build_dir}] not existed!"
    exit 1
fi

if [ ! -f "$build_makefile" ]; then
    echo "file [${build_makefile}] not existed!"
    exit 1
fi

cd ${build_dir} && make clean && make header && make