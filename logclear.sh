#!/bash/sh

# 当前路径
current_dir=$(pwd)

# lib目录
lib_dir="${current_dir}/lib"

# bin目录
bin_dir="${current_dir}/bin"

# log目录
log_dir="${current_dir}/log"

# webserver
target="webserver"

# 导出动态库路径，可使用 ldd webserver查看依赖的动态库
rm -rf ${log_dir}/*.*