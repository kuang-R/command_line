/** \mainpage

一个炫酷的命令行程序它到底需要什么？
- 构建系统
- 命令行参数解析
- 日志系统
- 代码文档系统

第三方库可以用cmake管理，自从qt官方构建系统转到cmake之后，感觉cmake基本上已经成为c++项目的事实构建系统了。
命令行参数解析和日志系统可以使用gflags和glog，代码文档系统自然是通用的doxygen。

## cmake
基本上可以把makefile淘汰掉了，makefile最致命的一点就是无法自动构建依赖。在c和c++中，源文件的头文件依赖如果使用makefile构建，要么就手动管理依赖（要命），要么就写个又长又臭的makefile来自动构建（还是要命）。在简单地学会cmake之后，就放心大胆地跟makefile说拜拜吧。

### 构建工程
有一些必须添加的首行比如指定cmake版本，添加源文件。
```
cmake_minimum_required(VERSION 3.20)
project(cmdline)
add_executable(${PROJECT_NAME} main.c)
```
还有就是引入依赖，这个工程我是基于glib做的。glib提供命令行解析、日志系统、单元测试等功能。
```
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
add_definitions(-DG_LOG_USE_STRUCTURED)
include_directories(${GLIB_INCLUDE_DIRS})
```
文档生成，引入Doxygen生成文档。
```
find_package(Doxygen)
if (DOXYGEN_FOUND)
        message("Doxygen build start")
        doxygen_add_docs(docs ${PROJECT_SOURCE_DIR} ALL)
endif()
```

## 命令行参数解析
命令行参数解析最常用的是posix标准getopt或者gnu的getopt，qt和glib都有自己的命令行解析工具。
我使用的是glib提供的命令行解析，简单用法跟getopt差不多。不过glib这个东西还扩展了一下用法，
有group这个概念，因为文档不详细我也没仔细研究。
``` c
static GOptionEntry entries[] = {
	{"ppid", 'p', 0, G_OPTION_ARG_STRING, &ppid, "Product id in apple mfi website", DEFAULT_PPID},
	{"logfile", 'l', 0, G_OPTION_ARG_STRING, &logfile, "The path to log file", DEFAULT_LOG_FILE},
	{NULL}
};

gboolean ret = FALSE;
GOptionContext *context = g_option_context_new("device-uuid");
/* setting parser */
g_option_context_add_main_entries(context, entries, NULL);
g_option_context_set_summary(context, "This program use to enable and disable apple token");
/* parse it */
ret = g_option_context_parse(context, &argc, &argv, NULL);
if (!ret)
	g_error("parse err");
```
解析完成之后，之前定义的变量就已经被改变，可以直接使用。

## 日志系统
glib的文档的确是有些少，如果需要找用法的话，把代码仓库克隆下来直接学习单元测试的代码比较好。
默认设置是不会输出g_info和g_debug和g_critical的，必须定义环境变量G_MESSAGE_DEBUG=all才行。
所以如果没有改变日志系统的全局回调函数，建议还是使用g_message, g_warning和g_error作为代替。

在这个工程中，日志我统一追加到日志文件的后面。debug由宏`NDEBUG`控制而且只输出到控制台，
error不但输出到控制台还会输出到日志文件中。具体实现可以看main.c，下面贴出设置回调的代码。
``` c
static GLogWriterOutput log_write(
		GLogLevelFlags log_level,
		const GLogField *fields,
		gsize n_fields,
		gpointer user_data)
{ ... }

static void set_log_path(const char *log_path)
{
	FILE *logfile = fopen(log_path, "a");
	if (logfile == NULL)
		g_error("open log file err: %s\n", strerror(errno));

	g_log_set_writer_func(log_write, (void *)logfile, NULL);
}

FILE *logfile = fopen(log_path, "a");
if (logfile == NULL)
	g_error("open log file err: %s\n", strerror(errno));
g_log_set_writer_func(log_write, (void *)logfile, NULL);
```

## 代码文档系统
直接从代码里生成文档是一件非常爽的事情，doxygen进行注释需要注意三个方面。
- 配置文件
- 文件头标识
- 函数注释和变量注释

### 配置文件
直接命令`doxygen -g`生成默认配置文件DoxyFile，具体的设置可以参考[官网](https://www.doxygen.nl/manual/config.html)。
在这个配置中我只改了一个变量，`EXTRACT_STATIC=YES`对static成员也生成注释。
一般的功能都可以用标识来实现。生成的话在上面的cmake配置文件里有写，当然直接生成也可以。

我在工程中是直接在从`CMakeLists.txt`中配置Doxygen的，所以并不需要Doxygen文件。

### 文件头标识
在这个文件（README.md)头加上`/** \mainpage`这一行就能把这个文件作为生成文档的主页面，
proxygen直接支持markdown语法，写起来还是挺爽的。

源代码文件头要加上一些标识符，指定对这个源文件进行文档生成。`\file`是必须有的，
具体可以参考[官网](https://www.doxygen.nl/manual/commands.html)
``` c
/**
 * \file main.c
 * \author Ray Ruan(a1173522112@163.com)
 * \date 2021-10-13
 */

```

### 注释
注释主要分两种，变量注释和一般注释。下面我展示我的用法，具体还是要看[官网](https://www.doxygen.nl/manual/docblocks.html)
~~~ c
int a; /**< variable a */
/**
 * \brief write buffer to somewhere
 * \details details of this function
 * \param buf buffer
 * \return none
 */
 void write(char buf[]);
 ~~~

 ## 工程代码
 工程代码我丢到了[github](https://github.com/kuang-R/command_line)上，有兴趣的话可以参考一下。