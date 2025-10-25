# 子模块：定义一个静态库（也可以是SHARED动态库）
add_library(app STATIC
  Sample-CLI-commands.c  # 子模块源文件
  UARTCommandConsole.c    # 头文件（可选，用于暴露接口）
)

# 可选：指定子模块的头文件路径（方便父项目包含）
target_include_directories(app
  PUBLIC  # PUBLIC意味着依赖该库的目标也能访问这些头文件
  ${CMAKE_CURRENT_LIST_DIR}  # 子模块自身的目录（包含sub_func.h）
)