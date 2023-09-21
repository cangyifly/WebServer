#!/bash/sh

# 当前路径
current_dir=$(pwd)

# lib目录
lib_dir="${current_dir}/lib"

# bin目录
bin_dir="${current_dir}/bin"

# webserver
target="webserver"

# 导出动态库路径，可使用 ldd webserver查看依赖的动态库
export LD_LIBRARY_PATH=${lib_dir}

${bin_dir}/${target}