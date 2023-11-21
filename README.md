Cmake工程 ：
    直接在当前目录下执行 ：  ./run.sh development   然后会自动创建一个build文件
    进入build 目录下   cd build   执行makefile 文件    make  
    就会生成对应的ELF文件  httpd  (仅支持在 arm 下运行)

对应四个目录：
    http ：返回HTTP请求

    script_executor_project ： 接收get请求  执行当前目录下的 test.sh 文件  并返回其脚本返回值 

    socket ： 处理socket部分 包括启动服务器 接收客户端请求  处理post请求  解析HTTP头部信息 和 拿到post请求携带的文件的头部信息

    string_handling： 字符串的处理 服务于socke请求  包括 获取http报文一行的数据，处理文件头部信息 处理文件内容 创建文件；


    

      